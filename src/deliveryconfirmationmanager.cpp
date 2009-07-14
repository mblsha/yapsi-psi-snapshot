/*
 * deliveryconfirmationmanager.cpp
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

#include "deliveryconfirmationmanager.h"

#include <QDomElement>
#include <QTimer>

#include "xmpp_xmlcommon.h"
#include "xmpp_task.h"
#include "psiaccount.h"
#include "psicontact.h"
#include "capsmanager.h"
#include "userlist.h"

DeliveryConfirmationManager::DeliveryConfirmationManager(PsiAccount* account)
	: QObject(account)
	, account_(account)
{
	timer_ = new QTimer(this);
	timer_->setSingleShot(false);
	timer_->setInterval(10000);
	connect(timer_, SIGNAL(timeout()), SLOT(timeout()));
}

DeliveryConfirmationManager::~DeliveryConfirmationManager()
{
}

void DeliveryConfirmationManager::timeout()
{
	static const int deliveryConfirmationTimeoutSecs = 60;
	QDateTime currentDateTime = QDateTime::currentDateTime();
	QMutableHashIterator<QString, QDateTime> it(confirmations_);
	while (it.hasNext()) {
		it.next();
		if (it.value().secsTo(currentDateTime) > deliveryConfirmationTimeoutSecs) {
// FIXME
#ifdef YAPSI
			emit deliveryConfirmationUpdated(it.key(), YaChatViewModel::DeliveryConfirmation_Timeout);
#endif
			it.remove();
		}
	}

	updateTimer();
}

void DeliveryConfirmationManager::updateTimer()
{
	if (!confirmations_.isEmpty()) {
		timer_->start();
	}
	else {
		timer_->stop();
	}
}

bool DeliveryConfirmationManager::shouldQueryWithoutTimeout(const XMPP::Jid& jid) const
{
	PsiContact* contact = account_ ? account_->findContact(jid.bare()) : 0;
	bool requestMessageReceipt = false;
	XMPP::Status::Type status = XMPP::Status::Offline;

	foreach(UserListItem* u, account_->findRelevant(jid)) {
		foreach(UserResource r, u->userResourceList()) {
			XMPP::Jid j(u->jid());
			j = j.withResource(r.name());
			if (jid.resource() == r.name()) {
				status = r.status().type();
				break;
			}
		}
	}

	if (!jid.resource().isEmpty()) {
		requestMessageReceipt = account_->capsManager()->features(jid).canMessageReceipts() &&
		                        (status != XMPP::Status::Offline);
	}
	else {
		foreach(UserListItem* u, account_->findRelevant(jid)) {
			foreach(UserResource r, u->userResourceList()) {
				XMPP::Jid j(u->jid());
				j = j.withResource(r.name());

				if (!account_->capsManager()->features(j).canMessageReceipts()) {
					requestMessageReceipt = false;
					break;
				}
			}
		}
	}

	if (requestMessageReceipt && contact && contact->authorizesToSeeStatus()) {
		return false;
	}

	return true;
}

void DeliveryConfirmationManager::start(const QString& id)
{
	Q_ASSERT(!id.isEmpty());
	confirmations_[id] = QDateTime::currentDateTime();
	timer_->start();
}

bool DeliveryConfirmationManager::processIncomingMessage(const XMPP::Message& m)
{
	if (m.id().isEmpty())
		return false;

	if (/* m.error().type != XMPP::Stanza::Error::Cancel && */ m.error().condition != XMPP::Stanza::Error::UndefinedCondition) {
		// QString error = tr("%1: %2", "general_error_description: detailed_error_description")
		//                 .arg(m.error().description().first)
		//                 .arg(m.error().description().second);
		QString error = m.error().description().first;
		emit messageError(m.id(), error);
// FIXME
#ifdef YAPSI
		emit deliveryConfirmationUpdated(m.id(), YaChatViewModel::DeliveryConfirmation_Error);
#endif
		confirmations_.remove(m.id());
		return true;
	}
	else if (m.messageReceipt() == XMPP::ReceiptRequest) {
		// http://xmpp.org/extensions/xep-0184.html#security
		PsiContact* contact = account_->findContact(m.from().bare());
		if (contact && contact->authorized()) {
			// FIXME: must be careful sending these when we're invisible
			Message tm(m.from());
			tm.setId(m.id());
			tm.setMessageReceipt(XMPP::ReceiptReceived);
			account_->dj_sendMessage(tm, false);
		}
	}
	else if (m.messageReceipt() == XMPP::ReceiptReceived) {
// FIXME
#ifdef YAPSI
		emit deliveryConfirmationUpdated(m.id(), YaChatViewModel::DeliveryConfirmation_Verified);
#endif
		if (confirmations_.contains(m.id()))
			confirmations_.remove(m.id());
	}

	updateTimer();
	return false;
}
