/*
 * yahistorycachemanager.cpp
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

#include "yahistorycachemanager.h"

#include <QTimer>
#include <QtAlgorithms>

#include "psiaccount.h"
#include "psicon.h"
#include "profiles.h"
#include "atomicxmlfile.h"
#include "xmpp_xmlcommon.h"
#include "psicontactlist.h"

static const int MAX_MESSAGES = 20;
static const int CHECK_FOR_DEAD_TASKS_TIMEOUT = 60;

YaHistoryCacheManager::YaHistoryCacheManager(PsiCon* parent)
	: QObject(parent)
	, controller_(parent)
{
	writeToDiskTimer_ = new QTimer(this);
	writeToDiskTimer_->setSingleShot(false);
	writeToDiskTimer_->setInterval(10 * 1000);
	connect(writeToDiskTimer_, SIGNAL(timeout()), SLOT(writeToDisk()));

	getMessagesTimer_ = new QTimer(this);
	getMessagesTimer_->setSingleShot(false);
	getMessagesTimer_->setInterval(3 * 1000);
	connect(getMessagesTimer_, SIGNAL(timeout()), SLOT(getMessages()));

	checkForDeadTasksTimer_ = new QTimer(this);
	checkForDeadTasksTimer_->setSingleShot(false);
	checkForDeadTasksTimer_->setInterval(60 * 1000);
	connect(checkForDeadTasksTimer_, SIGNAL(timeout()), SLOT(checkForDeadTasks()));
	checkForDeadTasksTimer_->start();

	load();
}

YaHistoryCacheManager::~YaHistoryCacheManager()
{
	save();
}

QString YaHistoryCacheManager::getHashKey(PsiAccount* account, const XMPP::Jid& jid) const
{
	Q_ASSERT(account);
	return account->name() + "/" + jid.bare();
}

bool yaHistoryCacheManagerMessageLessThan(const YaHistoryCacheManager::Message& m1, const YaHistoryCacheManager::Message& m2)
{
	return m1.timeStamp < m2.timeStamp;
}

bool yaHistoryCacheManagerMessageMoreThan(const YaHistoryCacheManager::Message& m1, const YaHistoryCacheManager::Message& m2)
{
	return !yaHistoryCacheManagerMessageLessThan(m1, m2);
}

void YaHistoryCacheManager::appendMessage(PsiAccount* account, const XMPP::Jid& jid, const YaHistoryCacheManager::Message& message)
{
	QString key = getHashKey(account, jid);
	if (!cache_.contains(key)) {
		cache_[key] = QList<YaHistoryCacheManager::Message>();
	}

	Q_ASSERT(cache_.contains(key));
	if (!cache_[key].contains(message)) {
		cache_[key] << message;

		qStableSort(cache_[key].begin(), cache_[key].end(), yaHistoryCacheManagerMessageLessThan);
		while (cache_[key].count() > MAX_MESSAGES) {
			cache_[key].takeFirst();
		}

		writeToDiskTimer_->start();
	}

	cacheTimes_[key] = cache_[key].last().timeStamp;
}

bool YaHistoryCacheManager::hasMood(PsiAccount* account, const XMPP::Jid& jid, const QString& mood)
{
	QString key = getHashKey(account, jid);
	if (cache_.contains(key)) {
		foreach(const YaHistoryCacheManager::Message& msg, cache_[key]) {
			if (msg.isMood && msg.body == mood) {
				return true;
			}
		}
	}

	return false;
}

void YaHistoryCacheManager::writeToDisk()
{
	writeToDiskTimer_->stop();
	save();
}

QString YaHistoryCacheManager::fileName() const
{
	return pathToProfile(activeProfile) + "/yahistorycache.xml";
}

// TODO: QXmlStreamReader
void YaHistoryCacheManager::load()
{
	QDomDocument doc;

	AtomicXmlFile f(fileName());
	if (!f.loadDocument(&doc))
		return;

	QDomElement root = doc.documentElement();
	if (root.tagName() != "yahistorycache")
		return;

	if (root.attribute("version") != "1.1")
		return;

	for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement c = n.toElement();
		if (c.isNull())
			continue;

		if (c.tagName() == "contact") {
			XMPP::Jid accountJid(c.attribute("account"));
			XMPP::Jid contactJid(c.attribute("jid"));
			PsiAccount* account = controller_->contactList()->getAccountByJid(accountJid);
			if (!account)
				continue;

			for (QDomNode n2 = c.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
				QDomElement m = n2.toElement();
				if (m.isNull())
					continue;

				if (m.tagName() == "message") {
					YaHistoryCacheManager::Message msg;
					msg.timeStamp = YaDateTime::fromYaIsoTime(m.attribute("timeStamp"));
					msg.body = m.text();
					XMLHelper::readBoolAttribute(m, "originLocal", &msg.originLocal);
					XMLHelper::readBoolAttribute(m, "isMood", &msg.isMood);

					appendMessage(account, contactJid, msg);
				}
			}
		}
	}
}

// TODO: QXmlStreamWriter
void YaHistoryCacheManager::save()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("yahistorycache");
	root.setAttribute("version", "1.1");
	doc.appendChild(root);

	QHashIterator<QString, QList<YaHistoryCacheManager::Message> > it(cache_);
	while (it.hasNext()) {
		it.next();

		QStringList accountJid = it.key().split("/");
		Q_ASSERT(accountJid.count() == 2);
		QDomElement contact = doc.createElement("contact");
		contact.setAttribute("account", accountJid[0]);
		contact.setAttribute("jid", accountJid[1]);

		foreach(const YaHistoryCacheManager::Message& msg, it.value()) {
			QDomElement m = doc.createElement("message");
			QDomText text = doc.createTextNode(msg.body);
			m.appendChild(text);
			m.setAttribute("timeStamp", msg.timeStamp.toYaIsoTime());
			XMLHelper::setBoolAttribute(m, "originLocal", msg.originLocal);
			XMLHelper::setBoolAttribute(m, "isMood", msg.isMood);
			contact.appendChild(m);
		}

		root.appendChild(contact);
	}

	AtomicXmlFile f(fileName());
	if (!f.saveDocument(doc))
		return;
}

QList<YaHistoryCacheManager::Message> YaHistoryCacheManager::getCachedMessagesFor(PsiAccount* account, const XMPP::Jid& jid) const
{
	QString key = getHashKey(account, jid);
	if (!cache_.contains(key))
		return QList<YaHistoryCacheManager::Message>();
	return cache_[key];
}

void YaHistoryCacheManager::getMessagesFor(PsiAccount* account, const XMPP::Jid& jid, QObject* receiver, const char* slot)
{
	Q_ASSERT(account);
	Q_ASSERT(receiver);
	Q_ASSERT(slot);
	GetMessagesRequest request;
	request.account = account;
	request.jid = jid;
	request.receiver = receiver;
	request.slot = slot;
	request.started = false;
	request.finished = false;
	if (getMessagesRequests_.contains(request)) {
		return;
	}
	getMessagesRequests_ << request;

	getMessages();
}

void YaHistoryCacheManager::getMessages()
{
	getMessagesTimer_->stop();
	if (getMessagesRequests_.isEmpty())
		return;

	QMutableListIterator<GetMessagesRequest> it(getMessagesRequests_);
	while (it.hasNext()) {
		GetMessagesRequest request = it.next();
		if (!request.started) {
			Q_ASSERT(!request.finished);
			PsiAccount* historyAccount = controller_->contactList()->yaServerHistoryAccount();
			// Q_ASSERT(historyAccount);
			if (historyAccount) {
				if (!historyAccount->isAvailable()) {
					getMessagesTimer_->start();
					break;
				}

				request.started = true;
				request.startTime = QDateTime::currentDateTime();
				request.task = new JT_YaRetrieveHistory(historyAccount->client()->rootTask());
				connect(request.task, SIGNAL(finished()), this, SLOT(retrieveHistoryFinished()));
				request.task->retrieve(request.jid, MAX_MESSAGES, cacheTimes_[getHashKey(request.account, request.jid)]);
				request.task->go(true);
			}
			it.setValue(request);
		}
	}

	it.toFront();
	while (it.hasNext()) {
		GetMessagesRequest request = it.next();
		if (request.finished) {
			it.remove();
		}
	}
}

void YaHistoryCacheManager::checkForDeadTasks()
{
	bool doGetMessages = false;

	QMutableListIterator<GetMessagesRequest> it(getMessagesRequests_);
	while (it.hasNext()) {
		GetMessagesRequest request = it.next();
		if (request.started) {
			Q_ASSERT(!request.finished);

			int timeout = qAbs(QDateTime::currentDateTime().secsTo(request.startTime));
			if (timeout > CHECK_FOR_DEAD_TASKS_TIMEOUT) {
				qWarning("YaHistoryCacheManager::checkForDeadTasks(): '%s' timeout '%s'",
				         qPrintable(request.jid.full()),
				         qPrintable(request.startTime.toString(Qt::ISODate)));

				request.started = false;
				request.startTime = QDateTime();
				delete request.task;
				request.task = 0;

				doGetMessages = true;
				it.setValue(request);
			}
		}
	}

	if (doGetMessages) {
		getMessages();
	}
}

void YaHistoryCacheManager::retrieveHistoryFinished()
{
	JT_YaRetrieveHistory* task = static_cast<JT_YaRetrieveHistory*>(sender());
	if (task) {
		QMutableListIterator<GetMessagesRequest> it(getMessagesRequests_);
		while (it.hasNext()) {
			GetMessagesRequest request = it.next();
			if (request.started && !request.finished) {
				if (request.jid.compare(task->contact(), false)) {
					request.finished = true;

					if (task->success()) {
						// cache_.remove(getHashKey(request.account, request.jid));

						foreach(JT_YaRetrieveHistory::Chat chat, task->messages()) {
							YaHistoryCacheManager::Message msg;
							msg.timeStamp = chat.timeStamp;
							msg.body = chat.body;
							msg.originLocal = chat.originLocal;
							msg.isMood = false;

							appendMessage(request.account, request.jid, msg);
						}
					}
				}
				it.setValue(request);

				if (request.receiver) {
					QMetaObject::invokeMethod(request.receiver, request.slot);
				}
			}
		}
	}

	getMessages();
}
