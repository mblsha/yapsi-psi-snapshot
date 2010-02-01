/*
 * yamainwin.h - roster window
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

#ifndef YAMAINWIN_H
#define YAMAINWIN_H

#include <QMap>
#include <QList>
#include <QString>
#include <QStringList>

#include "yaonlinemainwin.h"

#include "advwidget.h"
#include "ui_yamainwindow.h"

class QMenuBar;
class QAction;
class QPixmap;
class QPoint;
class QMenu;

class PsiCon;
class PsiToolBar;
class PsiAccount;
class IconAction;
class PsiIcon;
class ContactListModel;
class YaTrayIcon;
class StatusMenu;
class YaSettingsButtonExtraButton;
class YaInformer;
class YaPreferences;
class YaDayUse;
class YaContactListModel;
class YaDebugConsole;

using namespace XMPP;

class YaMainWin : public YaOnlineMainWin
{
	Q_OBJECT
private:
	YaMainWin(const YaMainWin&);
	YaMainWin& operator=(const YaMainWin&);

public:
	YaMainWin(bool onTop, bool asTool, PsiCon *, const char *name=0);
	~YaMainWin();

	YaContactListModel* contactListModel() const;

	XMPP::Status::Type lastLoggedInStatusType() const;
	XMPP::Status::Type statusType() const;
	QString statusMessage() const;

	void setWindowOpts(bool onTop, bool asTool);
	void setUseDock(bool);

signals:
	void statusChanged(int);
	void changeProfile();
	void blankMessage();
	void closeProgram();
	void doOptions();
	void doToolbars();
	void doGroupChat();
	void doFileTransDlg();
	void accountInfo();
	void recvNextEvent();

public slots:
	void filterContacts();

	void showNoFocus();

	// reimplemented
	void decorateButton(int);

	void updateReadNext(PsiIcon *nextAnim, int nextAmount);

	void optionsUpdate();
	void setTrayToolTip(const XMPP::Status &, bool usePriority = false);

	void createPreferences();
	void toggleVisible();

private slots:
	void accountCountChanged();
	void accountActivityChanged();
	void showSelfProfile();
	void clearCaches();
	void toggleGroupchat();

	// reimplemented
	virtual void statusSelected(XMPP::Status::Type);
	virtual void statusSelectedManually(XMPP::Status::Type);
	virtual void statusSelectedManuallyHelper(XMPP::Status::Type);

	void statusSelectedHelper(XMPP::Status::Type, bool isManualStatus);

	// reimplemented
	virtual void clearMoods();
	virtual void setWindowVisible(bool visible);
	virtual void togglePreferences();

	void resetLastManualStatusSafeGuard();
	void moodChanged();
	void statusSelected();
	void dndEnabledActionTriggered();
	void staysOnTopTriggered();

	void trayShow();
	void trayClicked();
	void trayDoubleClicked();

	void about();
	void quitApplication();

	void accountContactsChanged();
	// void updateFriendsFrameVisibility();

	void ipcMessage(const QString&);

	void setStatusOnline();
	void setStatusDND();
	void setStatusOffline();

	void updateSelfWidgetsVisibility();
	void dockActivated();
	void activateToShowError(YaInformer* informer);

	void toggleAccounts();
	void forcePreferencesVisible();

	void vcardChanged(const Jid& jid);

protected slots:
	// reimplemented
	virtual void optionChanged(const QString& option);
	virtual void paint(QPainter* p);

	void createMenuBar();

protected:
	QMenuBar* mainMenuBar() const;

	// reimplemented
	void closeEvent(QCloseEvent*);
	bool eventFilter(QObject*, QEvent*);
	void repaintBackground();
	bool expandWidthWhenMaximized() const;
	void contextMenuEvent(QContextMenuEvent*);

private:
	PsiAccount* yaAccount() const;
	PsiAccount* account() const;
	QList<PsiAccount*> outerAccounts() const;
	void paintOnlineLogo(QPainter* p);

	QPointer<PsiCon> psi_;
	YaDayUse* yaDayUse_;
#if 0
	ContactListModel* informersModel_;
#endif
	QPointer<YaTrayIcon> tray_;
	QAction* accountAction_;
	QAction* createAccountAction_;
	QAction* showOfflineContactsAction_;
	QAction* addGroupAction_;
	QAction* addContactAction_;
	QAction* showYapsiAction_;
	QAction* dndEnabledAction_;
	QAction* staysOnTopAction_;
	QAction* groupchatAction_;
	QAction* toggleGroupchatAction_;
	QAction* clearCachesAction_;
	QAction* optionsAction_;
	QAction* aboutAction_;
	QAction* quitAction_;
	QAction* showDebugConsoleAction_;
	QMenu* settingsMenu_;
	StatusMenu* statusMenu_;
	YaSettingsButtonExtraButton* settingsButton_;
	// QTimer* updateFriendsFrameVisibilityTimer_;
	Ui::MainWindow ui_;
	QPointer<YaPreferences> preferences_;
	QPointer<YaDebugConsole> debugConsole_;
	int topMargin_;
};

#endif
