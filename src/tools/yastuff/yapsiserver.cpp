/*
 * yapsiserver.cpp - COM server
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

#include "yapsiserver.h"

#include <QApplication>
#include <QAxFactory>
#include <QPointer>
#include <QSettings>
#include <QDir>

QAXFACTORY_BEGIN("{6801583f-e09f-49be-a2bf-ed06fa3aad76}",
                 "{30265966-7950-4438-b601-af5d9eb29f9e}")
	QAXCLASS(YaPsiServer)
QAXFACTORY_END()

#include <QMessageBox>
#include <QTimer>

void log(QString message)
{
	QMessageBox::information(0, "YaPsiServer",
	                         message);
}

#include "psicon.h"
#include "main.h"
#include "yaonline.h"
#include "yapsi_revision.h" // auto-generated file, see src.pro for details
#include "psilogger.h"
#include <private/qstylesheetstyle_p.h>
#include <private/qapplication_p.h>

YaPsiServer* YaPsiServer::instance_ = 0;

YaPsiServer::YaPsiServer(QObject* parent)
	: QObject(parent)
	, main_(0)
	, yaOnline_(0)
	, dynamicCallTimer_(0)
	, quitting_(false)
{
	LOG_TRACE;
	if (instance_) {
		LOG_TRACE;
		// log("YaPsiServer already instantiated!");
		delete instance_;
		instance_ = 0;
	}

	instance_ = this;
	LOG_TRACE;

	connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(applicationAboutToQuit()));

	dynamicCallTimer_ = new QTimer(this);
	connect(dynamicCallTimer_, SIGNAL(timeout()), SLOT(dynamicCallTimerTimeout()));
	dynamicCallTimer_->setInterval(50);
	dynamicCallTimer_->setSingleShot(false);
}

YaPsiServer::~YaPsiServer()
{
	onlineObject_ = 0;

	deinitProfile();
	delete yaOnline_;
	instance_ = 0;
}

void YaPsiServer::applicationAboutToQuit()
{
	PsiLogger::instance()->log("YaPsiServer::applicationAboutToQuit()");
	// deinitProfile();
}

void YaPsiServer::start(bool offlineMode)
{
	LOG_TRACE;
	if (main_) {
		return;
	}

	LOG_TRACE;
	if (!yaOnline_) {
		yaOnline_ = new YaOnline(this);
		emit onlineObjectChanged();
	}

	LOG_TRACE;
	initProfile(false);
	LOG_TRACE;

	// if (offlineMode) {
		setOfflineMode();
	// }
	LOG_TRACE;
}

void YaPsiServer::deinitProfile()
{
	LOG_TRACE;
	if (!main_.isNull()) {
		Q_ASSERT(yaOnline_);
		Q_ASSERT(main_->controller());
		yaOnline_->doHideSidebar();
		yaOnline_->rosterHandle(0);
		yaOnline_->setController(0);
		main_->controller()->setYaOnline(0);
		delete main_;
		main_ = 0;

		// the main problem with YaPsiServer::userChanged() always were
		// unhandled exceptions, and QStyleSheetStyle polishing was the
		// biggest troublemaker of 'em all. so now we're forcefully
		// cleaning it up so it won't access any invalid widget pointers
		// that still could be left inside its internal structures
		QStyleSheetStyle* proxy = qobject_cast<QStyleSheetStyle*>(qApp->style());
		LOG_TRACE;
		if (proxy) {
			QStyle* base = proxy->base;
			Q_ASSERT(base);
			LOG_TRACE;
			if (base) {
				LOG_TRACE;
				QApplicationPrivate::app_style = 0;
				QApplicationPrivate::styleSheet = QString();
				base->setParent(qApp);
				delete proxy;
				qApp->setStyle(base);
			}
		}
	}
	LOG_TRACE;
}

void YaPsiServer::initProfile(bool reconnect)
{
	LOG_TRACE;
	if (main_ || !yaOnline_)
		return;

	LOG_TRACE;
	main_ = new PsiMain();
	LOG_TRACE;
	qApp->processEvents();
	LOG_TRACE;
	yaOnline_->setController(main_->controller());
	LOG_TRACE;

	emit disconnectRereadSettingsAndReconnect(reconnect);
	LOG_TRACE;
}

void YaPsiServer::showText(const QString& text)
{
	log(text);
}

void YaPsiServer::toasterClicked(const QString& id, bool activate)
{
	emit doToasterClicked(id, activate);
}

void YaPsiServer::toasterScreenLocked(const QString& id)
{
	emit doToasterScreenLocked(id);
}

void YaPsiServer::toasterIgnored(const QString& id)
{
	emit doToasterIgnored(id);
}

void YaPsiServer::toasterSkipped(const QString& id)
{
	emit doToasterSkipped(id);
}

void YaPsiServer::onToasterDone(const QString& id)
{
	emit doToasterDone(id);
}

void YaPsiServer::showIgnoredToasters()
{
	emit doShowIgnoredToasters();
}

void YaPsiServer::screenUnlocked()
{
	emit doScreenUnlocked();
}

void YaPsiServer::showProperties()
{
	emit showPreferences();
}

void YaPsiServer::getChatPreferences()
{
	emit doGetChatPreferences();
}

QString YaPsiServer::getChatAccounts()
{
	QString result = doGetChatAccounts();
	return result;
}

void YaPsiServer::applyPreferences(const QString& xml)
{
	emit doApplyPreferences(xml);
}

void YaPsiServer::applyImmediatePreferences(const QString& xml)
{
	emit doApplyImmediatePreferences(xml);
}

void YaPsiServer::requestingShowTooltip(const QString& id)
{
	emit doShowTooltip(id);
}

void YaPsiServer::setOnlineObject(IDispatch* obj)
{
	if (!onlineObject_.isNull()) {
		onlineObject_->deleteLater();
		onlineObject_ = 0;
	}

	if (obj) {
		onlineObject_ = new QAxObject(reinterpret_cast<IUnknown*>(obj), 0);
	}

	emit onlineObjectChanged();
}

void YaPsiServer::doQuit()
{
	Q_ASSERT(!onlineObject_.isNull());
	Q_ASSERT(!InSendMessage());

	if (!InSendMessage())
		onlineObject_->dynamicCall("chatExit(const QString&)", QString());
	else
		dynamicCall("chatExit(const QString&)", QString());
}

void YaPsiServer::dynamicCall(const char* function, const QVariant& var1, const QVariant& var2, const QVariant& var3, const QVariant& var4, const QVariant& var5, const QVariant& var6, const QVariant& var7, const QVariant& var8)
{
	DynamicCall call;
	call.function = QString::fromUtf8(function);
	call.vars << var1;
	call.vars << var2;
	call.vars << var3;
	call.vars << var4;
	call.vars << var5;
	call.vars << var6;
	call.vars << var7;
	call.vars << var8;
	Q_ASSERT(call.vars.count() == 8);
	dynamicCalls_.append(call);

	dynamicCallTimer_->start();
}

void YaPsiServer::dynamicCallTimerTimeout()
{
	if (quitting_ || !instance_)
		return;

	// prevent 'A synchronous OLE call made by the recipient of an
	// inter-process/inter-thread SendMessage fails with 
	// RPC_E_CANTCALLOUT_ININPUTSYNCCALL(0x8001010D).' error
	// http://support.microsoft.com/default.aspx?scid=kb;EN-US;Q131056
	if (!InSendMessage() && !onlineObject_.isNull()) {
		while (!dynamicCalls_.isEmpty()) {
			DynamicCall call = dynamicCalls_.takeFirst();
			Q_ASSERT(call.vars.count() == 8);
			onlineObject_->dynamicCall(call.function.toUtf8().data(),
			                           call.vars[0], call.vars[1], call.vars[2],
			                           call.vars[3], call.vars[4], call.vars[5],
			                           call.vars[6], call.vars[7]);
		}
	}

	if (dynamicCalls_.isEmpty())
		dynamicCallTimer_->stop();
	else
		dynamicCallTimer_->start();
}

// YaPsiServer* YaPsiServer::instance()
// {
// 	return instance_;
// }

void YaPsiServer::shutdown()
{
	quitting_ = true;

	PsiLogger::instance()->log("YaPsiServer::shutdown()");
	delete onlineObject_;
	QPointer<QApplication> app(qApp);
	deinitProfile();

	deleteLater();
	if (!app.isNull()) {
		qApp->quit();
	}
}

void YaPsiServer::startActions()
{
	emit doStartActions();
}

void YaPsiServer::showRoster()
{
	emit doShowRoster(true);
}

void YaPsiServer::hideRoster()
{
	emit doShowRoster(false);
}

void YaPsiServer::hideAllWindows()
{
	foreach(QWidget* w, qApp->topLevelWidgets()) {
		w->hide();
	}
}

QString YaPsiServer::getBuildNumber()
{
	return QString::number(YAPSI_REVISION);
}

QString YaPsiServer::getUninstallPath()
{
	QSettings sUser(QSettings::UserScope, "Yandex", "Ya.Chat");
	QDir programDir(sUser.value("ProgramDir").toString());
	QStringList filters;
	filters << "unins*.exe";
	QStringList unins = programDir.entryList(filters, QDir::NoFilter, QDir::Time);
	if (unins.isEmpty())
		return QString();
	return programDir.absoluteFilePath(unins.first());
}

void YaPsiServer::setOfflineMode()
{
	emit doSetOfflineMode();
}

void YaPsiServer::proxyChanged()
{
	emit disconnectRereadSettingsAndReconnect(true);
}

void YaPsiServer::userChanged()
{
	LOG_TRACE;
	bool doShowRoster = main_ && main_->controller() && main_->controller()->mainWinVisible();

	deinitProfile();
	if (yaOnline_)
		yaOnline_->updateActiveProfile();
	initProfile(true);

	if (doShowRoster)
		showRoster();
}

void YaPsiServer::setDND(bool isDND)
{
	emit doSetDND(isDND);
}

void YaPsiServer::setStatus(const QString& status)
{
	emit doSetStatus(status);
}

void YaPsiServer::soundsChanged()
{
	emit doSoundsChanged();
}

#if 0
void YaPsiServer::onlineConnected()
{
	if (!yaOnline_)
		return;

	// we need to ensure that chat is / should be connected to server now
	emit doOnlineConnected();

	emit doPingServer();
}

void YaPsiServer::onlineDisconnected()
{
	emit doPingServer();
}
#endif

void YaPsiServer::playSound(const QString& type)
{
	emit doPlaySound(type);
}

void YaPsiServer::authAccept(const QString& id)
{
	emit doAuthAccept(id);
}

void YaPsiServer::authDecline(const QString& id)
{
	emit doAuthDecline(id);
}

void YaPsiServer::openHistory(const QString& id)
{
	emit doOpenHistory(id);
}

void YaPsiServer::openProfile(const QString& id)
{
	emit doOpenProfile(id);
}

void YaPsiServer::blockContact(const QString& id)
{
	emit doBlockContact(id);
}

void YaPsiServer::setShowMoodChangesEnabled(const QString& id, bool enabled)
{
	emit doSetShowMoodChangesEnabled(id, enabled);
}

void YaPsiServer::jabberAction(const QString& type, const QString& param1, const QString& param2)
{
	if (type == "init")
		emit doJInit();
	else if (type == "connect")
		emit doJConnect(param1 == "true", param2);
	else if (type == "disconnect")
		emit doJDisconnect();
	else if (type == "send_raw")
		emit doJSendRaw(param1);
	else if (type == "set_presence")
		emit doJSetPresence(param1);
	else if (type == "set_can_connect_connections")
		emit doJSetCanConnectConnections(param1 == "true");
	else
		Q_ASSERT(false);
}

void YaPsiServer::showXmlConsole(const QString& accountId)
{
	emit doShowXmlConsole(accountId);
}

void YaPsiServer::onlineHiding()
{
	emit doOnlineHiding();
}

void YaPsiServer::onlineVisible()
{
	emit doOnlineVisible();
}

void YaPsiServer::onlineCreated(int onlineWinId)
{
	emit doOnlineCreated(onlineWinId);
}

void YaPsiServer::changeMain(const QString& mainApp, int onlineX, int onlineY, int onlineW, int onlineH)
{
	QRect rect(onlineX, onlineY, onlineW, onlineH);
	emit doChangeMain(mainApp, rect);
}

void YaPsiServer::activateRoster()
{
	emit doActivateRoster();
}

void YaPsiServer::onlineDeactivated()
{
	emit doOnlineDeactivated();
}

void YaPsiServer::optionChanged(const QString& name, const QString& value)
{
	emit doOptionChanged(name, value);
}

void YaPsiServer::findContact(const QString& jidList, const QString& type)
{
	QStringList tmp = jidList.split("@@");
	emit doFindContact(tmp, type);
}

void YaPsiServer::addContact(const QString& jid, const QString& group)
{
	emit doAddContact(jid, group);
}

void YaPsiServer::scrollToContact(const QString& jid)
{
	emit doScrollToContact(jid);
}

void YaPsiServer::requestAuth(const QString& jid)
{
	emit doRequestAuth(jid);
}

void YaPsiServer::openChat(const QString& jid)
{
	emit doOpenChat(jid);
}

void YaPsiServer::cancelSearch()
{
	emit doCancelSearch();
}

void YaPsiServer::messageBoxClosed(const QString& id, int button)
{
	emit doMessageBoxClosed(id, button);
}

void YaPsiServer::stopAccountUpdates()
{
	emit doStopAccountUpdates();
}
