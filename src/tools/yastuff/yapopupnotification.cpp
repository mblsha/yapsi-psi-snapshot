/*
 * yapopupnotification.cpp - custom popup notifications
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

#include "yapopupnotification.h"

#include <QVBoxLayout>

#include "psiaccount.h"
#include "psicon.h"
#include "psievent.h"
#include "yaprofile.h"
#include "yatoster.h"
#include "yaeventnotifierframe.h"
#include "yacommon.h"
#include "chatdlg.h"
#include "globaleventqueue.h"
#include "psicontact.h"
#include "yatoastercentral.h"

#include "xmpp_resource.h"
#include "psipopup.h"
#include "psioptions.h"
#if defined(Q_WS_MAC) && defined(HAVE_GROWL)
#include "psigrowlnotifier.h"
#endif

#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif

void YaPopupNotification::notify(int id, PsiEvent* event)
{
	Q_ASSERT(event);

#ifndef YAPSI_ACTIVEX_SERVER
	// FIXME: should use PsiAccount::Private::noPopup()
	if (!event->account() || event->account()->status().type() != XMPP::Status::Online)
		return;
#endif

	if (event->type() == PsiEvent::Message || event->type() == PsiEvent::Mood) {
		ChatDlg* chatDlg = event->account()->findChatDialog(event->from());
		if (!chatDlg)
			chatDlg = event->account()->findChatDialog(event->jid());

		if (chatDlg && chatDlg->isActiveTab())
			return;
	}

	if (event->type() == PsiEvent::Message) {
#ifndef YAPSI_ACTIVEX_SERVER
		if (!PsiOptions::instance()->getOption("options.ya.popups.message.enable").toBool()) {
			return;
		}
#endif
	}

	if (event->type() == PsiEvent::Mood) {
		MoodEvent* moodEvent = static_cast<MoodEvent*>(event);
		if (!event->account()->psi()->yaToasterCentral()->showToaster(YaToasterCentral::MoodChange, event->from(), moodEvent->mood())) {
			// qWarning("YaToasterCentral blocked mood change: %s %s", qPrintable(contact->jid().full()), qPrintable(mood));
			return;
		}

		// qWarning("%s Changed mood to: %s", qPrintable(contact->jid().full()), qPrintable(mood));

#ifndef YAPSI_ACTIVEX_SERVER
		if (!PsiOptions::instance()->getOption("options.ya.popups.moods.enable").toBool()) {
			return;
		}
#endif

#if defined(Q_WS_MAC) && defined(HAVE_GROWL)
		PsiContact* contact = event->account()->findContact(event->from());
		if (contact) {
			QString contactName = Ya::contactName(contact->name(), contact->jid().bare());
			PsiGrowlNotifier::instance()->moodChanged(event->account(), contact->jid(), contactName, moodEvent->mood());
		}
		return;
#endif
	}

#ifdef YAPSI_ACTIVEX_SERVER
	if (event->account()->psi()->yaOnline()) {
		event->account()->psi()->yaOnline()->notify(id, event);
		return;
	}
#endif

#if defined(Q_WS_MAC) && defined(HAVE_GROWL)
	// nasty-nasty YaToster steals focus on Mac OS X
	return;
#endif

	YaPopupNotification* notification = new YaPopupNotification(id, event);
	int height = notification->sizeHint().height();
	notification->setMinimumSize(QSize(16, height));
	YaToster* toster = new YaToster(notification);
	toster->setStayForever(Ya::isSubscriptionRequest(event));
	toster->setTimes(1000, 5000, 500);
	toster->setSize(QSize(280, height));
	toster->start();
}

YaPopupNotification::YaPopupNotification(int id, PsiEvent* event)
	: QWidget()
{
	account_ = event->account();

	if (id >= 0)
		connect(event, SIGNAL(destroyed(QObject*)), SLOT(eventDestroyed()));

	QVBoxLayout* vbox = new QVBoxLayout(this);
	vbox->setMargin(0);
	eventNotifierFrame_ = new YaEventNotifierFrame(this);
	vbox->addWidget(eventNotifierFrame_);

	eventNotifierFrame_->setTosterMode(true);
	eventNotifierFrame_->setEvent(id, event);
	eventNotifierFrame_->setEventCounter(0, 0);
	connect(eventNotifierFrame_, SIGNAL(closeClicked()), SLOT(closeToster()));
	connect(eventNotifierFrame_, SIGNAL(clicked()), SLOT(clicked()));
}

void YaPopupNotification::closeToster()
{
	YaToster* toster = dynamic_cast<YaToster*>(parentWidget());
	Q_ASSERT(toster);
	toster->close();
}

void YaPopupNotification::eventDestroyed()
{
	closeToster();
}

void YaPopupNotification::mousePressEvent(QMouseEvent* e)
{
	closeToster();
	if (e->button() == Qt::LeftButton) {
		clicked();
	}
}

void YaPopupNotification::clicked()
{
	closeToster();
	openNotify(eventNotifierFrame_->id(),
	           account_,
	           eventNotifierFrame_->currentJid(),
	           UserAction);
}

void YaPopupNotification::openNotify(int id, PsiAccount* account, const XMPP::Jid& jid, ActivationType activationType)
{
	if (id >= 0) {
		if (GlobalEventQueue::instance()->ids().contains(id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(id);
			event->account()->psi()->processEvent(event, activationType);
			return;
		}
	}

	if (account) {
		if (activationType == UserAction) {
			account->actionOpenChatSpecific(jid);
		}
		else if (activationType == UserPassiveAction) {
			account->actionOpenPassiveChatSpecific(jid);
		}
		else {
			Q_ASSERT(activationType == FromXml);
			account->actionOpenSavedChat(jid);
		}
	}
}
