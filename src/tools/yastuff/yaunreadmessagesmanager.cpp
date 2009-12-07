/*
 * yaunreadmessagesmanager.cpp
 * Copyright (C) 2009  Yandex LLC (Michail Pishchagin)
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

#include "yaunreadmessagesmanager.h"

#include <QTimer>

#include "psicon.h"
#include "psiaccount.h"
#include "psicontactlist.h"
#include "psievent.h"
#include "xmpp_message.h"
#include "xmpp_tasks.h"
#include "globaleventqueue.h"

#include "xmpp_yadatetime.h"

static const int MAX_LAST_READ_MESSAGES = 100;

YaUnreadMessagesManager::YaUnreadMessagesManager(PsiCon* parent)
	: QObject(parent)
	, controller_(parent)
{
	connect(controller_->contactList(), SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
	accountCountChanged();
}

YaUnreadMessagesManager::~YaUnreadMessagesManager()
{
}

void YaUnreadMessagesManager::eventRead(PsiEvent* event)
{
	if (event->type() == PsiEvent::Message) {
		MessageEvent *me = static_cast<MessageEvent*>(event);
		const XMPP::Message& m = me->message();
		if (!m.yaMessageId().isEmpty()) {
			Q_ASSERT(event->account());
			if (event->account() && event->account()->isYaAccount()) {
				JT_YaMessageRead* task = new XMPP::JT_YaMessageRead(event->account()->client()->rootTask());
				YaDateTime timeStamp = YaDateTime::fromYaTime_t(m.yaMessageId());
				task->messageRead(m.from(), timeStamp);
				addReadMessage(m.from(), timeStamp);
				task->go(true);
			}
		}
	}
}

void YaUnreadMessagesManager::messageUnreadPush(const XMPP::Jid& jid, const XMPP::YaDateTime& timeStamp, const QString& body)
{
	PsiAccount* historyAccount = controller_->contactList()->yaServerHistoryAccount();
	Q_ASSERT(historyAccount);
	if (!historyAccount)
		return;

	ReadMessage readMessage(jid, timeStamp);
	if (lastReadMessages.contains(readMessage)) {
		qWarning("YaUnreadMessagesManager::messageUnreadPush: already read %s,%s,%s", qPrintable(jid.full()), qPrintable(timeStamp.toYaIsoTime()), qPrintable(body));
		return;
	}

	XMPP::Message m;
	m.setFrom(jid);
	m.setBody(body);
	m.setTimeStamp(timeStamp);
	m.setYaMessageId(timeStamp.toYaTime_t());

	historyAccount->client_messageReceived(m);
}

void YaUnreadMessagesManager::accountCountChanged()
{
	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		disconnect(account, SIGNAL(messageReadPush(const XMPP::Jid&, const XMPP::YaDateTime&)), this, SLOT(messageReadPush(const XMPP::Jid&, const XMPP::YaDateTime&)));
		connect(account,    SIGNAL(messageReadPush(const XMPP::Jid&, const XMPP::YaDateTime&)), this, SLOT(messageReadPush(const XMPP::Jid&, const XMPP::YaDateTime&)));

		disconnect(account, SIGNAL(messageUnreadPush(const XMPP::Jid&, const XMPP::YaDateTime&, const QString&)), this, SLOT(messageUnreadPush(const XMPP::Jid&, const XMPP::YaDateTime&, const QString&)));
		connect(account,    SIGNAL(messageUnreadPush(const XMPP::Jid&, const XMPP::YaDateTime&, const QString&)), this, SLOT(messageUnreadPush(const XMPP::Jid&, const XMPP::YaDateTime&, const QString&)));
	}
}

void YaUnreadMessagesManager::messageReadPush(const XMPP::Jid& jid, const XMPP::YaDateTime& timeStamp)
{
	PsiAccount* account = static_cast<PsiAccount*>(sender());
	if (account != controller_->contactList()->yaServerHistoryAccount())
		return;

	addReadMessage(jid, timeStamp);

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		Q_ASSERT(event);
		if (event->type() == PsiEvent::Message) {
			MessageEvent* me = static_cast<MessageEvent*>(event);
			YaDateTime event_timeStamp = YaDateTime::fromYaTime_t(me->message().yaMessageId());
			if (jid.compare(me->message().from(), false) &&
			    timeStamp == event_timeStamp)
			{
				me->account()->eventQueue()->dequeue(me);
				delete me;
			}
		}
	}

}

void YaUnreadMessagesManager::addReadMessage(const XMPP::Jid& jid, const XMPP::YaDateTime& timeStamp)
{
	ReadMessage readMessage(jid, timeStamp);
	if (!lastReadMessages.contains(readMessage)) {
		lastReadMessages.append(readMessage);
	}

	while (lastReadMessages.count() > MAX_LAST_READ_MESSAGES) {
		lastReadMessages.takeFirst();
	}
}
