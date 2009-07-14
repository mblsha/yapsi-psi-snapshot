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

static const int CHECK_UNREAD_INTERVAL = 60; // seconds

YaUnreadMessagesManager::YaUnreadMessagesManager(PsiCon* parent)
	: QObject(parent)
	, controller_(parent)
	, lastSecondsIdle_(-1)
{
	checkUnreadTimer_ = new QTimer(this);
	checkUnreadTimer_->setSingleShot(true);
	connect(checkUnreadTimer_, SIGNAL(timeout()), SLOT(checkUnread()));
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
			if (event->account()) {
				JT_YaMessageRead* task = new XMPP::JT_YaMessageRead(event->account()->client()->rootTask());
				YaDateTime timeStamp = YaDateTime::fromYaTime_t(m.yaMessageId());
				task->messageRead(m.from(), timeStamp);
				task->go(true);
			}
		}
	}
}

void YaUnreadMessagesManager::checkUnread()
{
	checkUnreadTimer_->stop();

	if (!lastCheckTime_.isNull()) {
		if (lastCheckTime_.secsTo(QDateTime::currentDateTime()) < CHECK_UNREAD_INTERVAL) {
			checkUnreadTimer_->setInterval((CHECK_UNREAD_INTERVAL - lastCheckTime_.secsTo(QDateTime::currentDateTime())) * 1000);
			checkUnreadTimer_->start();
			return;
		}
	}

	lastCheckTime_ = QDateTime::currentDateTime();

	PsiAccount* historyAccount = controller_->contactList()->yaServerHistoryAccount();
	if (historyAccount && historyAccount->isAvailable()) {
		if (task_)
			delete task_;

#if 0
		task_ = new XMPP::JT_YaRetrieveHistory(historyAccount->client()->rootTask());
		connect(task_, SIGNAL(finished()), this, SLOT(checkUnreadFinished()));
		task_->checkUnread();
		task_->go(true);
#endif
	}
	else {
		lastCheckTime_ = QDateTime();
		checkUnreadTimer_->setInterval(10 * 1000);
		checkUnreadTimer_->start();
	}
}

struct YaMessageEventFound {
	MessageEvent* event;
	bool found;
};

void YaUnreadMessagesManager::checkUnreadFinished()
{
	PsiAccount* historyAccount = controller_->contactList()->yaServerHistoryAccount();
	Q_ASSERT(historyAccount);
	if (!historyAccount)
		return;

	QList<YaMessageEventFound> messages;

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		Q_ASSERT(event);
		if (event->type() == PsiEvent::Message) {
			YaMessageEventFound m;
			m.event = static_cast<MessageEvent*>(event);
			m.found = false;
			messages << m;
		}
	}

	XMPP::JT_YaRetrieveHistory* task = static_cast<XMPP::JT_YaRetrieveHistory*>(sender());
	if (task) {
		if (task->success()) {
			foreach(XMPP::JT_YaRetrieveHistory::Chat chat, task->messages()) {
				// Q_ASSERT(!chat.originLocal);
				if (chat.originLocal) {
					qWarning("local message: '%s'", qPrintable(chat.body));
					continue;
				}

				bool found = false;
				QMutableListIterator<YaMessageEventFound> it(messages);
				while (it.hasNext()) {
					YaMessageEventFound mef = it.next();
					Q_ASSERT(!mef.event->originLocal());
					YaDateTime timeStamp = YaDateTime::fromYaTime_t(mef.event->message().yaMessageId());
					if (chat.jid.compare(mef.event->message().from(), false) &&
					    chat.body == mef.event->message().body() &&
					    chat.timeStamp == timeStamp)
					{
						found = true;
						mef.found = true;
						it.setValue(mef);
						break;
					}
				}

				if (!found) {
					XMPP::Message m;
					m.setFrom(chat.jid);
					m.setBody(chat.body);
					m.setTimeStamp(chat.timeStamp);
					m.setYaMessageId(chat.timeStamp.toYaTime_t());

					historyAccount->client_messageReceived(m);
				}
			}

			QMutableListIterator<YaMessageEventFound> it(messages);
			while (it.hasNext()) {
				YaMessageEventFound mef = it.next();
				if (mef.found || !mef.event)
					continue;

				mef.event->account()->eventQueue()->dequeue(mef.event);
				delete mef.event;
				mef.event = 0;
				it.setValue(mef);
			}
		}
	}
}

void YaUnreadMessagesManager::secondsIdle(int seconds)
{
	if (!seconds || seconds < lastSecondsIdle_) {
		checkUnread();
	}

	lastSecondsIdle_ = seconds;
}
