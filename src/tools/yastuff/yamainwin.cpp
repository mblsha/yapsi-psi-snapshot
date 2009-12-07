/*
 * yamainwin.cpp - roster window
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

#include "yamainwin.h"

#include <QDesktopServices>
#include <qicon.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qobject.h>
#include <QPainter>
#include <qsignalmapper.h>
#include <qmenubar.h>
#include <QPixmap>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QMenu>
#include <QMenuItem>
#include <QMessageBox>
#include <Q3PtrList>

#include "im.h"
#include "common.h"
#include "showtextdlg.h"
#include "psicon.h"
#include "psiiconset.h"
#include "applicationinfo.h"
#include "psiaccount.h"
#include "psitrayicon.h"
#include "psitoolbar.h"
#include "aboutdlg.h"
#include "psitoolbar.h"
#include "psipopup.h"
#include "psioptions.h"
#include "tipdlg.h"
#include "psicontactlist.h"
#if 0
#include "yainformersmodel.h"
#endif
#include "yaselflabel.h"
#include "yatrayicon.h"
#include "urlobject.h"
#include "yacommon.h"
#include "tabdlg.h"
#include "statusmenu.h"
#include "userlist.h"
#include "yaabout.h"
#include "yalastmailinformer.h"
#include "yaeventnotifier.h"
#include "shortcutmanager.h"
#include "yasettingsbutton.h"
#include "yavisualutil.h"
#ifdef GROUPCHAT
#include "groupchatdlg.h"
#endif
#include "yaipc.h"
#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif
#include "yapreferences.h"
#include "yaboldmenu.h"
#include "psitooltip.h"
#include "yadayuse.h"
#include "yachattooltip.h"
#include "vcardfactory.h"
#include "psicontact.h"
#include "avatars.h"
#include "chatdlg.h"
#include "yadebugconsole.h"

#include "mainwin_p.h"

static const QString lastLoggedInStatusTypeOptionPath = "options.ya.last-logged-in-status-type";
// static const QString tinyContactsOptionPath = "options.ya.main-window.contact-list.tiny-contacts";
static const QString alwaysOnTopOptionPath = "options.ya.main-window.always-on-top";
static const QString showDefaultMoodOptionPath = "options.ya.moods.show-default-mood";
static const QString DEFAULT_MOOD_TEXT = QString::fromUtf8("Работает Я.Онлайн: online.yandex.ru");

const static int MAX_NOTIFIER_WIDTH = 5;

using namespace XMPP;

class YaLogo : public QWidget
{
	Q_OBJECT
public:
	YaLogo(QWidget* parent)
		: QWidget(parent)
	{
		logo_ = QPixmap(":iconsets/system/default/logo_128.png");
		setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		size_ = QSize(50, 50);
	}

	// reimplemented
	QSize sizeHint() const
	{
		return QSize(size_.height(), size_.height());
	}

protected:
	// reimplemented
	void paintEvent(QPaintEvent*)
	{
		// QPainter p(this);
		// p.setRenderHint(QPainter::Antialiasing, true);
		// p.setRenderHint(QPainter::TextAntialiasing, true);
		// p.setRenderHint(QPainter::SmoothPixmapTransform, true);
		// QRect pixmapRect(0, 0, height(), height());
		// p.drawPixmap(pixmapRect, logo_);
	}

private:
	QPixmap logo_;
	QSize size_;
};

YaMainWin::YaMainWin(bool _onTop, bool _asTool, PsiCon* psi, const char* name)
	: YaOnlineMainWin(psi, 0, (_onTop ? Qt::WStyle_StaysOnTop : Qt::Widget) | (_asTool ? Qt::WStyle_Tool : Qt::Widget))
	, psi_(psi)
	, yaDayUse_(0)
#if 0
	, informersModel_(0)
#endif
	, tray_(0)
	, accountAction_(0)
{
	YaIPC::connect(this, SLOT(ipcMessage(const QString&)));

	yaDayUse_ = new YaDayUse(this);

	// Continue loading
	Q_UNUSED(name);
	// setObjectName(name);

	// Disabled click-through would be great, but it looks freaky at the moment
	// setAttribute(Qt::WA_MacNoClickThrough, true);

	// we want fancy roster tooltips to be visible even if mainwin is not focused
	setAttribute(Qt::WA_AlwaysShowToolTips, true);

	topMargin_ = 20;

	connect(VCardFactory::instance(), SIGNAL(vcardChanged(const Jid&)), SLOT(vcardChanged(const Jid&)));

	ui_.setupUi(this);
	updateContentsMargins(0, !theme().customFrame() ? -topMargin_ : 0, 0, 0);
	setMinimumWidth(200);
#ifdef YAPSI_ACTIVEX_SERVER
	setMinimumHeight(360);
#else
	setMinimumHeight(175);
#endif

#if 0
	ui_.topStack->init();
	ui_.topStack->setAnimationStyle(AnimatedStackedWidget::Animation_Push_Horizontal);
	ui_.topStack->setWidgetPriority(ui_.logoPage, 0);
	ui_.topStack->setWidgetPriority(ui_.selfWidgetsPage, 1);
#endif

	ui_.logoPage->installEventFilter(this);
	ui_.selfWidgetsPage->installEventFilter(this);

	YaLogo* yaLogo = new YaLogo(ui_.logoPage);
	replaceWidget(ui_.logoFrame, yaLogo);
	ui_.logoFrame = yaLogo;

	ui_.applicationName->setText(QString());

	// YaSelfMood must always be created the last, otherwise interaction with its
	// expanding menus will be severely limited on non-osx platforms
	YaSelfMood *selfMoodOld = ui_.selfMood;
	ui_.selfMood = new YaSelfMood(selfMoodOld->parentWidget());
	replaceWidget(selfMoodOld, ui_.selfMood);
	ui_.selfMood->raiseExtraInWidgetStack();

	YaChatContactInfo *selfInfoOld = ui_.selfInfo;
	ui_.selfInfo = new YaChatContactInfo(ui_.selfInfo->parentWidget());
	ui_.selfInfo->setMaximumSize(1, 1);
	replaceWidget(selfInfoOld, ui_.selfInfo);
	ui_.selfInfo->raiseExtraInWidgetStack();
	connect(ui_.selfInfo, SIGNAL(clicked()), SLOT(showSelfProfile()));

	ShortcutManager::connect("appwide.status-online",  this, SLOT(setStatusOnline()));
	ShortcutManager::connect("appwide.status-dnd",     this, SLOT(setStatusDND()));
	ShortcutManager::connect("appwide.status-offline", this, SLOT(setStatusOffline()));

	ShortcutManager::connect("appwide.filter-contacts", this, SLOT(filterContacts()));

	// menuBar()->hide();
	// setStatusBar(0);

	setWindowTitle(tr("Ya.Online"));
#ifndef Q_WS_MAC
	qApp->setWindowIcon(Ya::VisualUtil::defaultWindowIcon());
// #ifdef Q_WS_X11
	setWindowIcon(Ya::VisualUtil::defaultWindowIcon());
// #endif
#endif

	showOfflineContactsAction_ = new QAction(tr("Show Offline Contacts"), this);
	showOfflineContactsAction_->setCheckable(true);
	connect(showOfflineContactsAction_, SIGNAL(toggled(bool)), psi_->contactList(), SLOT(setShowOffline(bool)));
	connect(psi_->contactList(), SIGNAL(showOfflineChanged(bool)), showOfflineContactsAction_, SLOT(setChecked(bool)));

	addGroupAction_ = new QAction(tr("Add Group..."), this);
	addGroupAction_->setEnabled(ui_.roster->haveAvailableAccounts());
	connect(ui_.roster, SIGNAL(availableAccountsChanged(bool)), addGroupAction_, SLOT(setEnabled(bool)));
	connect(addGroupAction_, SIGNAL(triggered()), ui_.roster, SLOT(addGroup()), Qt::QueuedConnection);
	connect(ui_.roster, SIGNAL(enableAddGroupAction(bool)), addGroupAction_, SLOT(setVisible(bool)));

	addContactAction_ = new QAction(tr("Add Contact..."), this);
	addContactAction_->setEnabled(ui_.roster->haveAvailableAccounts());
	connect(ui_.roster, SIGNAL(availableAccountsChanged(bool)), addContactAction_, SLOT(setEnabled(bool)));
	connect(addContactAction_, SIGNAL(triggered()), ui_.roster, SLOT(addContact()), Qt::QueuedConnection);
	addContactAction_->setShortcuts(ShortcutManager::instance()->shortcuts("appwide.add-contact"));
	addContactAction_->setShortcutContext(Qt::ApplicationShortcut);
	//ShortcutManager::connect("appwide.add-contact", addContactAction_, SLOT(trigger())); // doesn't work on windows

	groupchatAction_ = new QAction(tr("Join Groupchat..."), this);
	YaBoldMenu::ensureActionBoldText(groupchatAction_);
	connect(groupchatAction_, SIGNAL(triggered()), SIGNAL(doGroupChat()));

	clearCachesAction_ = new QAction(tr("Clear Caches"), this);
	clearCachesAction_->setVisible(false);
	YaBoldMenu::ensureActionBoldText(clearCachesAction_);
	connect(clearCachesAction_, SIGNAL(triggered()), SLOT(clearCaches()));

	optionsAction_ = new QAction(tr("Preferences..."), this);
	optionsAction_->setMenuRole(QAction::PreferencesRole);
	connect(optionsAction_, SIGNAL(triggered()), SLOT(togglePreferences()));

#ifdef YAPSI_ACTIVEX_SERVER
	QAction* forceOptionsAction = new QAction(tr("Force Preferences..."), this);
	connect(forceOptionsAction, SIGNAL(triggered()), SLOT(forcePreferencesVisible()));

	QAction* setMainToOnlineAction = new QAction(tr("Hide Contact List"), this);
	connect(setMainToOnlineAction, SIGNAL(triggered()), SLOT(setMainToOnline()));
#endif

	aboutAction_ = new QAction(tr("About Ya.Online..."), this);
	aboutAction_->setMenuRole(QAction::AboutRole);
	connect(aboutAction_, SIGNAL(triggered()), SLOT(about()));
	ShortcutManager::connect("appwide.show-about-dialog", this, SLOT(about()));

	quitAction_ = new QAction(tr("Quit"), this);
	quitAction_->setMenuRole(QAction::QuitRole);
	connect(quitAction_, SIGNAL(triggered()), SLOT(quitApplication()));

	showYapsiAction_ = new QAction(tr("Show Ya.Online"), this);
	YaBoldMenu::ensureActionBoldText(showYapsiAction_);
	connect(showYapsiAction_, SIGNAL(triggered()), SLOT(trayClicked()));

	dndEnabledAction_ = new QAction(tr("Do not disturb"), this);
	dndEnabledAction_->setCheckable(true);
	connect(dndEnabledAction_, SIGNAL(triggered()), SLOT(dndEnabledActionTriggered()));

	staysOnTopAction_ = new QAction(tr("Window stays on top"), this);
	staysOnTopAction_->setCheckable(true);
	connect(staysOnTopAction_, SIGNAL(triggered()), SLOT(staysOnTopTriggered()));

	settingsMenu_ = new YaBoldMenu(this);

	statusMenu_ = new StatusMenu(settingsMenu_);
	statusMenu_->setTitle(tr("Status"));
	connect(statusMenu_, SIGNAL(statusChanged(XMPP::Status::Type)), ui_.selfMood, SLOT(setStatusType(XMPP::Status::Type)));
	connect(statusMenu_, SIGNAL(statusChanged(XMPP::Status::Type)), SLOT(statusSelectedManually(XMPP::Status::Type)));

	settingsMenu_->addAction(clearCachesAction_);
#ifdef GROUPCHAT
	if (GCMainDlg::mucEnabled()) {
		settingsMenu_->addAction(groupchatAction_);
		settingsMenu_->addSeparator();
	}
#endif
	settingsMenu_->addAction(showOfflineContactsAction_);
	settingsMenu_->addAction(staysOnTopAction_);
#ifdef YAPSI_ACTIVEX_SERVER
	settingsMenu_->addAction(setMainToOnlineAction);
#endif
	settingsMenu_->addSeparator();
	settingsMenu_->addAction(addContactAction_);
	settingsMenu_->addAction(addGroupAction_);
	// settingsMenu_->addSeparator();
	// settingsMenu_->addMenu(statusMenu_);
#ifndef Q_WS_MAC
	// settingsMenu_->addSeparator();
	settingsMenu_->addAction(optionsAction_);
#ifdef YAPSI_ACTIVEX_SERVER
	// settingsMenu_->addAction(forceOptionsAction);
#endif
	settingsMenu_->addSeparator();
	// settingsMenu_->addAction(aboutAction_);
	settingsMenu_->addAction(quitAction_);
#endif

#ifndef Q_WS_MAC
	settingsButton_ = new YaSettingsButtonExtraButton(extra());
	settingsButton_->setMenu(settingsMenu_);
#endif

#ifndef YAPSI_ACTIVEX_SERVER
	tray_ = YaTrayIcon::instance(psi_);
	tray_->updateIcon();
	QMenu* trayMenu = new YaBoldMenu(this);
	trayMenu->setSeparatorsCollapsible(true);

	trayMenu->addAction(showYapsiAction_);
	trayMenu->addAction(staysOnTopAction_);
	trayMenu->addAction(dndEnabledAction_);
	trayMenu->addSeparator();
	trayMenu->addAction(optionsAction_);
	trayMenu->addSeparator();
	trayMenu->addAction(quitAction_);

	tray_->setContextMenu(trayMenu);
#ifndef Q_WS_MAC
	connect(tray_, SIGNAL(clicked()), SLOT(trayClicked()));
	connect(tray_, SIGNAL(doubleClicked()), SLOT(trayDoubleClicked()));
#endif
#endif

	connect(qApp, SIGNAL(dockActivated()), SLOT(dockActivated()));

	connect(psi_->contactList(), SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
	connect(psi_->contactList(), SIGNAL(accountActivityChanged()), SLOT(accountActivityChanged()));
	accountCountChanged();

#if 0
	informersModel_ = new YaInformersModel(psi->contactList());
	informersModel_->invalidateLayout();
	ui_.statusBar->setModel(informersModel_);
	ui_.statusBar->setController(psi_);
#endif

	YaEventNotifierInformer* eventNotifierInformer = new YaEventNotifierInformer(this);
	eventNotifierInformer->notifier()->setController(psi_);
	ui_.statusBar->addInformer(eventNotifierInformer);

	if (!PsiOptions::instance()->getOption("options.ui.account.single").toBool()) {
		ShortcutManager::connect("appwide.activate-account-informer", this, SLOT(toggleAccounts()));
	}

	QTimer::singleShot(0, this, SLOT(createPreferences()));

	YaLastMailInformer* lastMailInformer = new YaLastMailInformer(this);
	lastMailInformer->setController(psi_);
	ui_.statusBar->addInformer(lastMailInformer);

	ui_.roster->setContactList(psi->contactList());
	ui_.roster->setContactListViewportMenu(settingsMenu_);
	connect(ui_.roster, SIGNAL(updateSelfWidgetsVisibility()), SLOT(updateSelfWidgetsVisibility()));
	updateSelfWidgetsVisibility();

	ui_.selfMood->setStatusType(XMPP::Status::Offline);
	connect(ui_.selfMood, SIGNAL(statusChanged(XMPP::Status::Type)), SLOT(statusSelected(XMPP::Status::Type)));
	connect(ui_.selfMood, SIGNAL(statusChangedManually(XMPP::Status::Type)), SLOT(statusSelectedManually(XMPP::Status::Type)));
	connect(ui_.selfMood, SIGNAL(statusChanged(const QString&)), SLOT(statusSelected()));
	connect(ui_.selfMood, SIGNAL(resetLastManualStatusSafeGuard()), SLOT(resetLastManualStatusSafeGuard()));

	// ui_.tabbedNotifier->setEventNotifier(ui_.eventNotifier);
	// ui_.tabbedNotifier->setMinimumSize(MAX_NOTIFIER_WIDTH, 0);

	// connect(ui_.rosterSplitter, SIGNAL(splitterMoved(int, int)), SLOT(splitterMoved(int, int)));

	// ui_.friendsFrame->hide();
	// updateFriendsFrameVisibilityTimer_ = new QTimer(this);
	// updateFriendsFrameVisibilityTimer_->setSingleShot(true);
	// connect(updateFriendsFrameVisibilityTimer_, SIGNAL(timeout()), SLOT(updateFriendsFrameVisibility()));

	debugConsole_ = new YaDebugConsole(psi_);
	debugConsole_->hide();

	showDebugConsoleAction_ = new QAction("Show Debug Console", this);
	connect(showDebugConsoleAction_, SIGNAL(triggered()), debugConsole_, SLOT(activate()));
	showDebugConsoleAction_->setShortcut(QKeySequence("Ctrl+Alt+Shift+D"));
	showDebugConsoleAction_->setShortcutContext(Qt::ApplicationShortcut);
	addAction(showDebugConsoleAction_);

	createMenuBar();

	setMinimizeEnabled(false);
	setMaximizeEnabled(false);

	optionChanged(alwaysOnTopOptionPath);

	setOpacityOptionPath("options.ui.contactlist.opacity");
	optionsUpdate();
}

YaMainWin::~YaMainWin()
{
	delete preferences_;
	delete debugConsole_;

	PsiPopup::deleteAll();
}

void YaMainWin::createMenuBar()
{
#ifdef Q_WS_MAC
	QMenu* mainMenu = mainMenuBar()->addMenu("Menu");
	mainMenu->addAction(optionsAction_);
	mainMenu->addAction(quitAction_);
	mainMenu->addAction(aboutAction_);

	QMenu* generalMenu = mainMenuBar()->addMenu(tr("General"));
	generalMenu->addAction(showOfflineContactsAction_);
	generalMenu->addSeparator();
	generalMenu->addAction(addContactAction_);
	generalMenu->addAction(addGroupAction_);
#ifdef GROUPCHAT
	if (GCMainDlg::mucEnabled()) {
		generalMenu->addSeparator();
		generalMenu->addAction(groupchatAction_);
	}
#endif
	generalMenu->addAction(clearCachesAction_);
#endif
}

QMenuBar* YaMainWin::mainMenuBar() const
{
#ifdef Q_WS_MAC
	return psi_->defaultMenuBar();
#else
	return 0; // menuBar();
#endif
}

XMPP::Status::Type YaMainWin::statusType() const
{
	PsiAccount* acc = account();
	if (acc)
		return acc->status().type();
	return XMPP::Status::Offline;
}

QString YaMainWin::statusMessage() const
{
	return ui_.selfMood->statusText();
}

/**
 * If there is active ya.ru account, returns it. Otherwise returns first
 * non-active ya.ru account. Otherwise returns zero.
 */
PsiAccount* YaMainWin::yaAccount() const
{
	if (psi_->contactList()->haveActiveAccounts())
		foreach(PsiAccount* account, psi_->contactList()->sortedEnabledAccounts())
			if (account->isAvailable() && account->isYaAccount())
				return account;

	foreach(PsiAccount* account, psi_->contactList()->sortedEnabledAccounts())
		if (account->isYaAccount())
			return account;

	return 0;
}

PsiAccount* YaMainWin::account() const
{
	if (psi_->contactList()->haveActiveAccounts())
		foreach(PsiAccount* account, psi_->contactList()->sortedEnabledAccounts())
			if (account->isAvailable())
				return account;

	foreach(PsiAccount* account, psi_->contactList()->sortedEnabledAccounts())
		return account;

	return 0;
}

/**
 * Returns a list of all enabled non-ya.ru accounts. Active are placed on
 * top of the list.
 */
QList<PsiAccount*> YaMainWin::outerAccounts() const
{
	QList<PsiAccount*> accounts;
	if (psi_->contactList()->haveActiveAccounts())
		foreach(PsiAccount* account, psi_->contactList()->sortedEnabledAccounts())
			if (account->isAvailable() && !account->isYaAccount())
				accounts << account;

	foreach(PsiAccount* account, psi_->contactList()->sortedEnabledAccounts())
		if (!account->isAvailable() && !account->isYaAccount())
			accounts << account;

	return accounts;
}

void YaMainWin::accountCountChanged()
{
	foreach(PsiAccount* account, psi_->contactList()->enabledAccounts()) {
#ifdef DEFAULT_XMLCONSOLE
		account->showXmlConsole();
#endif

		disconnect(account, SIGNAL(removeContact(const Jid &)), this, SLOT(accountContactsChanged()));
		connect(account,    SIGNAL(removeContact(const Jid &)), this, SLOT(accountContactsChanged()));
		disconnect(account, SIGNAL(updateContact(const Jid &)), this, SLOT(accountContactsChanged()));
		connect(account,    SIGNAL(updateContact(const Jid &)), this, SLOT(accountContactsChanged()));

		disconnect(account, SIGNAL(moodChanged()), this, SLOT(moodChanged()));
		connect(account,    SIGNAL(moodChanged()), this, SLOT(moodChanged()));
	}

	ui_.selfName->setContactList(psi_->contactList());
	ui_.selfUserpic->setContactList(psi_->contactList());
	ui_.selfName->setWindowExtra(windowExtra());

	// ui_.selfName->setVisible(!psi_->contactList()->enabledAccounts().isEmpty());
	// ui_.selfMood->setVisible(!psi_->contactList()->enabledAccounts().isEmpty());

	accountActivityChanged();
}

void YaMainWin::accountActivityChanged()
{
	if (!psi_->contactList()->accountsLoaded())
		return;

	PsiAccount* acc = this->account();
	PsiContact* contact = acc ? acc->selfContact() : 0;
	if (ui_.selfUserpic->selfContact() != contact) {
		ui_.selfUserpic->setSelfContact(contact);
		vcardChanged(contact ? contact->jid() : XMPP::Jid());
	}

	bool haveConnectingAccounts = psi_->contactList()->haveConnectingAccounts();
	ui_.selfMood->setHaveConnectingAccounts(haveConnectingAccounts);
	if (!tray_.isNull()) {
		tray_->setHaveConnectingAccounts(haveConnectingAccounts);
	}
}

void YaMainWin::trayShow()
{
	setWindowVisible(true);
}

void YaMainWin::trayClicked()
{
#ifdef Q_WS_X11
	toggleVisible();
#else
	setWindowVisible(true);
#endif
}

void YaMainWin::trayDoubleClicked()
{
	trayClicked();
}

void YaMainWin::dockActivated()
{
	setWindowVisible(true);
}

void YaMainWin::closeEvent(QCloseEvent* e)
{
#ifndef YAPSI_ACTIVEX_SERVER
	if (!tray_.isNull() && !tray_->isVisible())
		quitApplication();
#endif
	YaOnlineMainWin::closeEvent(e);
}

bool YaMainWin::eventFilter(QObject* obj, QEvent* e)
{
	static bool filtering = false;
	if (filtering)
		return false;

	if (e->type() == QEvent::Paint && (obj == ui_.logoPage || obj == ui_.selfWidgetsPage)) {
		QWidget* w = static_cast<QWidget*>(obj);
		QPainter p(w);
		Ya::VisualUtil::paintRosterBackground(w, &p);
		p.translate(-additionalLeftMargin(), -(topMargin_ + additionalTopMargin()));
		paintOnlineLogo(&p);
		return true;
	}
#if 0
	else if (e->type() == QEvent::MouseMove ||
	         e->type() == QEvent::MouseButtonRelease ||
	         e->type() == QEvent::MouseButtonPress)
	{
		// this hack is required because YaSelfMoodExtra's parent is YaMainWin, and thus
		// when it bypasses its mouse events, they don't go to the YaRoster's tool buttons
		QWidget* widget = static_cast<QWidget*>(obj);

		// we're proxying required mouse events directly
		QWidget* found = 0;

		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(e);
		foreach(QWidget* w, ui_.roster->proxyableWidgets()) {
			QRect r = QRect(w->mapToGlobal(w->rect().topLeft()),
			                w->mapToGlobal(w->rect().bottomRight()));
			if (r.contains(mouseEvent->globalPos())) {
				YaRosterToolButton* btn = dynamic_cast<YaRosterToolButton*>(w);
				if (btn) {
					btn->setUnderMouse(true);
				}

				QMouseEvent me(mouseEvent->type(),
				               mouseEvent->globalPos() - r.topLeft(),
				               mouseEvent->globalPos(),
				               mouseEvent->button(),
				               mouseEvent->buttons(),
				               mouseEvent->modifiers());
				filtering = true;
				qApp->sendEvent(w, &me);
				filtering = false;

				found = w;
				break;
			}
		}

		foreach(QWidget* w, ui_.roster->proxyableWidgets()) {
			if (w == found)
				continue;

			YaRosterToolButton* btn = dynamic_cast<YaRosterToolButton*>(w);
			if (btn) {
				btn->setUnderMouse(false);
			}
		}

		if (found) {
			// TODO: currently when YaSelfMoodExtra goes all the way to the
			// right side of the roster, and a click on Filter button is
			// performed, YaSelfMoodExtra is repainted on top of dull gray
			// background, as if YaWindow's paint event didn't function at
			// all. This line of code helps to make sure the dull gray
			// background isn't here to stay, but doesn't fix the flickering
			// problem. This must be investigated further.
			widget->update();

			return false;
		}
	}
	else if (e->type() == QEvent::Leave ||
	         e->type() == QEvent::WindowDeactivate)
	{
		foreach(QWidget* w, ui_.roster->proxyableWidgets()) {
			YaRosterToolButton* btn = dynamic_cast<YaRosterToolButton*>(w);
			if (btn) {
				btn->setUnderMouse(false);
			}
		}
	}
#endif

	return YaOnlineMainWin::eventFilter(obj, e);
}

void YaMainWin::about()
{
	YaAbout::show();
}

void YaMainWin::quitApplication()
{
	static bool quitting = false;
	if (quitting)
		return;

	quitting = true;

#ifndef YAPSI_ACTIVEX_SERVER
	psi_->closeProgram();
#else
	psi_->yaOnline()->doQuit();
#endif

	quitting = false;
}

void YaMainWin::setWindowOpts(bool , bool)
{
}

void YaMainWin::setUseDock(bool)
{
}

void YaMainWin::showNoFocus()
{
	qWarning("YaMainWin::showNoFocus");
}

void YaMainWin::statusSelected()
{
	statusSelected(ui_.selfMood->statusType());
}

void YaMainWin::statusSelectedHelper(XMPP::Status::Type statusType, bool _isManualStatus)
{
	if (statusType != XMPP::Status::Offline) {
#ifdef YAPSI_ACTIVEX_SERVER
		if (!psi_->contactList()->onlineAccount()->enabled()) {
			psi_->contactList()->onlineAccount()->setEnabled(true);
		}
#endif

		if (psi_->contactList()->enabledAccounts().isEmpty() &&
		    !psi_->contactList()->accounts().isEmpty())
		{
			psi_->contactList()->accounts().first()->setEnabled(true);
		}
	}

	bool isManualStatus = _isManualStatus || statusType == lastLoggedInStatusType();
	QString mood = ui_.selfMood->statusText();
	psi_->setStatusFromDialog(makeStatus(statusType, mood), false, isManualStatus);
}

/**
 * Generic call when status was changed.
 */
void YaMainWin::statusSelected(XMPP::Status::Type statusType)
{
	statusSelectedHelper(statusType, false);
}

/**
 * Status was changed via direct user interaction. We should call back Online.
 */
void YaMainWin::statusSelectedManually(XMPP::Status::Type statusType)
{
	QString mood = statusMessage();
	if (mood != DEFAULT_MOOD_TEXT) {
		PsiOptions::instance()->setOption(showDefaultMoodOptionPath, false);
	}

	statusSelectedManuallyHelper(statusType);
	YaOnlineMainWin::statusSelectedManually(statusType);
}

/**
 * Status was changed via direct user interaction, but we shouldn't
 * call Online functions there
 */
void YaMainWin::statusSelectedManuallyHelper(XMPP::Status::Type statusType)
{
	statusSelectedHelper(statusType, true);

	if (statusType != XMPP::Status::Offline) {
		PsiOptions::instance()->setOption(lastLoggedInStatusTypeOptionPath, statusType);
	}
}

void YaMainWin::clearMoods()
{
	YaOnlineMainWin::clearMoods();
	ui_.selfMood->clearMoods();
}

void YaMainWin::setWindowVisible(bool visible)
{
	YaOnlineMainWin::setWindowVisible(visible);
	if (visible) {
		ui_.roster->doSetFocus();
	}
}

void YaMainWin::dndEnabledActionTriggered()
{
	statusSelectedManually(dndEnabledAction_->isChecked() ? XMPP::Status::DND : XMPP::Status::Online);
}

void YaMainWin::staysOnTopTriggered()
{
	PsiOptions::instance()->setOption(alwaysOnTopOptionPath, staysOnTopAction_->isChecked());
}

XMPP::Status::Type YaMainWin::lastLoggedInStatusType() const
{
	int s = PsiOptions::instance()->getOption(lastLoggedInStatusTypeOptionPath).toInt();
	XMPP::Status::Type status = static_cast<XMPP::Status::Type>(s);
	if (s == -1)
		return XMPP::Status::Online;
	return status;
}

void YaMainWin::decorateButton(int value)
{
	YaOnlineMainWin::decorateButton(value);

	dndEnabledAction_->setChecked(statusType() == XMPP::Status::DND);
	ui_.selfMood->setStatusType(statusType());
	statusMenu_->setStatus(statusType());
}

void YaMainWin::resetLastManualStatusSafeGuard()
{
	if (!psi_)
		return;
	foreach(PsiAccount* account, psi_->contactList()->enabledAccounts()) {
		account->resetLastManualStatusSafeGuard();
	}
}

void YaMainWin::moodChanged()
{
	PsiAccount* account = static_cast<PsiAccount*>(sender());
	QString mood = account->mood();
	if (!mood.isEmpty()) {
		PsiOptions::instance()->setOption(showDefaultMoodOptionPath, false);
	}

	if (account == this->account()) {
		if (mood.isEmpty() && PsiOptions::instance()->getOption(showDefaultMoodOptionPath).toBool()) {
			mood = DEFAULT_MOOD_TEXT;
		}
		ui_.selfMood->setStatusText(mood);
	}
}

void YaMainWin::updateReadNext(PsiIcon *, int)
{
	// YaEventNotifier is the king
}

void YaMainWin::optionsUpdate()
{
}

void YaMainWin::setTrayToolTip(const XMPP::Status &, bool)
{
	qWarning("YaMainWin::setTrayToolTip");
}

void YaMainWin::toggleVisible()
{
	setWindowVisible(!isVisible());
}

void YaMainWin::accountContactsChanged()
{
	// updateFriendsFrameVisibilityTimer_->start(10);
}

// void YaMainWin::updateFriendsFrameVisibility()
// {
// 	bool enableFriends = false;
// 	foreach(PsiAccount* account, psi_->contactList()->enabledAccounts()) {
// 		Q3PtrListIterator<UserListItem> it(*account->userList());
// 		UserListItem* item;
// 		while ((item = it.current()) != 0) {
// 			++it;
// 			if (Ya::isInFriends(item)) {
// 				enableFriends = true;
// 				break;
// 			}
// 		}
// 	}
// 
// 	ui_.friendsFrame->setVisible(enableFriends);
// }

void YaMainWin::ipcMessage(const QString& message)
{
	if (message == "activate") {
		bringToFront(this);
	}
#ifndef YAPSI_ACTIVEX_SERVER
	else if (message == "quit" ||
	         message == "quit:installing" ||
	         message == "quit:uninstalling") {
		quitApplication();
	}
#endif
}

void YaMainWin::paintOnlineLogo(QPainter* p)
{
	if (!theme().customFrame())
		return;

	static QPixmap logo;
	if (logo.isNull()) {
		logo = QPixmap(":iconsets/system/default/logo_16.png");
	}
	Q_ASSERT(!logo.isNull());

	// p->setRenderHint(QPainter::TextAntialiasing, true);
	QFont f = p->font();
	// f.setStyleStrategy(QFont::PreferAntialias);
	f.setPixelSize(14);
	f.setBold(false);
	p->setFont(f);

	// QString text = tr("Online");
	QRect textRect(13 + 16 + 2 + additionalLeftMargin() - 5,
	               4 + additionalTopMargin(),
	               width(),
	               23);
	if (!theme().theme().rosterLogoBackground().isNull()) {
		const int leftMargin = 4;
		QRect r(textRect.adjusted(-leftMargin, 2, 0, -2));
		r.setWidth(theme().theme().rosterLogoBackground().width());
		if (theme().theme().rosterLogoBackground().height() == 19) {
			r.adjust(1, -1, 1, -1);
		}
		else {
			r.adjust(5, 3, 5, 3);
		}
		r.setHeight(theme().theme().rosterLogoBackground().height());
		p->drawPixmap(r, theme().theme().rosterLogoBackground());
	}

	p->drawPixmap(13 + additionalLeftMargin() - 6, 7 + additionalTopMargin(), logo);
	// p->setPen(theme().theme().rosterLogoColor());
	// p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
}

void YaMainWin::filterContacts()
{
	ui_.roster->toggleFilterContacts();
}

void YaMainWin::setStatusOnline()
{
	statusSelected(XMPP::Status::Online);
}

void YaMainWin::setStatusDND()
{
	statusSelected(XMPP::Status::DND);
}

void YaMainWin::setStatusOffline()
{
	statusSelected(XMPP::Status::Offline);
}

void YaMainWin::updateSelfWidgetsVisibility()
{
	bool visible = ui_.roster->selfWidgetsShouldBeVisible();
	ui_.topStack->setCurrentWidget(visible ? ui_.selfWidgetsPage : ui_.logoPage);

	ui_.statusBar->setShouldBeVisible(visible);
	if (tray_) {
		tray_->setVisible(visible);
	}
	update(); // re-draw using YaWindowTheme
}

void YaMainWin::activateToShowError(YaInformer* informer)
{
#ifndef YAPSI_ACTIVEX_SERVER
	if (ui_.roster->selfWidgetsShouldBeVisible())
		informer->setVisible(true);
#else
	Q_UNUSED(informer);
#endif
}

void YaMainWin::repaintBackground()
{
	YaOnlineMainWin::repaintBackground();
	ui_.logoPage->repaint();
	ui_.selfWidgetsPage->repaint();
}

void YaMainWin::createPreferences()
{
	if (preferences_)
		return;

	preferences_ = new YaPreferences();
	preferences_->setController(psi_);
}

void YaMainWin::toggleAccounts()
{
	togglePreferences();
	preferences_->openAccounts();
}

void YaMainWin::togglePreferences()
{
	YaOnlineMainWin::togglePreferences();
	createPreferences();
	preferences_->toggle();
}

void YaMainWin::forcePreferencesVisible()
{
	createPreferences();
	preferences_->forceVisible();
}

bool YaMainWin::expandWidthWhenMaximized() const
{
	return false;
}

void YaMainWin::contextMenuEvent(QContextMenuEvent* e)
{
	e->accept();
}

void YaMainWin::optionChanged(const QString& option)
{
	if (option == alwaysOnTopOptionPath) {
		bool visible = isVisible();
		bool alwaysOnTop = PsiOptions::instance()->getOption(alwaysOnTopOptionPath).toBool();
		setStaysOnTop(alwaysOnTop);
		staysOnTopAction_->setChecked(alwaysOnTop);
		if (visible) {
			bringToFront(this);
		}
	}

	YaOnlineMainWin::optionChanged(option);
}

void YaMainWin::paint(QPainter* p)
{
	paintOnlineLogo(p);
	YaOnlineMainWin::paint(p);
}

void YaMainWin::showSelfProfile()
{
	PsiContact* contact = ui_.selfUserpic->selfContact();
	if (contact) {
		QRect rect = ui_.selfInfo->extraGeometry();
		YaChatToolTip::instance()->showText(rect, contact, 0, 0);
	}
}

void YaMainWin::vcardChanged(const Jid& jid)
{
	if (psi_ && psi_->contactList() && psi_->contactList()->accountsLoaded()) {
		PsiAccount* acc = psi_->contactList()->yaServerHistoryAccount();
		if (acc && acc->jid().compare(jid, false)) {
			// selfContact's vcardChanged() slot could be triggered later than
			// YaMainWin's, so we're manually invalidating its cache
			acc->selfContact()->rereadVCard();
			ui_.selfMood->setGender(acc->selfContact()->gender());
#ifdef YAPSI_ACTIVEX_SERVER
			psi_->yaOnline()->setSelfInfo(acc->selfContact());
#endif
		}
	}
}

YaContactListModel* YaMainWin::contactListModel() const
{
	return ui_.roster->contactListModel();
}

void YaMainWin::clearCaches()
{
	if (!psi_ || !psi_->contactList() || !psi_->contactList()->accountsLoaded())
		return;

	VCardFactory::instance()->clearCache();

	foreach(PsiAccount* account, psi_->contactList()->accounts()) {
		account->avatarFactory()->clearCache();
		account->clearRingbuf();
		foreach(ChatDlg* chat, account->findAllDialogs<ChatDlg*>()) {
			chat->doClear();

			if (!chat->isTabbed()) {
				delete chat;
			}
		}
	}
}

#include "yamainwin.moc"
