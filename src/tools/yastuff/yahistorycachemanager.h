/*
 * yahistorycachemanager.h
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

#ifndef YAHISTORYCACHEMANAGER_H
#define YAHISTORYCACHEMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QPointer>

class QTimer;
class PsiAccount;
class PsiCon;

#include "xmpp_jid.h"
#include "xmpp_yadatetime.h"

class YaHistoryCacheManager : public QObject
{
	Q_OBJECT
public:
	YaHistoryCacheManager(PsiCon* parent);
	~YaHistoryCacheManager();

	struct Message {
		XMPP::YaDateTime timeStamp;
		QString body;
		bool originLocal;
		bool isMood;

		bool operator==(const Message& other) const
		{
			return timeStamp == other.timeStamp &&
			       body == other.body &&
			       originLocal == other.originLocal &&
			       isMood == other.isMood;
		}
	};

	void getMessagesFor(PsiAccount* account, const XMPP::Jid& jid, QObject* receiver, const char* slot);
	QList<YaHistoryCacheManager::Message> getCachedMessagesFor(PsiAccount* account, const XMPP::Jid& jid) const;
	bool hasMood(PsiAccount* account, const XMPP::Jid& jid, const QString& mood);

private slots:
	void writeToDisk();
	void getMessages();
	void load();
	void save();

	void retrieveHistoryFinished();

protected:
	void appendMessage(PsiAccount* account, const XMPP::Jid& jid, const YaHistoryCacheManager::Message& message);

	QString getHashKey(PsiAccount* account, const XMPP::Jid& jid) const;
	QString fileName() const;

private:
	struct GetMessagesRequest {
		PsiAccount* account;
		XMPP::Jid jid;
		QPointer<QObject> receiver;
		const char* slot;
		bool started;
		bool finished;

		bool operator==(const GetMessagesRequest& other) const
		{
			return account == other.account &&
			       jid == other.jid &&
			       receiver == other.receiver &&
			       slot == other.slot;
		}
	};

	PsiCon* controller_;
	QTimer* writeToDiskTimer_;
	QTimer* getMessagesTimer_;
	QHash<QString, QList<YaHistoryCacheManager::Message> > cache_;
	QHash<QString, XMPP::YaDateTime> cacheTimes_;
	QList<GetMessagesRequest> getMessagesRequests_;
};

bool yaHistoryCacheManagerMessageLessThan(const YaHistoryCacheManager::Message& m1, const YaHistoryCacheManager::Message& m2);
bool yaHistoryCacheManagerMessageMoreThan(const YaHistoryCacheManager::Message& m1, const YaHistoryCacheManager::Message& m2);

#endif
