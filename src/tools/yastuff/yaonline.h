/*
 * yaonline.h - communication with running instance of Online
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

#ifndef YAONLINE_H
#define YAONLINE_H

#include <QObject>
#include <QVariant>
#include <QMessageBox>
#include <QPointer>

class PsiAccount;
class PsiCon;
class PsiEvent;
class YaPsiServer;
class YcuApiWrapper;
class PsiContact;
class YaAddContactHelper;
class QTimer;
class YaOnlineMainWin;

#include "xmpp_message.h"
#include "xmpp_status.h"

using namespace XMPP;

class YaOnline;
class YaOnlineHelper
{
public:
	// it's better to use PsiCon::yaOnline() to get this, though
	static YaOnline* instance();
	friend class PsiCon;
	friend class PsiMain;
	friend class YaRemoveConfirmationMessageBoxManager;
};

class YaOnline : public QObject
{
	Q_OBJECT
public:
	YaOnline(YaPsiServer* parent);
	~YaOnline();

	void notify(int id, PsiEvent* event, int soundType);
	void closeNotify(int id, PsiEvent* event);
	void setSelfInfo(PsiContact* selfContact);

	void openUrl(const QString& url, bool isYandex);
	void openMail(const QString& email);
	void clearedMessageHistory();
	void rosterHandle(int winId);

	void updateActiveProfile();
	QString activeProfile() const;
	void setController(PsiCon* controller);
	void setMainWin(YaOnlineMainWin* mainWin);

	void doHideSidebar();

	static bool onlineIsRunning();
	XMPP::Status::Type lastStatus() const;
	bool onlineShouldBeVisible() const;
	bool chatIsMain() const;
	QRect onlineSidebarGeometry() const;

	void doOnlineConnected();
	void jabberNotify(const QString& type, const QString& param1 = QString(), const QString& param2 = QString());
	void optionChanged(const QString& name, const QString& value);
	void setVisible();
	void showOnline(const QRect& rosterRect, bool animate, const QVariantMap& map);
	void hideOnline();
	void activateOnline();
	void deactivateOnline();
	void onlineMoved();
	void setMainToOnline();
	void login();

	void addContactClicked(QRect addButtonRect, QRect windowRect);
	void addContact(int rosterX, int y, int rosterW, const QStringList& groups);
	void foundContact(PsiAccount* account, const XMPP::Jid& jid, bool inRoster, bool authorized, bool selfContact);
	void searchComplete();

	bool useSsl() const;
	QString sslPemPath() const;
	bool onlinePasswordVerified() const;

	void messageBox(const QString& parent, const QString& id, const QString& caption, const QString& text, const QStringList& buttons, QMessageBox::Icon icon);

signals:
	void hideRoster();
	void showRoster();
	void forceStatus(XMPP::Status::Type statusType);
	void forceManualStatus(XMPP::Status::Type statusType);
	void clearMoods();
	void showYapsiPreferences();
	void doGetChatPreferences();
	QString doGetChatAccounts();
	void doStopAccountUpdates();
	void doApplyPreferences(const QString& xml);
	void doApplyImmediatePreferences(const QString& xml);
	void doOnlineHiding();
	void doOnlineVisible();
	void doOnlineCreated(int onlineWinId);
	void doActivateRoster();
	void doOnlineDeactivated();
	void showRelativeToOnline(const QRect& onlineRect);
	void scrollToContact(PsiContact* contact);
	void messageBoxClosed(const QString& id, int button);

public slots:
	void doQuit();
	void showSidebar();
	void showPreferences();
	void setPreferences(const QString& xml);
	void setAccounts(const QString& xml);
	void setImmediatePreferences(const QString& xml);
	void setDND(bool isDND);
	void setStatus(XMPP::Status::Type statusType);
	void setCurrentlyVisibleStatus(XMPP::Status::Type statusType);
	void doShowIgnoredToasters();
	void incomingStanza(const QString& xml);
	void showSuccessfullyConnectedToaster(PsiAccount* account);
	void showConnectionErrorToaster(PsiAccount* account, const QString& error);

private slots:
	void doStartActions();
	void notifyAllUnshownEvents();
	void onlineObjectChanged();
	void accountCountChanged();
	void lastMailNotify(const XMPP::Message&);
	void disconnectRereadSettingsAndReconnect(bool reconnect);
	void doSetOfflineMode();
	void doSetDND(bool isDND);
	void doSetStatus(const QString& status);
	void doSetStatus(XMPP::Status::Type statusType);
	void doSoundsChanged();
	void doPingServer();
	void doToasterClicked(const QString& id, bool activate);
	void doToasterScreenLocked(const QString& id);
	void doToasterIgnored(const QString& id);
	void doToasterSkipped(const QString& id);
	void doToasterDone(const QString& id);
	void doScreenUnlocked();
	void doPlaySound(const QString& type);
	void doAuthAccept(const QString& id);
	void doAuthDecline(const QString& id);
	void doOpenHistory(const QString& id);
	void doOpenProfile(const QString& id);
	void doBlockContact(const QString& id);
	void doSetShowMoodChangesEnabled(const QString& id, bool enabled);
	void doShowTooltip(const QString& id);
	void doShowXmlConsole(const QString& accountId);
	void doScrollToContact(const QString& jid);
	void doRequestAuth(const QString& jid);
	void doOpenChat(const QString& jid);

	void ipcMessage(const QString& message);
	void checkForAliveOnlineProcess();

	void startEventQueueTimer();
	void eventQueueChanged();
	void onlineAccountUpdated();

	void doJInit();
	void doJConnect(bool useSsl, const QString& pemPath);
	void doJDisconnect();
	void doJSendRaw(const QString& xml);
	void doJSetPresence(const QString& type);
	void doJSetCanConnectConnections(bool canConnectConnections);

	void doShowRoster(bool visible);
	void doChangeMain(const QString& mainApp, const QRect& onlineRect);
	void onlineOptionChanged(const QString& name, const QString& value);
	void psiOptionChanged(const QString& option);
	void showOfflineChanged(bool showOffline);

	void doFindContact(const QStringList& jids, const QString& type);
	void doAddContact(const QString& jid, const QString& group);
	void doCancelSearch();

	void doMessageBoxClosed(const QString& id, int button);
	void vcardChanged(const Jid&);

private:
	static YaOnline* instance_;
	YaPsiServer* server_;
	PsiCon* controller_;
	YcuApiWrapper* ycuApi_;
	YaAddContactHelper* addContactHelper_;
	XMPP::Status::Type lastStatus_;
	QString activeProfile_;
	bool onlineAccountDnd_;
	bool onlineAccountConnected_;
	bool canConnectConnections_;
	bool doStartActions_;
	QTimer* queueChangedTimer_;
	QDateTime queueChangedTimerStartTime_;
	QTimer* checkForAliveOnlineProcessTimer_;
	QPointer<YaOnlineMainWin> mainWin_;

	bool useSsl_;
	QString sslPemPath_;
	bool onlinePasswordVerified_;

	PsiAccount* onlineAccount() const;
	void rereadSettings();
	void updateOnlineAccount();
	void updateProxySettings();
	void updateMiscSettings();
	void showToaster(const QString& type, PsiAccount* account, const XMPP::Jid& jid, const QString& message, const QDateTime& timeStamp, const QString& callbackId, int soundType);
	bool doToasterIgnored(PsiAccount* account, const XMPP::Jid& jid);
	void showConnectionToaster(PsiAccount* account, const QString& error, bool showingError);

	friend class YaOnlineHelper;
};

#endif
