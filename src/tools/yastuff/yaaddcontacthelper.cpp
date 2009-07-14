/*
 * yaaddcontacthelper.cpp
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "yaaddcontacthelper.h"

#include <QtAlgorithms>
#include <QTimer>

#include "vcardfactory.h"
#include "yacommon.h"
#include "xmpp_jid.h"
#include "xmpp_tasks.h"
#include "psiaccount.h"
#include "yaonline.h"
#include "yarostertooltip.h"
#include "psicon.h"
#include "psicontactlist.h"
#include "psicontact.h"
#include "yacontactlistmodel.h"

YaAddContactHelper::YaAddContactHelper(QObject* parent)
	: QObject(parent)
{
	searchTimer_ = new QTimer(this);
	connect(searchTimer_, SIGNAL(timeout()), SLOT(searchTimerTimeout()));
	searchTimer_->setInterval(20000);
	searchTimer_->setSingleShot(true);

	ljTimer_ = new QTimer(this);
	connect(ljTimer_, SIGNAL(timeout()), SLOT(ljTimerTimeout()));
	ljTimer_->setSingleShot(true);
}

YaAddContactHelper::~YaAddContactHelper()
{
}

void YaAddContactHelper::setYaOnline(YaOnline* yaOnline)
{
	yaOnline_ = yaOnline;
}

void YaAddContactHelper::setAccount(PsiAccount* account)
{
	account_ = account;
}

void YaAddContactHelper::setContactListModel(YaContactListModel* model)
{
	contactListModel_ = model;
}

QStringList YaAddContactHelper::groups() const
{
	QStringList result;
	if (account_ && account_->psi()) {
		foreach(PsiAccount* account, account_->psi()->contactList()->enabledAccounts()) {
			foreach(const QString& g, account->groupList()) {
				if (!result.contains(g))
					result += g;
			}
		}
	}

	if (contactListModel_) {
		foreach(const QString& g, contactListModel_->virtualGroupList()) {
			if (!result.contains(g))
				result += g;
		}
	}

	result.sort();

	result.removeAll(Ya::INFORMERS_GROUP_NAME);

#ifdef USE_GENERAL_CONTACT_GROUP
	// 'General' must be the first one
	result.removeAll(PsiContact::generalGroupName());
	result.prepend(PsiContact::generalGroupName());
#endif

	return result;
}

void YaAddContactHelper::addContactClicked(QRect addButtonRect, QRect windowRect)
{
	if (!yaOnline_.isNull()) {
		yaOnline_->addContact(windowRect.left(), addButtonRect.bottom(), windowRect.width(), groups());
	}
}

void YaAddContactHelper::findContact(const QStringList& _jids, const QString& type)
{	
	if (yaOnline_.isNull() || account_.isNull())
		return;

	jids_ = _jids;

	if (!jids_.isEmpty() && type == "jabber") {
		XMPP::Jid jid(jids_.first());

		if (jid.node().isEmpty() && !jid.domain().isEmpty()) {
			jid.setNode(jid.domain());
			foreach(PsiAccount* account, account_->psi()->contactList()->accounts()) {
				jid.setDomain(account->jid().domain());
				jids_ << jid.bare();
			}
		}
	}

#if 0
	{
		// prevent showing self contacts in search results
		foreach(PsiAccount* account, account_->psi()->contactList()->enabledAccounts()) {
			if (jids_.contains(account->jid().bare())) {
				jids_.removeAll(account->jid().bare());
			}
		}

		if (jids_.isEmpty()) {
			searchComplete();
			return;
		}
	}
#endif

	QStringList tmpJids;
	foreach(const QString& j, jids_) {
		tmpJids << Ya::yaRuAliasing(j);
	}
	jids_ = tmpJids;

	jids_.sort();
	jids_.removeDuplicates();

	jidsOriginal_ = jids_;
	findStartedTime_ = QTime::currentTime();
	searchTimer_->start();

	foreach(QString j, jids_) {
#if 0 // ONLINE-2072
		if (VCardFactory::instance()->vcard(j)) {
			taskFinished(j);
			continue;
		}
#endif

		PsiAccount* account = account_;
		XMPP::Jid jid(j);
		if (jid.domain() == "yandex-team.ru") {
			if (account_->psi()->contactList()->yandexTeamAccount() &&
			    account_->psi()->contactList()->yandexTeamAccount()->isAvailable())
			{
				account = account_->psi()->contactList()->yandexTeamAccount();
			}
		}

		VCardFactory::instance()->getVCard(j, account->client()->rootTask(), this, SLOT(taskFinished()));
	}

	if (!jids_.isEmpty()) {
		bool doLjHack = false;
		foreach(QString j, jids_) {
			XMPP::Jid jid(j);
			Q_ASSERT(jid.bare() == j);
			if (jid.domain() == "livejournal.com" && !jids_.contains(ljHackJid())) {
				doLjHack = true;
			}
		}

		if (doLjHack) {
			VCardFactory::instance()->getVCard(ljHackJid(), account_->client()->rootTask(), this, SLOT(taskFinished()));
		}
	}
}

QString YaAddContactHelper::ljHackJid() const
{
	return "yaonline@livejournal.com";
}

void YaAddContactHelper::cancelSearch()
{
	searchTimer_->stop();
	ljTimer_->stop();

	jids_.clear();
	jidsOriginal_.clear();
}

void YaAddContactHelper::addContact(const QString& jid, const QString& _group)
{
	if (yaOnline_.isNull() || account_.isNull())
		return;

	const XMPP::VCard* vcard = VCardFactory::instance()->vcard(jid);
	QStringList groups;
	QString group = _group;
#ifdef USE_GENERAL_CONTACT_GROUP
	if (group == PsiContact::generalGroupName())
		group = QString();
#endif
	if (!group.isEmpty())
		groups << group;

	XMPP::Jid j(jid);
	emit YaRosterToolTip::instance()->addContact(jid,
	        j.domain() != "yandex-team.ru" ? account_ : 0,
	        groups,
			vcard ? Ya::nickFromVCard(jid, vcard) : QString());
}

void YaAddContactHelper::foundContact(PsiAccount* account, const XMPP::Jid& jid)
{
	if (yaOnline_.isNull() || !account)
		return;

	if (jid.bare().isEmpty())
		return;

	bool selfContact = false;
	if (account && account->psi()) {
		foreach(PsiAccount* a, account->psi()->contactList()->enabledAccounts()) {
			if (a->jid() == jid) {
				selfContact = true;
				break;
			}
		}
	}

	PsiContact* contact = Ya::findContact(account ? account->psi() : 0, jid.bare());
	yaOnline_->foundContact(account, jid,
	                        contact ? contact->inList() : false,
	                        contact ? contact->authorizesToSeeStatus() : false,
	                        selfContact);
}

void YaAddContactHelper::searchComplete()
{
	if (yaOnline_.isNull() || account_.isNull())
		return;

	cancelSearch();
	yaOnline_->searchComplete();
}

void YaAddContactHelper::taskFinished(const XMPP::Jid& jid)
{
	if (yaOnline_.isNull() || account_.isNull())
		return;

	if (!jids_.contains(jid.full())) {
		if (jid.full() == ljHackJid()) {
			ljTimer_->setInterval(findStartedTime_.msecsTo(QTime::currentTime()) * 2);
			ljTimer_->start();
		}
		return;
	}

	if (VCardFactory::instance()->vcard(jid)) {
		foundContact(account_, jid);
	}
	else {
		// special logic for gmail.com contacts, whose vCards
		// are not shown unless one has their subscription
		if (jid.domain() == "gmail.com" && jidsOriginal_.count() == 1) {
			foundContact(account_, jid);
		}
	}

	jids_.removeAll(jid.full());
	if (jids_.isEmpty()) {
		searchComplete();
	}
}

void YaAddContactHelper::taskFinished()
{
	XMPP::JT_VCard* task = dynamic_cast<XMPP::JT_VCard*>(sender());
	Q_ASSERT(task);

	taskFinished(task->jid());
}

void YaAddContactHelper::searchTimerTimeout()
{
	if (!jids_.isEmpty()) {
		searchComplete();
	}
}

void YaAddContactHelper::ljTimerTimeout()
{
	searchTimerTimeout();
}
