/*
 * yaunreadmessagesmanager.h
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

#ifndef YAUNREADMESSAGESMANAGER_H
#define YAUNREADMESSAGESMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QPointer>

class QTimer;

class PsiCon;
class PsiEvent;
namespace XMPP {
	class JT_YaRetrieveHistory;
};

#include "xmpp_jid.h"
#include "xmpp_yadatetime.h"

class YaUnreadMessagesManager : public QObject
{
	Q_OBJECT
public:
	YaUnreadMessagesManager(PsiCon* parent);
	~YaUnreadMessagesManager();

	void eventRead(PsiEvent* event);

private slots:
	void messageReadPush(const XMPP::Jid& jid, const XMPP::YaDateTime& timeStamp);
	void messageUnreadPush(const XMPP::Jid& jid, const XMPP::YaDateTime& timeStamp, const QString& body);
	void accountCountChanged();

private:
	QPointer<PsiCon> controller_;

	struct ReadMessage {
		ReadMessage(const XMPP::Jid& _jid, const XMPP::YaDateTime& _timeStamp)
			: jid(_jid)
			, timeStamp(_timeStamp)
		{}

		bool operator==(const ReadMessage& other) const
		{
			return jid.compare(other.jid, false) &&
			       timeStamp == other.timeStamp;
		}

		XMPP::Jid jid;
		XMPP::YaDateTime timeStamp;
	};
	QList<ReadMessage> lastReadMessages;
	void addReadMessage(const XMPP::Jid& jid, const XMPP::YaDateTime& timeStamp);
};

#endif
