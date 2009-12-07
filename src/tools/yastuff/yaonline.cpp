/*
 * yaonline.cpp - communication with running instance of Online
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

#include "yaonline.h"

#include <QDomDocument>
#include <QDomElement>
#include <QtCrypto>
#include <QTimer>
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QFile>
#include <QDomElement>
#include <QVariant>
#include <QSettings>

#include <windows.h>
#include <tlhelp32.h>
#include <sddl.h>

#include "dummystream.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psicontactlist.h"
#include "psievent.h"
#include "xmpp_yalastmail.h"
#include "yaipc.h"
#include "yapopupnotification.h"
#include "yapsiserver.h"
#include "ycuapiwrapper.h"
#include "profiles.h"
#include "proxy.h"
#include "psioptions.h"
#include "common.h"
#include "globaleventqueue.h"
#include "psicontact.h"
#include "psicon.h"
#include "tabmanager.h"
#include "tabdlg.h"
#include "tabbablewidget.h"
#include "yachatdlg.h"
#include "psilogger.h"
#include "textutil.h"
#include "yavisualutil.h"
#include "yacommon.h"
#include "xmpp_xmlcommon.h"
#include "cutejson.h"
#include "yaaddcontacthelper.h"
#include "vcardfactory.h"
#include "yaunreadmessagesmanager.h"
#include "yaonlinemainwin.h"

static const QString alwaysOnTopOptionPath = "options.ya.main-window.always-on-top";
static const QString migrateOldOptionsOptionPath = "options.ya.migrate-old-options";
static const QString chatBackgroundOptionPath = "options.ya.chat-background";
static const QString alwaysShowToastersOptionPath = "options.ya.popups.always-show-toasters";

YaOnline* YaOnlineHelper::instance()
{
	Q_ASSERT(YaOnline::instance_);
	return YaOnline::instance_;
}

YaOnline* YaOnline::instance_ = 0;

YaOnline::YaOnline(YaPsiServer* parent)
	: QObject(parent)
	, server_(parent)
	, controller_(0)
	, ycuApi_(0)
	, addContactHelper_(0)
	, lastStatus_(XMPP::Status::Online)
	, onlineAccountDnd_(false)
	, onlineAccountConnected_(false)
	, canConnectConnections_(false)
	, useSsl_(true)
	, onlinePasswordVerified_(false)
	, doStartActions_(false)
	, checkForAliveOnlineProcessTimer_(0)
{
	// Q_ASSERT(!instance_);
	instance_ = this;

	LOG_TRACE;
	addContactHelper_ = new YaAddContactHelper(this);
	addContactHelper_->setYaOnline(this);
	LOG_TRACE;
	ycuApi_ = new YcuApiWrapper(this);
	LOG_TRACE;
	connect(parent, SIGNAL(onlineObjectChanged()), SLOT(onlineObjectChanged()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doStartActions()), SLOT(doStartActions()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doShowRoster(bool)), SLOT(doShowRoster(bool)), Qt::QueuedConnection);
	connect(parent, SIGNAL(disconnectRereadSettingsAndReconnect(bool)), SLOT(disconnectRereadSettingsAndReconnect(bool)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doSetOfflineMode()), SLOT(doSetOfflineMode()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doSetDND(bool)), SLOT(doSetDND(bool)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doSetStatus(const QString&)), SLOT(doSetStatus(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doSoundsChanged()), SLOT(doSoundsChanged()), Qt::QueuedConnection);
	// connect(parent, SIGNAL(doPingServer()), SLOT(doPingServer()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doToasterClicked(const QString&, bool)), SLOT(doToasterClicked(const QString&, bool)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doToasterScreenLocked(const QString&)), SLOT(doToasterScreenLocked(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doToasterIgnored(const QString&)), SLOT(doToasterIgnored(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doToasterSkipped(const QString&)), SLOT(doToasterSkipped(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doToasterDone(const QString&)), SLOT(doToasterDone(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doShowIgnoredToasters()), SLOT(doShowIgnoredToasters()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doScreenUnlocked()), SLOT(doScreenUnlocked()), Qt::QueuedConnection);
	// connect(parent, SIGNAL(doPlaySound(const QString&)), SLOT(doPlaySound(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(clearMoods()), SIGNAL(clearMoods()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doAuthAccept(const QString&)), SLOT(doAuthAccept(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doAuthDecline(const QString&)), SLOT(doAuthDecline(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOpenHistory(const QString&)), SLOT(doOpenHistory(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOpenProfile(const QString&)), SLOT(doOpenProfile(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doBlockContact(const QString&)), SLOT(doBlockContact(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doSetShowMoodChangesEnabled(const QString&, bool)), SLOT(doSetShowMoodChangesEnabled(const QString&, bool)), Qt::QueuedConnection);
	// connect(parent, SIGNAL(doOnlineConnected()), SLOT(doOnlineConnected()), Qt::QueuedConnection);
	connect(parent, SIGNAL(showPreferences()), SIGNAL(showYapsiPreferences()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doGetChatPreferences()), SIGNAL(doGetChatPreferences()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doGetChatAccounts()), SIGNAL(doGetChatAccounts()), Qt::DirectConnection);
	connect(parent, SIGNAL(doStopAccountUpdates()), SIGNAL(doStopAccountUpdates()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doApplyPreferences(const QString&)), SIGNAL(doApplyPreferences(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doApplyImmediatePreferences(const QString&)), SIGNAL(doApplyImmediatePreferences(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doShowTooltip(const QString&)), SLOT(doShowTooltip(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doShowXmlConsole(const QString&)), SLOT(doShowXmlConsole(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doScrollToContact(const QString&)), SLOT(doScrollToContact(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doRequestAuth(const QString&)), SLOT(doRequestAuth(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOpenChat(const QString&)), SLOT(doOpenChat(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doJInit()), SLOT(doJInit()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doJConnect(bool, const QString&)), SLOT(doJConnect(bool, const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doJDisconnect()), SLOT(doJDisconnect()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doJSendRaw(const QString&)), SLOT(doJSendRaw(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doJSetPresence(const QString&)), SLOT(doJSetPresence(const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doJSetCanConnectConnections(bool)), SLOT(doJSetCanConnectConnections(bool)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOnlineHiding()), SIGNAL(doOnlineHiding()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOnlineVisible()), SIGNAL(doOnlineVisible()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOnlineCreated(int)), SIGNAL(doOnlineCreated(int)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doChangeMain(const QString&, const QRect&)), SLOT(doChangeMain(const QString&, const QRect&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doActivateRoster()), SIGNAL(doActivateRoster()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOnlineDeactivated()), SIGNAL(doOnlineDeactivated()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doOptionChanged(const QString&, const QString&)), SLOT(onlineOptionChanged(const QString&, const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doFindContact(const QStringList&, const QString&)), SLOT(doFindContact(const QStringList&, const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doCancelSearch()), SLOT(doCancelSearch()), Qt::QueuedConnection);
	connect(parent, SIGNAL(doAddContact(const QString&, const QString&)), SLOT(doAddContact(const QString&, const QString&)), Qt::QueuedConnection);
	connect(parent, SIGNAL(doMessageBoxClosed(const QString&, int)), SLOT(doMessageBoxClosed(const QString&, int)), Qt::QueuedConnection);
	updateActiveProfile();

	queueChangedTimer_ = new QTimer(this);
	connect(queueChangedTimer_, SIGNAL(timeout()), SLOT(eventQueueChanged()));
	queueChangedTimer_->setSingleShot(true);
	queueChangedTimer_->setInterval(500);

	YaIPC::connect(this, SLOT(ipcMessage(const QString&)));

	checkForAliveOnlineProcessTimer_ = new QTimer(this);
	connect(checkForAliveOnlineProcessTimer_, SIGNAL(timeout()), SLOT(checkForAliveOnlineProcess()));
	checkForAliveOnlineProcessTimer_->setSingleShot(false);
	checkForAliveOnlineProcessTimer_->setInterval(500);
	checkForAliveOnlineProcessTimer_->start();
}

YaOnline::~YaOnline()
{
	rosterHandle(0);
}

void YaOnline::setController(PsiCon* controller)
{
	controller_ = controller;

	if (controller_) {
		Q_ASSERT(controller_->contactList());
		controller_->setYaOnline(this);
		addContactHelper_->setAccount(onlineAccount());
		addContactHelper_->setContactListModel(controller_->contactListModel());
		lastStatus_ = controller_->lastLoggedInStatusType();
		setDND(lastStatus_ == XMPP::Status::DND);
		setStatus(lastStatus_);

		connect(controller_->contactList(), SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
		connect(onlineAccount(), SIGNAL(updatedActivity()), SLOT(onlineAccountUpdated()));
		disconnect(GlobalEventQueue::instance(), SIGNAL(queueChanged()), this, SLOT(startEventQueueTimer()));
		connect(GlobalEventQueue::instance(),    SIGNAL(queueChanged()), this, SLOT(startEventQueueTimer()));
		connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(psiOptionChanged(const QString&)));
		connect(controller_->contactList(), SIGNAL(showOfflineChanged(bool)), SLOT(showOfflineChanged(bool)));
		disconnect(VCardFactory::instance(), SIGNAL(vcardChanged(const Jid&)), this, SLOT(vcardChanged(const Jid&)));
		connect(VCardFactory::instance(),    SIGNAL(vcardChanged(const Jid&)), this, SLOT(vcardChanged(const Jid&)));

		QSettings sUser(QSettings::UserScope, "Yandex", "Online");
		if (PsiOptions::instance()->getOption(migrateOldOptionsOptionPath).toInt() != 1) {
			sUser.setValue("chat/dock_stayontop", PsiOptions::instance()->getOption(alwaysOnTopOptionPath).toBool() ? "true" : "false");
			PsiOptions::instance()->setOption(migrateOldOptionsOptionPath, 1);
		}

		onlineOptionChanged("stayontop", sUser.value("chat/dock_stayontop", "false").toString());
		onlineOptionChanged("style", sUser.value("chat/dock_style", "ice").toString());
		onlineOptionChanged("sounds_enabled", sUser.value("sounds_enabled", true).toString());
		onlineOptionChanged("show_jabber_errors", sUser.value("show_jabber_errors", true).toString());
		psiOptionChanged(alwaysShowToastersOptionPath);
	}
}

void YaOnline::setMainWin(YaOnlineMainWin* mainWin)
{
	mainWin_ = mainWin;
}

void YaOnline::onlineObjectChanged()
{
	// doesn't seem to work as "init()" for some weird reason
	// server_->dynamicCall("init(const QString&)", QString());
}

void YaOnline::accountCountChanged()
{
	if (!controller_)
		return;

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		disconnect(account, SIGNAL(lastMailNotify(const XMPP::Message&)), this, SLOT(lastMailNotify(const XMPP::Message&)));
		connect(account,    SIGNAL(lastMailNotify(const XMPP::Message&)), this, SLOT(lastMailNotify(const XMPP::Message&)));
	}
}

void YaOnline::lastMailNotify(const XMPP::Message& msg)
{
	Q_ASSERT(!msg.lastMailNotify().isNull());

	Q_UNUSED(msg);
	// DummyStream stream;
	// XMPP::Stanza s = msg.toStanza(&stream);
	// server_->dynamicCall("lastMailNotify(const QString&)", s.toString());
}

struct YaOnlineEventData {
	int id;
	PsiAccount* account;
	QString jid;

	YaOnlineEventData()
		: id(-1)
		, account(0)
	{}

	bool isNull() const
	{
		return (id == -1) && (!account || jid.isEmpty());
	}
};

static YaOnlineEventData yaOnlineIdToEventData(PsiCon* controller, const QString& id)
{
	YaOnlineEventData result;

	QStringList data = id.split(":");
	Q_ASSERT(data.size() == 2);
	Q_ASSERT(data[0] == "yachat");
	if (data.size() != 2 || data[0] != "yachat")
		return result;

	QDomDocument doc;
	if (!doc.setContent(QCA::Base64().decodeString(data[1])))
		return result;

	QDomElement root = doc.documentElement();
	if (root.tagName() != "notify")
		return result;

	result.id = root.attribute("id").toInt();
	result.account = controller->contactList()->getAccount(root.attribute("account"));
	result.jid = root.attribute("jid");
	return result;
}

static PsiContact* yaOnlineIdToContact(PsiCon* controller, const QString& id)
{
	YaOnlineEventData eventData = yaOnlineIdToEventData(controller, id);
	if (!eventData.isNull() && eventData.account) {
		XMPP::Jid jid(eventData.jid);
		return eventData.account->findContact(jid.bare());
	}
	return 0;
}


void YaOnline::doToasterClicked(const QString& id, bool activate)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			bool deleteEvent = false;
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			event->setShownInOnline(true);

			if (event->type() == PsiEvent::Auth) {
				AuthEvent* authEvent = static_cast<AuthEvent*>(event);
				if (authEvent->authType() == "subscribe") {
					notify(eventData.id, event);
					return;
				}

				deleteEvent = true;
			}
			else if (event->type() == PsiEvent::Mood) {
				deleteEvent = true;
			}

			if (deleteEvent) {
				event->account()->eventQueue()->dequeue(event);
				controller_->yaUnreadMessagesManager()->eventRead(event);
				delete event;
			}
		}

		YaPopupNotification::openNotify(-1, eventData.account, eventData.jid,
		                                activate ? UserAction : UserPassiveAction);
	}

	startEventQueueTimer();
}

void YaOnline::doToasterScreenLocked(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			event->setShownInOnline(false);
			emit event->account()->eventQueue()->queueChanged();
		}
	}

	startEventQueueTimer();
}

bool YaOnline::doToasterIgnored(PsiAccount* account, const XMPP::Jid& jid)
{
	bool found = false;
	foreach(TabDlg* tabDlg, controller_->tabManager()->tabSets()) {
		TabbableWidget* tab = tabDlg->getTabPointer(account, jid.full());
		YaChatDlg* chat = tab ? dynamic_cast<YaChatDlg*>(tab) : 0;
		if (chat) {
			found = true;
			if (!tabDlg->isVisible()) {
				tabDlg->showWithoutActivation();
			}

			if (!chat->isActiveWindow()) {
				tabDlg->selectTab(chat);
			}
			chat->addPendingMessage();
		}
	}

	return found;
}

void YaOnline::doToasterIgnored(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			event->setShownInOnline(true);

			if (event->type() == PsiEvent::Message) {
				Q_ASSERT(event->account());
				bool found = doToasterIgnored(event->account(), event->from());

				if (!found && eventData.account && !controller_->tabManager()->tabSets().isEmpty()) {
					eventData.account->actionOpenSavedChat(event->from().full());
					doToasterIgnored(event->account(), event->from());
				}
			}
			else if (event->type() == PsiEvent::Auth) {
				// doToasterScreenLocked(id);
			}
			else if (event->type() == PsiEvent::Mood) {
				// nothing
			}
			else {
				Q_ASSERT(false);
			}
		}
	}

	startEventQueueTimer();
}

void YaOnline::doToasterSkipped(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			event->account()->eventQueue()->dequeue(event);
			controller_->yaUnreadMessagesManager()->eventRead(event);
			delete event;
		}
	}

	startEventQueueTimer();
}

void YaOnline::doToasterDone(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			event->setShownInOnline(true);
			emit event->account()->eventQueue()->queueChanged();
		}
	}

	startEventQueueTimer();
}

void YaOnline::doShowIgnoredToasters()
{
	if (!controller_)
		return;

	static bool showingIgnoredToasters = false;
	if (showingIgnoredToasters)
		return;

	showingIgnoredToasters = true;

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		// when chat to contact is opened it could delete some
		// more unread events related to that contact
		if (!GlobalEventQueue::instance()->ids().contains(id))
			continue;

		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		Q_ASSERT(event);
		if (!event)
			continue;

		if (event->type() == PsiEvent::Message) {
			if (event->account()) {
				// we need to open all the chats without activating them in the process
				event->account()->actionOpenSavedChat(event->from().full());
				doToasterIgnored(event->account(), event->from());
			}
		}
		else if (event->type() == PsiEvent::Auth) {
			// if (!event->shownInOnline()) {
			// 	notify(id, event);
			// }
		}
		else if (event->type() == PsiEvent::Mood) {
			// nothing
		}
		else {
			Q_ASSERT(false);
		}
	}

	foreach(TabDlg* tabDlg, controller_->tabManager()->tabSets()) {
		bringToFront(tabDlg);
	}

	showingIgnoredToasters = false;
}

static QString yaOnlineNotifyMid(int id, PsiAccount* account, const XMPP::Jid& jid)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("notify");
	doc.appendChild(root);
	root.setAttribute("id",      QString::number(id));
	root.setAttribute("account", account->id());
	root.setAttribute("jid",     jid.full());

	return QString("yachat:%1")
	       .arg(QCA::Base64().encodeString(doc.toString()));
}

static QString yaOnlineNotifyMid(int id, PsiEvent* event)
{
	return yaOnlineNotifyMid(id, event->account(), event->from());
}

void YaOnline::notifyAllUnshownEvents()
{
	if (!controller_)
		return;

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		if (!GlobalEventQueue::instance()->ids().contains(id))
			continue;

		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		if (event->shownInOnline()) {
			doToasterIgnored(yaOnlineNotifyMid(id, event));
			continue;
		}

		notify(id, event);
	}

	startEventQueueTimer();
}

void YaOnline::doScreenUnlocked()
{
	if (!controller_)
		return;

	notifyAllUnshownEvents();
}

static QVariantMap toasterParams(const QString& type, PsiAccount* account, const XMPP::Jid& jid, const QString& _message, const QDateTime& timeStamp, const QString& callbackId)
{
	QString message = Ya::limitText(_message, 300);
	message = TextUtil::plain2richSimple(message);
	bool processMessageNotifierText = true;

	bool showToaster = false;
	if (type == "chat") {
		showToaster = PsiOptions::instance()->getOption("options.ya.popups.message.enable").toBool();
	}
	else if (type == "mood") {
		showToaster = PsiOptions::instance()->getOption("options.ya.popups.moods.enable").toBool();
		processMessageNotifierText = false;
	}
	else if (type == "subscribe" || type == "subscribed" || type == "unsubscribed") {
		showToaster = true;
	}
	else {
		Q_ASSERT(false);
	}

	QString emoticonified = message;
	if (processMessageNotifierText) {
		emoticonified = Ya::messageNotifierText(message);
	}
	emoticonified = TextUtil::emoticonifyForYaOnline(emoticonified);

	QVariantMap map;
	map["id"] = callbackId;
	map["type"] = type;
	map["text"] = emoticonified;
	map["text_original"] = message;
	map["contact_name"] = Ya::VisualUtil::contactName(account, jid);
	map["avatar"] = Ya::VisualUtil::scaledAvatarPath(account, jid);
	map["timestamp"] = timeStamp;
	map["show_toaster"] = showToaster;
	return map;
}

void YaOnline::showToaster(const QString& type, PsiAccount* account, const XMPP::Jid& jid, const QString& message, const QDateTime& timeStamp, const QString& callbackId)
{
	QVariantMap map = toasterParams(type, account, jid, message, timeStamp, callbackId);
	map["gender"] = Ya::VisualUtil::contactGenderString(account, jid);

	QString json = CuteJson::variantToJson(map);
	server_->dynamicCall("showToaster(const QString&)", json);
}

void YaOnline::notify(int id, PsiEvent* event)
{
	if (!controller_)
		return;
	if (!doStartActions_)
		return;

	// event->setShownInOnline(true);
	emit event->account()->eventQueue()->queueChanged();

	if (event->type() == PsiEvent::Mood) {
		MoodEvent* moodEvent = static_cast<MoodEvent*>(event);

		showToaster("mood",
		            event->account(), event->from(),
		            moodEvent->mood().trimmed(),
		            moodEvent->timeStamp(),
		            yaOnlineNotifyMid(id, event));
		return;
	}

	if (event->type() == PsiEvent::Auth) {
		AuthEvent* authEvent = static_cast<AuthEvent*>(event);
		QVariantMap map = toasterParams(authEvent->authType(),
		                                event->account(),
		                                event->jid(),
		                                QString(),
		                                QDateTime::currentDateTime(),
		                                yaOnlineNotifyMid(id, event));
		map["gender"] = Ya::VisualUtil::contactGenderString(event->account(), event->from());

		QString json = CuteJson::variantToJson(map);
		server_->dynamicCall("authRequest(const QString&)", json);
		return;
	}

	// only message events are supported for now
	if (event->type() != PsiEvent::Message) {
		return;
	}

	XMPP::Message m;
	m.setFrom("lastmail.ya.ru/yachat");
	YaLastMail lastMail;

	lastMail.subject   = event->description();
	lastMail.timeStamp = event->timeStamp();
	lastMail.mid       = yaOnlineNotifyMid(id, event);
	m.setLastMailNotify(lastMail);

	showToaster("chat",
	            event->account(), event->from(),
	            lastMail.subject,
	            lastMail.timeStamp,
	            lastMail.mid);
}

void YaOnline::closeNotify(int id, PsiEvent* event)
{
	if (!controller_)
		return;

	if (event->type() == PsiEvent::Auth ||
	    event->type() == PsiEvent::Mood ||
	    event->type() == PsiEvent::Message)
	{
		server_->dynamicCall("removeToaster(const QString&)", yaOnlineNotifyMid(id, event));
		return;
	}
	else {
		Q_ASSERT(false);
	}
}

void YaOnline::openUrl(const QString& url, bool isYandex)
{
	server_->dynamicCall("openUrl(const QString&, bool)",
	                     url, isYandex);
}

void YaOnline::openMail(const QString& email)
{
	server_->dynamicCall("openMail(const QString&)",
	                     email);
}

void YaOnline::clearedMessageHistory()
{
	server_->dynamicCall("eraseHistory()");
}

void YaOnline::ipcMessage(const QString& message)
{
	static bool uninstalling = false;
	if (message == "quit:uninstalling") {
		if (uninstalling) {
			return;
		}
		uninstalling = true;
		PsiLogger::instance()->log("YaOnline::ipcMessage(): uninstalling");
		server_->dynamicCall("uninstall(const QString&)", QString());
	}
	else if (message == "quit:installing") {
		PsiLogger::instance()->log("YaOnline::ipcMessage(): installing");
		server_->shutdown();
	}
}

static QString getPSIDFromProcessId(DWORD process_id) {
	QString result = 0;

	HANDLE process_handle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, process_id);
	if(process_handle) {
		HANDLE process_token;
		if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &process_token)) {
			DWORD cb_info = 0;
			if(!GetTokenInformation(process_token, TokenOwner, 0, 0, &cb_info) &&
			   ERROR_INSUFFICIENT_BUFFER == GetLastError())
			{
				TOKEN_OWNER* pto = (TOKEN_OWNER*)malloc(cb_info);
				if(GetTokenInformation(process_token, TokenOwner, pto, cb_info, &cb_info)) {
					char* string_sid = 0;
					if(ConvertSidToStringSidA(pto->Owner, &string_sid)) {
						result = string_sid;
						LocalFree(string_sid);
					}
				}
				free(pto);
			}

			CloseHandle(process_token);
		}

		CloseHandle(process_handle);
	}

	return result;
}

static bool processIsRunning(const QString& processName)
{
	bool processFound = false;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 pe;
	memset(&pe, 0, sizeof(PROCESSENTRY32));
	pe.dwSize = sizeof(PROCESSENTRY32);

	bool result = false;
	result = Process32First(snapshot, &pe);
	while (result) {
		QString exeFile = QString::fromWCharArray(pe.szExeFile);
		if (processName.toLower() == exeFile.toLower()) {
			QString self_owner = getPSIDFromProcessId(GetCurrentProcessId());
			QString process_owner = getPSIDFromProcessId(pe.th32ProcessID);
			if(self_owner == process_owner) {
				processFound = true;
				break;
			}
		}
		result = Process32Next(snapshot, &pe);
	}
	CloseHandle(snapshot);
	return processFound;
}

bool YaOnline::onlineIsRunning()
{
	return processIsRunning("online.exe");
}

void YaOnline::checkForAliveOnlineProcess()
{
	if (!onlineIsRunning()) {
		PsiLogger::instance()->log("YaOnline::checkForAliveOnlineProcess(): shutting down");
		server_->shutdown();
	}
}

PsiAccount* YaOnline::onlineAccount() const
{
	if (!controller_)
		return 0;

	PsiAccount* result = controller_->contactList()->onlineAccount();
	Q_ASSERT(result);
	Q_ASSERT(!result->userAccount().saveable);
	return result;
}

void YaOnline::updateOnlineAccount()
{
	if (!onlineAccount())
		return;

	UserAccount acc = onlineAccount()->userAccount();
	XMPP::Jid jid("foo@bar");
	jid.setDomain("ya.ru");
	jid.setNode(ycuApi_->getUsername());
	if (!jid.node().isEmpty())
		acc.jid = jid.full();
	else
		acc.jid = QString();
	acc.pass = ycuApi_->getPassword();
	acc.opt_enabled = !acc.jid.isEmpty() && !acc.pass.isEmpty();
	onlineAccount()->setUserAccount(acc);
}

void YaOnline::updateProxySettings()
{
	if (!controller_)
		return;

	ProxyItemList proxyItems;
	ProxyItem proxy;
	proxy.id = 0;
	proxy.name = "Proxy";
	proxy.type = "http";
	proxy.settings.host = ycuApi_->getProxyHost();
	proxy.settings.port = ycuApi_->getProxyPort();
	proxy.settings.useAuth = ycuApi_->getProxyAuthEnabled();
	proxy.settings.user = ycuApi_->getProxyLogin();
	proxy.settings.pass = ycuApi_->getProxyPassword();
	if (ycuApi_->getProxyEnabled()) {
		proxyItems << proxy;
	}

	controller_->proxy()->setItemList(proxyItems);

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		// Q_ASSERT(!account->isAvailable());
		UserAccount acc = account->userAccount();
		acc.proxy_index = proxyItems.count();
		account->setUserAccount(acc);
	}
}

void YaOnline::updateMiscSettings()
{
	// useSound = ycuApi_->getSoundsEnabled();
}

void YaOnline::rereadSettings()
{
	ycuApi_->rereadData();

	updateOnlineAccount();
	updateProxySettings();
	updateMiscSettings();

	// TODO
}

void YaOnline::disconnectRereadSettingsAndReconnect(bool reconnect)
{
	doSetOfflineMode();
	rereadSettings();
	if (reconnect) {
		emit forceStatus(lastStatus_);
	}
}

void YaOnline::doSetOfflineMode()
{
	if (!controller_)
		return;

	foreach(PsiAccount* account, controller_->contactList()->enabledAccounts()) {
		if (account->isActive()) {
			account->forceDisconnect();
		}
	}
}

void YaOnline::doSetDND(bool isDND)
{
	doSetStatus(isDND ? XMPP::Status::DND : XMPP::Status::Online);
}

void YaOnline::doSetStatus(const QString& status)
{
	if (status == "dnd")
		doSetStatus(XMPP::Status::DND);
	else if (status == "away")
		doSetStatus(XMPP::Status::Away);
	else
		doSetStatus(XMPP::Status::Online);
}

void YaOnline::doSetStatus(XMPP::Status::Type statusType)
{
	lastStatus_ = statusType;
	if (!controller_)
		return;

	emit forceManualStatus(lastStatus_);
}

/**
 * Ensures that all enabled accounts are in active state, if they don't have
 * auto-connects explicitly disabled (in order to avoid resource conflicts)
 */
void YaOnline::doOnlineConnected()
{
	if (!controller_)
		return;

	foreach(PsiAccount* account, controller_->contactList()->enabledAccounts()) {
		if (!account->isAvailable() && !account->disableAutoConnect()) {
			account->setStatus(makeStatus(lastStatus() == XMPP::Status::DND ? XMPP::Status::DND : XMPP::Status::Online,
			                              controller_->currentStatusMessage()), false);
		}
	}
}

void YaOnline::doSoundsChanged()
{
	ycuApi_->rereadData();
	updateMiscSettings();
}

void YaOnline::doQuit()
{
	// ONLINE-2530
	// doHideSidebar();
	server_->doQuit();
}

void YaOnline::doHideSidebar()
{
	if (!mainWin_.isNull()) {
		mainWin_->hideOnline();
	}
}

void YaOnline::showSidebar()
{
	server_->dynamicCall("showSidebar(const QString&)", QString());
}

void YaOnline::showPreferences()
{
	server_->dynamicCall("showPreferences(const QString&)", QString());
}

void YaOnline::setPreferences(const QString& xml)
{
	server_->dynamicCall("setPreferences(const QString&)", xml);
}

void YaOnline::setAccounts(const QString& xml)
{
	server_->dynamicCall("setAccounts(const QString&)", xml);
}

void YaOnline::setImmediatePreferences(const QString& xml)
{
	server_->dynamicCall("setImmediatePreferences(const QString&)", xml);
}

void YaOnline::doPingServer()
{
	if (!controller_)
		return;

	foreach(PsiAccount* account, controller_->contactList()->enabledAccounts()) {
		account->pingServer();
	}
}

void YaOnline::setDND(bool isDND)
{
	server_->dynamicCall("setDND(bool)", isDND);
}

static QString statusTypeToString(XMPP::Status::Type statusType)
{
	QString statusName;
	if (statusType == XMPP::Status::DND)
		statusName = "dnd";
	else if (statusType == XMPP::Status::Away)
		statusName = "away";
	else
		statusName = "online";
	return statusName;
}

void YaOnline::setStatus(XMPP::Status::Type statusType)
{
	server_->dynamicCall("setStatus(const QString&)", statusTypeToString(statusType));
}

void YaOnline::setCurrentlyVisibleStatus(XMPP::Status::Type statusType)
{
	server_->dynamicCall("setCurrentlyVisibleStatus(const QString&)", statusTypeToString(statusType));
}

// void YaOnline::doPlaySound(const QString& type)
// {
// 	if (!onlineAccount())
// 		return;
//
// 	if (type == "message") {
// 		onlineAccount()->playSound(eChat1);
// 	}
// }

static const int maximumEventCount = 100;
static const int maximumQueueChangedDelay = 30; // in seconds
static QStringList skipEvents(QList<int>& goodIds, int type)
{
	QStringList skipIds;

	if (goodIds.count() > maximumEventCount) {
		QMutableListIterator<int> it(goodIds);
		while (it.hasNext()) {
			int id = it.next();
			PsiEvent* event = GlobalEventQueue::instance()->peek(id);
			Q_ASSERT(event);
			if (event->type() == type) {
				skipIds << yaOnlineNotifyMid(id, event);
				it.remove();
			}

			if (goodIds.count() <= maximumEventCount)
				break;
		}
	}

	return skipIds;
}

void YaOnline::startEventQueueTimer()
{
	if (queueChangedTimerStartTime_.isNull())
		queueChangedTimerStartTime_ = QDateTime::currentDateTime();

	if (queueChangedTimerStartTime_.secsTo(QDateTime::currentDateTime()) > maximumQueueChangedDelay)
		eventQueueChanged();
	else
		queueChangedTimer_->start();
}

void YaOnline::eventQueueChanged()
{
	queueChangedTimerStartTime_ = QDateTime();
	queueChangedTimer_->stop();

	static bool inEventQueueChanged = false;

	if (!controller_ || !doStartActions_)
		return;

	if (!controller_->contactList() || !controller_->contactList()->accountsLoaded())
		return;

	if (inEventQueueChanged)
		return;
	inEventQueueChanged = true;

	QList<int> goodIds;

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		Q_ASSERT(event);
		if (event->type() == PsiEvent::Message ||
		    event->type() == PsiEvent::Auth ||
		    event->type() == PsiEvent::Mood)
		{
			goodIds += id;
		}
	}

	{
		QStringList skipIds;
		skipIds += skipEvents(goodIds, PsiEvent::Mood);
		skipIds += skipEvents(goodIds, PsiEvent::Auth);

		if (goodIds.count() > maximumEventCount) {
			bool onlyMessageEvents = true;
			bool lastEventIsMessage = false;
			foreach(int id, goodIds) {
				PsiEvent* event = GlobalEventQueue::instance()->peek(id);
				if (event->type() != PsiEvent::Message) {
					onlyMessageEvents = false;
					break;
				}
			}

			if (!goodIds.isEmpty()) {
				PsiEvent* event = GlobalEventQueue::instance()->peek(goodIds.last());
				lastEventIsMessage = event->type() == PsiEvent::Message;
			}

			if (onlyMessageEvents && lastEventIsMessage) {
				skipIds += skipEvents(goodIds, PsiEvent::Message);
			}
		}

		// PsiLogger::instance()->log(QString("YaOnline::eventQueueChanged(): skipIds.count() = %1").arg(skipIds.count()));
		foreach(QString id, skipIds) {
			doToasterSkipped(id);
		}
	}

	QStringList mids;
	static QString separator = " ";

	int eventCount = 0;
	foreach(int id, goodIds) {
		PsiEvent* event = GlobalEventQueue::instance()->peek(id);

		Q_ASSERT(event);
		eventCount++;

		QString mid = yaOnlineNotifyMid(id, event);
		Q_ASSERT(!mid.contains(separator));

		Q_ASSERT(eventCount <= maximumEventCount);
		mids += mid;
	}

	// PsiLogger::instance()->log(QString("YaOnline::eventQueueChanged(): mids.count() = %1").arg(mids.count()));
	server_->dynamicCall("setIgnoredToasters(const QString&)", mids.join(separator));

	inEventQueueChanged = false;
}

void YaOnline::onlineAccountUpdated()
{
	if (!onlineAccount())
		return;

	if (onlineAccountConnected_ != onlineAccount()->isAvailable() ||
	    onlineAccountDnd_ != (lastStatus_ == XMPP::Status::DND))
	{
		onlineAccountConnected_ = onlineAccount()->isAvailable();
		onlineAccountDnd_ = (lastStatus_ == XMPP::Status::DND);

		server_->dynamicCall("onlineAccountUpdated(bool, bool)", onlineAccountConnected_, onlineAccountDnd_);
	}
}

XMPP::Status::Type YaOnline::lastStatus() const
{
	return lastStatus_;
}

void YaOnline::updateActiveProfile()
{
	LOG_TRACE;
	Q_ASSERT(ycuApi_);
	ycuApi_->rereadData();
	onlinePasswordVerified_ = false;

	activeProfile_ = ycuApi_->getUsername();
	PsiLogger::instance()->log(QString("YaOnline::updateActiveProfile():1 activeProfile_ = '%1'").arg(activeProfile_));
	if (!activeProfile_.isEmpty()) {
		activeProfile_ = "ya_" + activeProfile_;

		if (!QDir(pathToProfile(activeProfile_)).exists() && QDir(pathToProfile("default")).exists()) {
			Ya::copyDir(pathToProfile("default"), pathToProfile(activeProfile_));
		}
	}
	else {
		activeProfile_ = "default";
	}
	PsiLogger::instance()->log(QString("YaOnline::updateActiveProfile():2 activeProfile_ = '%1'").arg(activeProfile_));
}

QString YaOnline::activeProfile() const
{
	if (activeProfile_.isEmpty()) {
		((YaOnline*)this)->updateActiveProfile();
	}

	return activeProfile_;
}

void YaOnline::doAuthAccept(const QString& id)
{
	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull() && eventData.account) {
		eventData.account->dj_auth(eventData.jid);
	}
}

void YaOnline::doAuthDecline(const QString& id)
{
	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull() && eventData.account) {
		eventData.account->dj_deny(eventData.jid);
	}
}

void YaOnline::doOpenHistory(const QString& id)
{
	PsiContact* contact = yaOnlineIdToContact(controller_, id);
	if (contact) {
		contact->history();
	}
}

void YaOnline::doOpenProfile(const QString& id)
{
	PsiContact* contact = yaOnlineIdToContact(controller_, id);
	if (contact) {
		contact->yaProfile();
	}
}

void YaOnline::doBlockContact(const QString& id)
{
	PsiContact* contact = yaOnlineIdToContact(controller_, id);
	if (contact) {
		contact->toggleBlockedState();
	}
}

void YaOnline::doSetShowMoodChangesEnabled(const QString& id, bool enabled)
{
	PsiContact* contact = yaOnlineIdToContact(controller_, id);
	if (contact) {
		contact->setMoodNotificationsEnabled(enabled);
	}
}

static QVariantMap contactParams(const QString& id, PsiAccount* account, const XMPP::Jid& jid, PsiContact* contact)
{
	const XMPP::VCard* vcard = VCardFactory::instance()->vcard(jid);

	QVariantMap map;
	map["id"] = id;
	map["nick"] = contact ? contact->name() : Ya::nickFromVCard(jid, vcard);
	map["jid"] = jid.bare();
	map["avatar"] = Ya::VisualUtil::scaledAvatarPath(account, jid);
	map["enable_history"] = Ya::historyAvailable(account, jid);
	map["enable_profile"] = Ya::isYaJid(jid);
	map["enable_block"] = contact ? (contact->blockAvailable() && !contact->isBlocked()) : false;
	map["gender"] = Ya::VisualUtil::contactGenderString(account, jid);
	map["show_mood_changes"] = contact ? contact->moodNotificationsEnabled() : false;

	bool stubTextWasUsed;
	QString text = Ya::VisualUtil::contactAgeLocation(account, jid, &stubTextWasUsed);
	map["data"] = text;
	map["no_data"] = stubTextWasUsed;
	return map;
}

void YaOnline::doShowTooltip(const QString& id)
{
	PsiContact* contact = yaOnlineIdToContact(controller_, id);
	if (contact) {
		QVariantMap map = contactParams(id, contact->account(), contact->jid(), contact);
		QString json = CuteJson::variantToJson(map);
		server_->dynamicCall("showTooltip(const QString&)", json);
	}
}

void YaOnline::setSelfInfo(PsiContact* contact)
{
	if (contact) {
		QVariantMap map = contactParams(yaOnlineNotifyMid(-1, contact->account(), contact->jid()),
		                                contact->account(), contact->jid(), contact);
		QString json = CuteJson::variantToJson(map);
		server_->dynamicCall("setSelfInfo(const QString&)", json);
	}
}

void YaOnline::doShowXmlConsole(const QString& accountId)
{
	if (!controller_)
		return;

	PsiAccount* account = controller_->contactList()->getAccount(accountId);
	if (account) {
		account->showXmlConsole();
	}
}

void YaOnline::incomingStanza(const QString& xml)
{
	jabberNotify("message", xml);
}

void YaOnline::showConnectionToaster(PsiAccount* account, const QString& error, bool showingError)
{
	Q_ASSERT(account);
	if (!account)
		return;

	if (!PsiOptions::instance()->getOption("options.ya.popups.connection.enable").toBool()) {
		return;
	}

	QVariantMap map;
	map["id"] = yaOnlineNotifyMid(-1, account, account->jid());
	map["jid"] = account->jid().bare();
	map["nick"] = account->nick();
	map["error"] = error;
	map["is_error"] = showingError;
	map["account_id"] = account->id();

	QString json = CuteJson::variantToJson(map);
	server_->dynamicCall("showConnectionToaster(const QString&)", json);
}

void YaOnline::showSuccessfullyConnectedToaster(PsiAccount* account)
{
	showConnectionToaster(account, QString(), false);
}

void YaOnline::showConnectionErrorToaster(PsiAccount* account, const QString& error)
{
	showConnectionToaster(account, error, true);
}

void YaOnline::doJInit()
{
}

void YaOnline::doJConnect(bool useSsl, const QString& pemPath)
{
	LOG_TRACE;
	// make sure that main account information wasn't updated
	// since the last time we've read it. if it was, treat it
	// as 'userChanged' event in order to make sure we use the
	// proper profile directory
	bool oldOnlinePasswordVerified_ = onlinePasswordVerified_;
	QString oldActiveProfile_ = activeProfile_;
	updateActiveProfile();
	if (oldActiveProfile_ == activeProfile_) {
		onlinePasswordVerified_ = oldOnlinePasswordVerified_;
	}
	else {
		server_->userChanged();
	}

	useSsl_ = useSsl;
	sslPemPath_ = pemPath;

	rereadSettings();
	updateOnlineAccount();
	updateProxySettings();
	doSetStatus(lastStatus_);
}

void YaOnline::doJDisconnect()
{
	LOG_TRACE;
	if (onlineAccount() && onlineAccount()->isActive()) {
		onlineAccount()->forceDisconnect();
	}
}

void YaOnline::doJSendRaw(const QString& xml)
{
	if (!onlineAccount())
		return;

	onlineAccount()->client()->send(xml);
}

void YaOnline::doJSetPresence(const QString& type)
{
	LOG_TRACE;
	doSetStatus(type);
}

void YaOnline::doJSetCanConnectConnections(bool canConnectConnections)
{
	bool changed = canConnectConnections_ != canConnectConnections;
	canConnectConnections_ = canConnectConnections;
	if (changed) {
		foreach(PsiAccount* account, controller_->contactList()->enabledAccounts()) {
			QString jid = account->jid().full();
			if (account != onlineAccount()) {
				if (canConnectConnections)
					account->autoLogin();
				else
					account->forceDisconnect();
			}
		}
	}
}

void YaOnline::jabberNotify(const QString& type, const QString& param1, const QString& param2)
{
	server_->dynamicCall("jabberNotify(const QString&, const QString&, const QString&)", type, param1, param2);
}

bool YaOnline::useSsl() const
{
	return useSsl_;
}

QString YaOnline::sslPemPath() const
{
	return sslPemPath_;
}

bool YaOnline::onlinePasswordVerified() const
{
	return canConnectConnections_;
}

void YaOnline::doStartActions()
{
	Q_ASSERT(!doStartActions_);
	if (doStartActions_)
		return;
	doStartActions_ = true;

	if (controller_) {
		controller_->restoreSavedChats();
	}

	// moved comment from PsiCon::init()
	// Loading of individual account's events is in QTimer::singleShot(0, d, SLOT(loadQueue())),
	// so we're postponing re-sending of toasters till 10 seconds later
	notifyAllUnshownEvents();
	startEventQueueTimer();
}

void YaOnline::doChangeMain(const QString& mainApp, const QRect& onlineRect)
{
	if (mainApp == "chat") {
		emit showRelativeToOnline(onlineRect);
	}
	else {
		doShowRoster(false);
	}
}

void YaOnline::psiOptionChanged(const QString& option)
{
	if (option == alwaysOnTopOptionPath) {
		optionChanged("stayontop", PsiOptions::instance()->getOption(alwaysOnTopOptionPath).toBool() ? "true" : "false");
	}
	else if (option == chatBackgroundOptionPath) {
		optionChanged("style", PsiOptions::instance()->getOption(chatBackgroundOptionPath).toString());
	}
	else if (option == alwaysShowToastersOptionPath) {
		optionChanged("always_show_toasters", PsiOptions::instance()->getOption(alwaysShowToastersOptionPath).toString());
	}
}

void YaOnline::showOfflineChanged(bool showOffline)
{
	optionChanged("show_offline", showOffline ? "true" : "false");
}

void YaOnline::onlineOptionChanged(const QString& name, const QString& value)
{
	Q_ASSERT(!value.isEmpty());
	if (name == "stayontop") {
		PsiOptions::instance()->setOption(alwaysOnTopOptionPath, value == "true");
	}
	else if (name == "style") {
		PsiOptions::instance()->setOption("options.ya.chat-background", value);
	}
	else if (name == "sounds_enabled") {
		PsiOptions::instance()->setOption("options.ui.notifications.sounds.enable", (value == "true" || value == "1"));
	}
	else if (name == "show_jabber_errors") {
		PsiOptions::instance()->setOption("options.ya.popups.connection.enable", (value == "true" || value == "1"));
	}
}

void YaOnline::optionChanged(const QString& name, const QString& value)
{
	QVariantMap map;
	map["name"] = name;
	map["value"] = value;

	QString json = CuteJson::variantToJson(map);
	server_->dynamicCall("optionChanged(const QString&)", json);
}

void YaOnline::setVisible()
{
	server_->dynamicCall("setVisible(const QString&)", QString());
}

void YaOnline::showOnline(const QRect& rosterRect, bool animate, const QVariantMap& params)
{
	QVariantMap map;
	map["chat_x"] = rosterRect.x();
	map["chat_y"] = rosterRect.y();
	map["chat_w"] = rosterRect.width();
	map["chat_h"] = rosterRect.height();
	map["animate"] = animate;

	QMapIterator<QString, QVariant> it(params);
	while (it.hasNext()) {
		it.next();
		map[it.key()] = it.value();
	}

	QString json = CuteJson::variantToJson(map);
	server_->dynamicCall("showOnline(const QString&)", json);
}

void YaOnline::hideOnline()
{
	server_->dynamicCall("hideOnline(const QString&)", QString());
}

void YaOnline::activateOnline()
{
	server_->dynamicCall("activateOnline(const QString&)", QString());
}

void YaOnline::deactivateOnline()
{
	server_->dynamicCall("deactivateOnline(const QString&)", QString());
}

void YaOnline::doShowRoster(bool visible)
{
	if (visible)
		emit showRoster();
	else
		emit hideRoster();
}

bool YaOnline::onlineShouldBeVisible() const
{
	QSettings sUser(QSettings::UserScope, "Yandex", "Online");
	return sUser.value("chat/dock_online_open", "true").toString() == "true";
}

bool YaOnline::chatIsMain() const
{
	QSettings sUser(QSettings::UserScope, "Yandex", "Online");
	return sUser.value("chat/dock_main", "chat").toString() == "chat";
}

QRect YaOnline::onlineSidebarGeometry() const
{
	QSettings sUser(QSettings::UserScope, "Yandex", "Online");
	int x = sUser.value("chat/dock_online_x", 0).toInt();
	int y = sUser.value("chat/dock_online_y", 0).toInt();
	int w = sUser.value("chat/dock_online_w", 0).toInt();
	int h = sUser.value("chat/dock_online_h", 0).toInt();
	return QRect(x, y, w, h);
}

void YaOnline::onlineMoved()
{
	server_->dynamicCall("onlineMoved(const QString&)", QString());
}

void YaOnline::setMainToOnline()
{
	server_->dynamicCall("setMainToOnline(const QString&)", QString());
}

void YaOnline::login()
{
	server_->dynamicCall("login(const QString&)", QString());
}

void YaOnline::doFindContact(const QStringList& jids, const QString& type)
{
	QStringList tmp;
	foreach(QString j, jids) {
		XMPP::Jid jid(j);
		tmp << jid.bare();
	}
	addContactHelper_->findContact(tmp, type);
}

void YaOnline::doCancelSearch()
{
	addContactHelper_->cancelSearch();
}

void YaOnline::doAddContact(const QString& jid, const QString& group)
{
	addContactHelper_->addContact(jid, group);
}

void YaOnline::addContactClicked(QRect addButtonRect, QRect windowRect)
{
	addContactHelper_->addContactClicked(addButtonRect, windowRect);
}

void YaOnline::addContact(int rosterX, int y, int rosterW, const QStringList& groups)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("groups");
	doc.appendChild(root);
	foreach(QString g, groups) {
		root.appendChild(XMLHelper::textTag(doc, "item", g));
	}

	QVariantMap map;
	map["x"] = rosterX;
	map["y"] = y;
	map["roster_w"] = rosterW;

	QString json = CuteJson::variantToJson(map);
	server_->dynamicCall("addContact(const QString&, const QString&)",
	                     json,
	                     doc.toString());
}

void YaOnline::foundContact(PsiAccount* account, const XMPP::Jid& jid, bool inRoster, bool authorized, bool selfContact)
{
	QVariantMap map = contactParams(QString(), account, jid, 0);
	map["in_roster"] = inRoster;
	map["authorized"] = authorized;
	map["my_jid"] = selfContact;

	QString json = CuteJson::variantToJson(map);
	server_->dynamicCall("foundContact(const QString&)", json);
}

void YaOnline::searchComplete()
{
	server_->dynamicCall("searchComplete(const QString&)", QString());
}

void YaOnline::doRequestAuth(const QString& jid)
{
	if (onlineAccount()) {
		onlineAccount()->dj_authReq(XMPP::Jid(jid).bare());
	}
}

void YaOnline::doScrollToContact(const QString& jid)
{
	PsiContact* c = Ya::findContact(controller_, jid);
	if (c) {
		emit scrollToContact(c);
	}
}

void YaOnline::doOpenChat(const QString& jid)
{
	PsiContact* c = Ya::findContact(controller_, jid);
	if (c) {
		c->openChat();
	}
}

void YaOnline::messageBox(const QString& parent, const QString& id, const QString& caption, const QString& text, const QStringList& buttons, QMessageBox::Icon icon)
{
	Q_ASSERT(!id.isEmpty());
	Q_ASSERT(!caption.isEmpty());
	Q_ASSERT(!text.isEmpty());
	Q_ASSERT(!buttons.isEmpty());

	QVariantMap map;
	map["parent_window"] = parent;
	map["id"] = QCA::Base64().encodeString(id);
	map["caption"] = caption;
	map["text"] = text;
	QString iconText;
	switch (icon) {
	case QMessageBox::Question:
		iconText = "question";
		break;
	case QMessageBox::Warning:
		iconText = "warning";
		break;
	case QMessageBox::Critical:
		iconText = "error";
		break;
	case QMessageBox::Information:
	default:
		iconText = "info";
		break;
	}
	map["icon"] = iconText;
	map["buttons"] = buttons.join("|");

	QString json = CuteJson::variantToJson(map);
	server_->dynamicCall("messageBox(const QString&)", json);
}

void YaOnline::doMessageBoxClosed(const QString& id, int button)
{
	emit messageBoxClosed(QCA::Base64().decodeString(id), button);
}

void YaOnline::rosterHandle(int winId)
{
	server_->dynamicCall("rosterHandle(int)", winId);
}

void YaOnline::vcardChanged(const Jid& jid)
{
	if (onlineAccount() && onlineAccount()->jid().compare(jid, false)) {
		const XMPP::VCard* vcard = VCardFactory::instance()->vcard(jid);
		if (vcard) {
			server_->dynamicCall("selfGenderChanged(const QString&)", Ya::VisualUtil::contactGenderString(vcard->gender()));
		}
	}
}
