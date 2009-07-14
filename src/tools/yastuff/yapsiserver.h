/*
 * yapsiserver.h - COM server
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

#ifndef YAPSISERVER_H
#define YAPSISERVER_H

#include <QObject>
#include <QPointer>
#include <QAxObject>

class PsiMain;
class YaOnline;
struct IDispatch;
class QTimer;

class YaPsiServer : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("ClassID",     "{9701704f-7b96-49a7-9e7c-179c1b0bcd72}")
	Q_CLASSINFO("InterfaceID", "{b02d7f7c-d475-4f19-a1d4-e4df4e276186}")
	Q_CLASSINFO("EventsID",    "{6396e824-d832-4b90-9ff7-d7d2e6d645a6}")

public:
	YaPsiServer(QObject* parent = 0);
	~YaPsiServer();

	void doQuit();
	void dynamicCall(const char* function, const QVariant& var1 = QVariant(), const QVariant& var2 = QVariant(), const QVariant& var3 = QVariant(), const QVariant& var4 = QVariant(), const QVariant& var5 = QVariant(), const QVariant& var6 = QVariant(), const QVariant& var7 = QVariant(), const QVariant& var8 = QVariant());

	// static YaPsiServer* instance();

signals:
	void onlineObjectChanged();
	void doStartActions();
	void doShowRoster(bool visible);
	void disconnectRereadSettingsAndReconnect(bool reconnect);
	void doSetOfflineMode();
	void doSetDND(bool isDND);
	void doSetStatus(const QString& status);
	void doSoundsChanged();
	// void doPingServer();
	void doToasterClicked(const QString& id, bool activate);
	void doToasterScreenLocked(const QString& id);
	void doToasterIgnored(const QString& id);
	void doToasterSkipped(const QString& id);
	void doToasterDone(const QString& id);
	void doShowIgnoredToasters();
	void doScreenUnlocked();
	// void doPlaySound(const QString& type);
	void clearMoods();
	void doAuthAccept(const QString& id);
	void doAuthDecline(const QString& id);
	// void doOnlineConnected();
	void showPreferences();
	void doGetChatPreferences();
	void doStopAccountUpdates();
	void doApplyPreferences(const QString& xml);
	void doApplyImmediatePreferences(const QString& xml);
	void doOpenHistory(const QString& id);
	void doOpenProfile(const QString& id);
	void doBlockContact(const QString& id);
	void doSetShowMoodChangesEnabled(const QString& id, bool enabled);
	void doShowTooltip(const QString& id);
	void doShowXmlConsole(const QString& accountId);
	void doScrollToContact(const QString& jid);
	void doRequestAuth(const QString& jid);
	void doOpenChat(const QString& jid);

	void doJInit();
	void doJConnect(bool useSsl, const QString& pemPath);
	void doJDisconnect();
	void doJSendRaw(const QString& xml);
	void doJSetPresence(const QString& type);

	void doOnlineHiding();
	void doOnlineVisible();
	void doOnlineCreated(int onlineWinId);
	void doChangeMain(const QString& mainApp, const QRect& onlineRect);
	void doActivateRoster();
	void doOnlineDeactivated();
	void doOptionChanged(const QString& name, const QString& value);

	void doFindContact(const QStringList& jids, const QString& type);
	void doAddContact(const QString& jid, const QString& group);
	void doCancelSearch();

	void doMessageBoxClosed(const QString& id, int button);

// Note: the following slots are intended for use by the COM/OLE clients **only**
public slots:
	void start(bool offlineMode);
	void shutdown();
	void startActions();
	void showRoster();
	void hideRoster();
	void hideAllWindows();
	QString getBuildNumber();
	QString getUninstallPath();
	void proxyChanged();
	void userChanged();
	void setDND(bool isDND);
	void setStatus(const QString& status);
	void soundsChanged();
	// void onlineConnected();
	// void onlineDisconnected();
	// void playSound(const QString& type);

	void showText(const QString& text);
	void setOnlineObject(IDispatch* obj);
	void toasterClicked(const QString& text, bool activate);
	void toasterScreenLocked(const QString& id);
	void toasterIgnored(const QString& id);
	void toasterSkipped(const QString& id);
	void onToasterDone(const QString& id);
	void showIgnoredToasters();
	void screenUnlocked();
	void showProperties();
	void getChatPreferences();
	void stopAccountUpdates();
	void applyPreferences(const QString& xml);
	void applyImmediatePreferences(const QString& xml);
	void requestingShowTooltip(const QString& id);
	void openHistory(const QString& id);
	void openProfile(const QString& id);
	void blockContact(const QString& id);
	void setShowMoodChangesEnabled(const QString& id, bool enabled);
	void showXmlConsole(const QString& accountId);
	void scrollToContact(const QString& jid);
	void requestAuth(const QString& jid);
	void openChat(const QString& jid);

	void authAccept(const QString& id);
	void authDecline(const QString& id);

	void jabberAction(const QString& type, const QString& param1, const QString& param2);

	// docking support
	void onlineHiding();
	void onlineVisible();
	void onlineCreated(int onlineWinId);
	void changeMain(const QString& mainApp, int onlineX, int onlineY, int onlineW, int onlineH);
	void activateRoster();
	void onlineDeactivated();
	void optionChanged(const QString& name, const QString& value);

	void findContact(const QString& jidList, const QString& type);
	void addContact(const QString& jid, const QString& group);
	void cancelSearch();

	void messageBoxClosed(const QString& id, int button);

private slots:
	void dynamicCallTimerTimeout();
	void applicationAboutToQuit();

private:
	QPointer<PsiMain> main_;
	QPointer<YaOnline> yaOnline_;
	QPointer<QAxObject> onlineObject_;

	static YaPsiServer* instance_;

	struct DynamicCall {
		QString function;
		QVariantList vars;
	};
	QList<DynamicCall> dynamicCalls_;
	QTimer* dynamicCallTimer_;
	bool quitting_;

	void deinitProfile();
	void initProfile(bool reconnect);
	void setOfflineMode();
};

#endif
