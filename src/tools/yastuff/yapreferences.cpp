/*
 * yapreferences.cpp - preferences window
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

#include "yapreferences.h"

#include <QSettings>
#include <QCoreApplication>
#include <QFontDialog>
#include <QPainter>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QDomElement>
#include <QMapIterator>
#include <QtAlgorithms>

#include "psicon.h"
#include "psiaccount.h"
#include "common.h"
#include "psioptions.h"
#include "shortcutmanager.h"
#include "psicontactlist.h"
#include "eventdb.h"
#include "psicontact.h"
#include "psitooltip.h"
#include "yavisualutil.h"
#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif
#include "removeconfirmationmessagebox.h"
#include "yamanageaccounts.h"
#include "yawindowtheme.h"
#include "chatdlg.h"
#include "xmpp_xmlcommon.h"
#include "xmpp_tasks.h"
#include "cutejson.h"
#include "JsonToVariant.h"
#include "desktoputil.h"
#include "applicationinfo.h"
#include "yaprogressdialog.h"
#include "xmpp_message.h"

static const QString autoStartRegistryKey = "CurrentVersion/Run/yachat.exe";

YaPreferences::YaPreferences()
	: YaWindow(0)
	, controller_(0)
	, theme_(YaWindowTheme::Roster)
	, accountsPage_(0)
{
	ui_.setupUi(this);
	updateContentsMargins();
	ui_.preferencesPage->installEventFilter(this);

#if defined(Q_WS_WIN) && !defined(YAPSI_ACTIVEX_SERVER)
	ui_.startAutomatically->show();
#else
	ui_.startAutomatically->hide();
#endif

	ui_.ignoreNonRosterContacts->hide();
	ui_.showOfflineContacts->hide();

#ifndef YAPSI_ACTIVEX_SERVER
	ui_.alwaysShowToasters->hide();
#endif

#ifdef Q_WS_MAC
	ui_.ctrlEnterSendsChatMessages->setText(ui_.ctrlEnterSendsChatMessages->text().replace("Ctrl+Enter", QString::fromUtf8("⌘↩")));
#endif

	ui_.clearMessageHistory->setAlignment(Qt::AlignLeft);
	ui_.clearMessageHistory->setEncloseInBrackets(true);

	ui_.showMessageHistory->setAlignment(Qt::AlignLeft);
	ui_.showMessageHistory->setEncloseInBrackets(true);

	connect(ui_.okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(ui_.clearMessageHistory, SIGNAL(clicked()), SLOT(clearMessageHistory()));
	connect(ui_.showMessageHistory, SIGNAL(clicked()), SLOT(showLocalHistory()));
	// ui_.clearMessageHistory->setButtonStyle(YaPushButton::ButtonStyle_Destructive);

	if (PsiOptions::instance()->getOption("options.ui.account.single").toBool()) {
		ui_.manageAccountsButton->hide();
	}

	accountsPage_ = new YaManageAccounts(this);
	ui_.stackedWidget->addWidget(accountsPage_);

	foreach(YaPreferencesTabButton* btn, findChildren<YaPreferencesTabButton*>()) {
		btn->setCheckable(true);
		btn->setButtonStyle(YaPushButton::ButtonStyle_Normal);
		btn->updateGeometry();
	}
	openPreferences();
	connect(ui_.preferencesButton, SIGNAL(clicked()), SLOT(openPreferences()));
	connect(ui_.manageAccountsButton, SIGNAL(clicked()), SLOT(openAccounts()));

	{
		QMap<QString, QString> m;

		m["academic"] = tr("Academic");
		m["baroque"] = tr("Baroque");
		m["glamour"] = tr("Glamour");
		m["hawaii"] = tr("Hawaii");
		m["ice"] = tr("Ice");
		m["sea"] = tr("Sea");
		m["sky"] = tr("Sky");
		m["spring"] = tr("Spring");
		m["violet"] = tr("Violet");

		foreach(QString b, YaWindowTheme::funnyThemes()) {
			Q_ASSERT(m.contains(b));
			ui_.chatBackgroundComboBox->addItem(m[b], b);

			ui_.chatBackgroundComboBox->setItemData(
			    ui_.chatBackgroundComboBox->count() - 1,
			    QVariant(b), PreferenceMappingData);
		}

#if 0
		ui_.chatBackgroundComboBox->addItem(tr("Random"), "random");
		ui_.chatBackgroundComboBox->setItemData(
		    ui_.chatBackgroundComboBox->count() - 1,
		    QVariant("random"), PreferenceMappingData);
#endif
	}

	{
		ui_.contactListAvatarsComboBox->addItem(tr("None"), 0);
		ui_.contactListAvatarsComboBox->setItemData(
		    ui_.contactListAvatarsComboBox->count() - 1,
		    QVariant("none"), PreferenceMappingData);

		ui_.contactListAvatarsComboBox->addItem(tr("Big"), 2);
		ui_.contactListAvatarsComboBox->setItemData(
		    ui_.contactListAvatarsComboBox->count() - 1,
		    QVariant("big"), PreferenceMappingData);

		ui_.contactListAvatarsComboBox->addItem(tr("Small"), 3);
		ui_.contactListAvatarsComboBox->setItemData(
		    ui_.contactListAvatarsComboBox->count() - 1,
		    QVariant("small"), PreferenceMappingData);
/*
		ui_.contactListAvatarsComboBox->addItem(tr("Auto-sized"), 1);
		ui_.contactListAvatarsComboBox->setItemData(
		    ui_.contactListAvatarsComboBox->count() - 1,
		    QVariant("auto"), PreferenceMappingData);
*/
	}

	{
		ui_.offlineEmailsMaxLast->addItem(tr("Never"), -1);
		ui_.offlineEmailsMaxLast->setItemData(
		    ui_.offlineEmailsMaxLast->count() - 1,
		    QVariant("-1"), PreferenceMappingData);

		ui_.offlineEmailsMaxLast->addItem(tr("After 7 days"), 604800);
		ui_.offlineEmailsMaxLast->setItemData(
		    ui_.offlineEmailsMaxLast->count() - 1,
		    QVariant("7"), PreferenceMappingData);

		ui_.offlineEmailsMaxLast->addItem(tr("After 14 days"), 1209600);
		ui_.offlineEmailsMaxLast->setItemData(
		    ui_.offlineEmailsMaxLast->count() - 1,
		    QVariant("14"), PreferenceMappingData);

		ui_.offlineEmailsMaxLast->addItem(tr("After 30 days"), 2592000);
		ui_.offlineEmailsMaxLast->setItemData(
		    ui_.offlineEmailsMaxLast->count() - 1,
		    QVariant("30"), PreferenceMappingData);
	}

	foreach(QFrame* frame, ui_.preferencesFrame->findChildren<QFrame*>()) {
		if (!frame->objectName().startsWith("expandingFillerFrame"))
			continue;
		QGridLayout* grid = dynamic_cast<QGridLayout*>(ui_.preferencesFrame->layout());
		Q_ASSERT(grid);
		if (!grid)
			break;

		int row, column, rowSpan, columnSpan;
		grid->getItemPosition(grid->indexOf(frame), &row, &column, &rowSpan, &columnSpan);
		if (rowSpan > 1) {
			Q_ASSERT(columnSpan == 1);
			Q_ASSERT(column > 0);
			grid->setColumnStretch(column, 100);
		}
		else {
			Q_ASSERT(columnSpan > 1);
			Q_ASSERT(rowSpan == 1);
			Q_ASSERT(row > 0);
			grid->setRowStretch(row, 100);
		}
	}

	preferenceKeyMapping_["ctrl_enter_send"] = ui_.ctrlEnterSendsChatMessages;
	preferenceKeyMapping_["groups_show"] = ui_.showContactListGroups;
	preferenceKeyMapping_["avatars_list"] = ui_.contactListAvatarsComboBox;
	// preferenceKeyMapping_["win_colors"] = ui_.chatBackgroundComboBox;
	preferenceKeyMapping_["show_smiles"] = ui_.enableEmoticons;
	preferenceKeyMapping_["local_history"] = ui_.logMessageHistory;
	preferenceKeyMapping_["font_settings"] = ui_.fontCombo;
	preferenceKeyMapping_["nastr_change_show"] = ui_.showMoodChangePopups;
	preferenceKeyMapping_["msg_notify"] = ui_.showMessageNotifications;
	preferenceKeyMapping_["msg_notify_text"] = ui_.showMessageText;
	// preferenceKeyMapping_["connection_notify"] = ui_.showConnectionNotifications;
	// preferenceKeyMapping_["chat_sounds"] = ui_.playSounds;
	preferenceKeyMapping_["show_offline_contacts"] = ui_.showOfflineContacts;
	preferenceKeyMapping_["publish_mood_on_yaru"] = ui_.publishMoodOnYaru;
	preferenceKeyMapping_["offline_emails_max_last"] = ui_.offlineEmailsMaxLast;
	preferenceKeyMapping_["always_show_toasters"] = ui_.alwaysShowToasters;

	YaPushButton::initAllButtons(this);
#ifdef Q_WS_MAC
	resize(646, 386);
#else
	resize(601, 346);
#endif
	setProperty("show-offscreen", true);
}

YaPreferences::~YaPreferences()
{
	if (controller_) {
		controller_->dialogUnregister(this);
	}
}

void YaPreferences::openPreferences()
{
	setCurrentPage(Page_Preferences);
}

void YaPreferences::openAccounts()
{
	setCurrentPage(Page_Accounts);
}

void YaPreferences::setCurrentPage(YaPreferences::Page page)
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);
	bool pp = page == Page_Preferences;
	ui_.preferencesButton->setChecked(pp);
	ui_.preferencesButton->setEnabled(!pp);
	ui_.manageAccountsButton->setChecked(!pp);
	ui_.manageAccountsButton->setEnabled(pp);

	ui_.stackedWidget->setCurrentWidget(pp ? ui_.preferencesPage : accountsPage_);
	if (!pp) {
		accountsPage_->selectFirstAccount();
	}
	setUpdatesEnabled(updatesEnabled);
}

// TODO: also make sure option saving is snappy
struct Connection {
	Connection(QObject* obj, const char* signal, const char* slot)
		: obj_(obj)
		, signal_(signal)
		, slot_(slot)
	{}

	QObject* obj_;
	const char* signal_;
	const char* slot_;
};

void YaPreferences::setChangedConnectionsEnabled(bool changedConnectionsEnabled)
{
	QList<Connection> connections;
	connections << Connection(ui_.fontCombo, SIGNAL(currentFontChanged(const QFont &)), SLOT(changeChatFontFamily(const QFont &)));
	connections << Connection(ui_.fontSizeCombo, SIGNAL(activated(const QString &)), SLOT(changeChatFontSize(const QString &)));
	connections << Connection(ui_.chatBackgroundComboBox, SIGNAL(currentIndexChanged(int)), SLOT(smthSet()));
	connections << Connection(ui_.contactListAvatarsComboBox, SIGNAL(currentIndexChanged(int)), SLOT(smthSet()));
	connections << Connection(ui_.offlineEmailsMaxLast, SIGNAL(currentIndexChanged(int)), SLOT(smthSet()));
	foreach(QCheckBox* checkBox, findChildren<QCheckBox*>()) {
		connections << Connection(checkBox, SIGNAL(stateChanged(int)), SLOT(smthSet()));
	}

	foreach(Connection c, connections) {
		if (changedConnectionsEnabled)
			connect(c.obj_, c.signal_, this, c.slot_);
		else
			disconnect(c.obj_, c.signal_, this, c.slot_);
	}
}

void YaPreferences::setController(PsiCon* controller)
{
	controller_ = controller;
	Q_ASSERT(controller_);
	controller_->dialogRegister(this);
	accountsPage_->setController(controller_);

#ifdef YAPSI_ACTIVEX_SERVER
	connect(controller_->yaOnline(), SIGNAL(doApplyImmediatePreferences(const QString&)), SLOT(applyImmediatePreferences(const QString&)));
	connect(controller_->yaOnline(), SIGNAL(doApplyPreferences(const QString&)), SLOT(applyPreferences(const QString&)));
	connect(controller_->yaOnline(), SIGNAL(doGetChatPreferences()), SLOT(getChatPreferences()));
	connect(controller_->yaOnline(), SIGNAL(doGetChatAccounts()), SLOT(getChatAccounts()));
#endif

	ui_.yaOnlinePreferences->hide();
}

void YaPreferences::activate()
{
	restore();
}

void YaPreferences::restore()
{
	setChangedConnectionsEnabled(false);
#ifndef YAPSI_ACTIVEX_SERVER
	ui_.playSounds->setChecked(PsiOptions::instance()->getOption("options.ui.notifications.sounds.enable").toBool());
#endif

	ui_.ignoreNonRosterContacts->setChecked(PsiOptions::instance()->getOption("options.messages.ignore-non-roster-contacts").toBool());

#if defined(Q_WS_WIN) && !defined(YAPSI_ACTIVEX_SERVER)
	QSettings autoStartSettings(QSettings::NativeFormat, QSettings::UserScope, "Microsoft", "Windows");
	ui_.startAutomatically->setChecked(autoStartSettings.contains(autoStartRegistryKey));
#else
	ui_.startAutomatically->hide();
#endif

	ui_.showOfflineContacts->setChecked(controller_->contactList()->showOffline());
	ui_.ctrlEnterSendsChatMessages->setChecked(!ShortcutManager::instance()->shortcuts("chat.send").contains(QKeySequence(Qt::Key_Return)));
	ui_.enableEmoticons->setChecked(PsiOptions::instance()->getOption("options.ui.emoticons.use-emoticons").toBool());

	ui_.showContactListGroups->setChecked(PsiOptions::instance()->getOption("options.ya.main-window.contact-list.show-groups").toBool());
	ui_.showMoodChangePopups->setChecked(PsiOptions::instance()->getOption("options.ya.popups.moods.enable").toBool());
	ui_.showMessageNotifications->setChecked(PsiOptions::instance()->getOption("options.ya.popups.message.enable").toBool());
	ui_.showMessageText->setChecked(PsiOptions::instance()->getOption("options.ya.popups.message.show-text").toBool());
	ui_.showMessageText->setEnabled(ui_.showMessageNotifications->isChecked());
#ifndef YAPSI_ACTIVEX_SERVER
	ui_.showConnectionNotifications->setChecked(PsiOptions::instance()->getOption("options.ya.popups.connection.enable").toBool());
#endif
	ui_.publishMoodOnYaru->setChecked(PsiOptions::instance()->getOption("options.ya.publish-mood-on-yaru").toBool());
	ui_.alwaysShowToasters->setChecked(PsiOptions::instance()->getOption("options.ya.popups.always-show-toasters").toBool());

	bool logEnabled = false;
	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		if (account->userAccount().opt_log) {
			logEnabled = true;
			break;
		}
	}
	ui_.logMessageHistory->setChecked(logEnabled);

	chatFont_.fromString(PsiOptions::instance()->getOption("options.ui.look.font.chat").toString());
	ui_.fontCombo->setCurrentFont(chatFont_);
	changeChatFontSize(QString::number(chatFont_.pointSize()));

#ifndef YAPSI_ACTIVEX_SERVER
	int chatBackgroundIndex = ui_.chatBackgroundComboBox->findData(
	                              PsiOptions::instance()->getOption("options.ya.chat-background").toString()
	                          );
	Q_ASSERT(chatBackgroundIndex != -1);
	ui_.chatBackgroundComboBox->setCurrentIndex(chatBackgroundIndex);
#endif

	int avatarStyle = PsiOptions::instance()->getOption("options.ya.main-window.contact-list.avatar-style").toInt();
	int avatarsIndex = ui_.contactListAvatarsComboBox->findData(avatarStyle);
	if (avatarsIndex == -1) {
		avatarsIndex = ui_.contactListAvatarsComboBox->findData("small");
	}
	ui_.contactListAvatarsComboBox->setCurrentIndex(avatarsIndex);

	{
		int data = PsiOptions::instance()->getOption("options.ya.offline-emails-max-last").toInt();
		int index = ui_.offlineEmailsMaxLast->findData(data);
		if (index == -1) {
			index = 0;
		}
		ui_.offlineEmailsMaxLast->setCurrentIndex(index);
	}

	setChangedConnectionsEnabled(true);
}

void YaPreferences::save()
{
#ifndef YAPSI_ACTIVEX_SERVER
	PsiOptions::instance()->setOption("options.ui.notifications.sounds.enable", ui_.playSounds->isChecked());
#endif

	PsiOptions::instance()->setOption("options.messages.ignore-non-roster-contacts", ui_.ignoreNonRosterContacts->isChecked());

#if defined(Q_WS_WIN) && !defined(YAPSI_ACTIVEX_SERVER)
	QSettings autoStartSettings(QSettings::NativeFormat, QSettings::UserScope, "Microsoft", "Windows");
	if (!ui_.startAutomatically->isChecked()) {
		autoStartSettings.remove(autoStartRegistryKey);
	}
	else {
		autoStartSettings.setValue(autoStartRegistryKey,
		                           QCoreApplication::applicationFilePath());
	}
#endif

#ifndef YAPSI_ACTIVEX_SERVER
	PsiOptions::instance()->setOption("options.ya.chat-background",
	                                  ui_.chatBackgroundComboBox->itemData(
	                                      ui_.chatBackgroundComboBox->currentIndex()
	                                  ).toString());
#endif

	controller_->contactList()->setShowOffline(ui_.showOfflineContacts->isChecked());

	// Soft return.
	// Only update this if the value actually changed, or else custom presets
	// might go lost.
	bool soft = ShortcutManager::instance()->shortcuts("chat.send").contains(QKeySequence(Qt::Key_Return));
	if (soft != !ui_.ctrlEnterSendsChatMessages->isChecked()) {
		QVariantList vl;
		if (!ui_.ctrlEnterSendsChatMessages->isChecked()) {
			vl << qVariantFromValue(QKeySequence(Qt::Key_Enter)) << qVariantFromValue(QKeySequence(Qt::Key_Return));
		}
		else  {
			vl << qVariantFromValue(QKeySequence(Qt::Key_Enter+Qt::CTRL)) << qVariantFromValue(QKeySequence(Qt::CTRL+Qt::Key_Return));
		}
		PsiOptions::instance()->setOption("options.shortcuts.chat.send",vl);
	}

	PsiOptions::instance()->setOption("options.ui.emoticons.use-emoticons", ui_.enableEmoticons->isChecked());
	PsiOptions::instance()->setOption("options.ya.emoticons-enabled", ui_.enableEmoticons->isChecked());

	PsiOptions::instance()->setOption("options.ya.main-window.contact-list.show-groups", ui_.showContactListGroups->isChecked());

	PsiOptions::instance()->setOption("options.ya.popups.moods.enable", ui_.showMoodChangePopups->isChecked());

	PsiOptions::instance()->setOption("options.ya.popups.message.enable", ui_.showMessageNotifications->isChecked());

	PsiOptions::instance()->setOption("options.ya.popups.message.show-text", ui_.showMessageText->isChecked());
	ui_.showMessageText->setEnabled(ui_.showMessageNotifications->isChecked());

#ifndef YAPSI_ACTIVEX_SERVER
	PsiOptions::instance()->setOption("options.ya.popups.connection.enable", ui_.showConnectionNotifications->isChecked());
#endif

	if (PsiOptions::instance()->getOption("options.ya.publish-mood-on-yaru").toBool() != ui_.publishMoodOnYaru->isChecked()) {
		PsiOptions::instance()->setOption("options.ya.publish-mood-on-yaru", ui_.publishMoodOnYaru->isChecked());
		if (controller_->contactList()->defaultAccount()) {
			JT_YaMoodSwitch *task = new JT_YaMoodSwitch(controller_->contactList()->defaultAccount()->client()->rootTask());
			task->set(ui_.publishMoodOnYaru->isChecked());
			task->go();
		}
	}

	PsiOptions::instance()->setOption("options.ya.popups.always-show-toasters", ui_.alwaysShowToasters->isChecked());

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		UserAccount ua = account->userAccount();
		ua.opt_log = ui_.logMessageHistory->isChecked();
		account->setUserAccount(ua);
	}

	PsiOptions::instance()->setOption("options.ui.look.font.chat", chatFont_.toString());

	int avatarStyle = 0;
	avatarStyle = ui_.contactListAvatarsComboBox->itemData(
	                  ui_.contactListAvatarsComboBox->currentIndex()
	              ).toInt();
	PsiOptions::instance()->setOption("options.ya.main-window.contact-list.avatar-style",
	                                  avatarStyle);

	{
		int data = ui_.offlineEmailsMaxLast->itemData(
		               ui_.offlineEmailsMaxLast->currentIndex()
		           ).toInt();
		PsiOptions::instance()->setOption("options.ya.offline-emails-max-last", data);
		if (controller_->contactList()->defaultAccount()) {
			JT_YaMaxLast *task = new JT_YaMaxLast(controller_->contactList()->defaultAccount()->client()->rootTask());
			task->set(data);
			task->go();
		}
	}
}

void YaPreferences::clearMessageHistory()
{
	clearMessageHistory(ClearHistory_Local_All, 0);
}

void YaPreferences::clearMessageHistory(YaPreferences::ClearMessageHistoryType type, PsiAccount* account)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("clearMessageHistory");
	root.setAttribute("type", QString::number(type));
	root.setAttribute("account", account ? account->id() : "");
	doc.appendChild(root);

	if (type != ClearHistory_Local_All) {
		Q_ASSERT(account);
		if (!account)
			return;
	}

	QString msg;
	if (type == ClearHistory_Remote) {
		msg = tr("Do you really want to remove <b>all</b> message logs?");
	}
	else {
		msg = tr("Do you really want to remove <b>all</b> message logs from this computer?");
	}

	RemoveConfirmationMessageBoxManager::instance()->
		removeConfirmation(doc.toString(), this, "clearMessageHistoryConfirmation",
		                   tr("Removing message logs"),
		                   msg,
		                   this,
		                   tr("Remove All"));
}

void YaPreferences::clearMessageHistoryConfirmation(const QString& id, bool confirmed)
{
	if (!controller_)
		return;

	QDomDocument doc;
	if (!doc.setContent(id)) {
		Q_ASSERT(false);
		return;
	}

	QDomElement root = doc.documentElement();
	if (root.tagName() != "clearMessageHistory") {
		Q_ASSERT(false);
		return;
	}

	YaPreferences::ClearMessageHistoryType type = static_cast<YaPreferences::ClearMessageHistoryType>(root.attribute("type").toInt());
	PsiAccount* account = controller_->contactList()->getAccount(root.attribute("account"));
	if (type != ClearHistory_Local_All) {
		Q_ASSERT(account);
		if (!account) {
			return;
		}
	}

	if (confirmed) {
		confirmationDelete(type, account);
	}
}

void YaPreferences::changeChatFontFamily(const QFont & font)
{
	chatFont_.setFamily(font.family());
	save();
}

void YaPreferences::changeChatFontSize(const QString & value)
{
	bool ok;
	int size = value.toInt(&ok, 10);
	if (!ok) {
		ui_.fontSizeCombo->setEditText(QString::number(chatFont_.pointSize()));
	}
	else {
		int fontIndex = ui_.fontSizeCombo->findText(value);
		if (fontIndex == -1) {
			ui_.fontSizeCombo->addItem(value);
			fontIndex = ui_.fontSizeCombo->findText(value);
			Q_ASSERT(fontIndex != -1);
		}
		ui_.fontSizeCombo->setCurrentIndex(fontIndex);

		for (int i = 0; i < ui_.fontSizeCombo->count(); ++i) {
			ui_.fontSizeCombo->setItemData(
			    i,
			    QVariant(ui_.fontSizeCombo->itemText(i)), PreferenceMappingData);
		}

		if (chatFont_.pointSize() != size) {
			chatFont_.setPointSize(size);
			save();
		}
	}
}

void YaPreferences::accept()
{
	close();
}

void YaPreferences::smthSet()
{
	save();
}

bool YaPreferences::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::Paint) {
		if (obj == ui_.preferencesFrame) {
			QWidget* w = static_cast<QWidget*>(obj);
			QPainter p(w);
			p.fillRect(w->rect(), Qt::white);
			return true;
		}
		else if (obj == ui_.preferencesPage) {
			QPainter p(ui_.preferencesPage);
			QRect g = ui_.stackedWidget->geometry();
			QRect r = rect().adjusted(-g.left(), -g.top(), 0, 0);
			theme_.paintBackground(&p, r, isActiveWindow());
			return true;
		}
		else if (obj == this) {
			QPainter p(this);
			Ya::VisualUtil::drawWindowTheme(&p, theme(), rect(), yaContentsRect(), showAsActiveWindow());
			Ya::VisualUtil::drawAACorners(&p, theme(), rect(), yaContentsRect());
			return true;
		}
	}

	return YaWindow::eventFilter(obj, e);
}

void YaPreferences::confirmationDelete(YaPreferences::ClearMessageHistoryType type, PsiAccount* specificAccount)
{
	int numLogs = 0;
	// int processedLogs = 0;
	foreach(PsiAccount* account, controller_->contactList()->accounts())
		numLogs += account->contactList().count();

	// QProgressDialog progress(tr("Removing logs..."), tr("Abort"), 0, numLogs, this);
	// progress.setWindowModality(Qt::WindowModal);
	// progress.show();

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		if (type != ClearHistory_Local_All && account != specificAccount) {
			continue;
		}

		if (type == ClearHistory_Remote) {
			if (controller_->contactList()->yaServerHistoryAccount()) {
				JT_YaRemoveHistory *task = new JT_YaRemoveHistory(controller_->contactList()->yaServerHistoryAccount()->client()->rootTask());
				task->remove(controller_->contactList()->yaServerHistoryAccount()->jid(), account->jid());
				task->go(true);
			}
			continue;
		}

		EDBHandle dbHandle(account->edb());
		foreach(PsiContact* contact, account->contactList()) {
			// progress.setValue(processedLogs++);
			qApp->processEvents();

			// if (progress.wasCanceled())
			// 	break;

			dbHandle.erase(contact->account(), contact->jid());
		}

		while (dbHandle.busy())
			QCoreApplication::instance()->processEvents();

#if 0 // ONLINE-2078
		foreach(ChatDlg* chat, account->findAllDialogs<ChatDlg*>()) {
			chat->doClear();
		}
#endif

		EDBFlatFile::File::removeAccountHistoryDir(account);

		// if (progress.wasCanceled())
		// 	break;
	}

	EDBFlatFile::File::removeOldHistoryDir();
#ifdef YAPSI_ACTIVEX_SERVER
	controller_->yaOnline()->clearedMessageHistory();
#endif
}

void YaPreferences::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Escape) {
		close();
		return;
	}
#ifdef Q_WS_MAC
	else if (event->key() == Qt::Key_W && event->modifiers() & Qt::ControlModifier) {
		close();
		return;
	}
#endif
	QWidget::keyPressEvent(event);
}

QString YaPreferences::getPreferencesXml() const
{
	if (!controller_)
		return QString();

	QVariantMap map;
	QMapIterator<QString, QWidget*> it(preferenceKeyMapping_);
	while (it.hasNext()) {
		it.next();

		QVariant value;
		QWidget* w = it.value();
		Q_ASSERT(w);
		if (w->inherits("QCheckBox")) {
			value = QVariant(dynamic_cast<QCheckBox*>(w)->isChecked());
		}
		else if (w->inherits("QFontComboBox")) {
			QFontComboBox* combo = dynamic_cast<QFontComboBox*>(w);
			Q_ASSERT(combo);
			QVariantMap fmap;
			int pointHeight = ui_.fontSizeCombo->itemData(
			                      ui_.fontSizeCombo->currentIndex(),
			                      PreferenceMappingData).toInt();
			fmap["points"] = pointHeight;

			QFontMetrics fm(chatFont_);
			int pixelHeight = fm.ascent();
			if (pointHeight < 12)
				pixelHeight -= 1;
			else if (pointHeight < 20)
				pixelHeight -= 2;
			else if (pointHeight < 36)
				pixelHeight -= 3;
			fmap["px"] = pixelHeight; // only for correct display on Online side in preferences
			fmap["weight"] = chatFont_.bold() ? "bold" : "regular";
			fmap["italic"] = chatFont_.italic();
			fmap["underline"] = chatFont_.underline();
			fmap["strikeout"] = chatFont_.strikeOut();
			fmap["facename"] = combo->currentFont().family();
			fmap["color"] = PsiOptions::instance()->getOption("options.ya.chat-window.text-color").value<QColor>().name();
			
			value = fmap;
		}
		else if (w->inherits("QComboBox")) {
			QComboBox* combo = dynamic_cast<QComboBox*>(w);
			Q_ASSERT(combo);
			value = combo->itemData(combo->currentIndex(), PreferenceMappingData).toString();
		}

		if (value.isNull()) {
			continue;
		}

		map.insert(it.key(), value);
	}

	return CuteJson::variantToJson(map);
}

void YaPreferences::applyPreferences(const QString& xml)
{
	if (!controller_)
		return;

	QVariant variant;
	try {
		variant = JsonQt::JsonToVariant::parse(xml);
	}
	catch(...) {
		// boom!
	}
	QVariantMap map = variant.toMap();

	QMapIterator<QString, QVariant> it(map);
	while (it.hasNext()) {
		it.next();

		QString id = it.key();
		if (preferenceKeyMapping_.contains(id)) {
			QVariant value = it.value();
			QWidget* w = preferenceKeyMapping_[id];
			Q_ASSERT(w);
			if (w->inherits("QCheckBox")) {
				QCheckBox* check = dynamic_cast<QCheckBox*>(w);
				Q_ASSERT(check);
				check->setChecked(value.toBool());
			}
			else if (w->inherits("QFontComboBox")) {
				QFontComboBox* combo = dynamic_cast<QFontComboBox*>(w);
				Q_ASSERT(combo);

				QVariantMap fmap = value.toMap();
				QString facename = fmap["facename"].toString();
				QFont font(facename);
				combo->setFont(font);
				changeChatFontFamily(facename);
				changeChatFontSize(fmap["points"].toString());
				chatFont_.setBold(fmap["weight"].toString() == "bold");
				chatFont_.setItalic(fmap["italic"].toBool());
				chatFont_.setUnderline(fmap["underline"].toBool());
				chatFont_.setStrikeOut(fmap["strikeout"].toBool());
				PsiOptions::instance()->setOption("options.ya.chat-window.text-color", QColor(fmap["color"].toString()));
			}
			else if (w->inherits("QComboBox")) {
				QComboBox* combo = dynamic_cast<QComboBox*>(w);
				Q_ASSERT(combo);

				combo->setCurrentIndex(
				    combo->findData(
				        value.toString(), PreferenceMappingData
				    ));
			}
		}
	}

	save();
}

void YaPreferences::toggle()
{
	moveToCenterOfScreen();
	activate();
	openPreferences();
#ifndef YAPSI_ACTIVEX_SERVER
	bringToFront(this);
#endif

#ifdef YAPSI_ACTIVEX_SERVER
	Q_ASSERT(controller_);
	if (!controller_)
		return;
	controller_->yaOnline()->showPreferences();
#endif
}

void YaPreferences::getChatPreferences()
{
#ifdef YAPSI_ACTIVEX_SERVER
	accountsPage_->setSendModelUpdates(true);
	controller_->yaOnline()->setPreferences(getPreferencesXml());

	controller_->yaOnline()->setAccounts(getChatAccounts());
#endif
}

QString YaPreferences::getChatAccounts()
{
#ifdef YAPSI_ACTIVEX_SERVER
	QDomDocument accountsDoc = accountsPage_->getAccountsXml();
	return accountsDoc.toString();
#endif
	return QString();
}

void YaPreferences::forceVisible()
{
	toggle();
	bringToFront(this);
}

void YaPreferences::applyImmediatePreferences(const QString& xml)
{
	if (!controller_)
		return;

	QDomDocument doc;
	if (!doc.setContent(xml))
		return;

	QDomElement root = doc.documentElement();
	if (root.tagName() != "action")
		return;

	if (root.attribute("type") == "history_clear") {
		clearMessageHistory();
	}
	else if (root.attribute("type") == "history_clear_local") {
		if (root.hasAttribute("id")) {
			Q_ASSERT(!root.attribute("id").isEmpty());
			clearMessageHistory(ClearHistory_Local, controller_->contactList()->getAccount(root.attribute("id")));
		}
		else {
			clearMessageHistory();
		}
	}
	else if (root.attribute("type") == "history_clear_server") {
		Q_ASSERT(!root.attribute("id").isEmpty());
		clearMessageHistory(ClearHistory_Remote, controller_->contactList()->getAccount(root.attribute("id")));
	}
	else if (root.attribute("type") == "history_show_local") {
		showLocalHistory();
	}
}

void YaPreferences::convertHistory(PsiContact* contact)
{
	Q_ASSERT(contact);
	if (!contact || !contact->account() || !contact->account()->edb()) {
		return;
	}

	while (contact->account()->edb()->hasPendingRequests()) {
		QCoreApplication::instance()->processEvents();
	}

	contact->account()->edb()->closeHandles();

	if (!EDBFlatFile::File::historyExists(contact->account(), contact->jid()) ||
	    QFile::exists(EDBFlatFile::File::jidToYaFileName(contact->account(), contact->jid())))
	{
		return;
	}

	QList<PsiEvent*> events;
	EDBHandle exp(contact->account()->edb());
	QString id;

	while (true) {
		exp.get(contact->account(), contact->jid(), id, EDB::Forward, 1000);

		while (exp.busy()) {
			QCoreApplication::instance()->processEvents();
		}

		const EDBResult* r = exp.result();
		if (r && r->count() > 0) {
			Q3PtrListIterator<EDBItem> it(*r);
			for (; it.current(); ++it) {
				id = it.current()->nextId();
				PsiEvent *e = it.current()->event();
				Q_ASSERT(e);
				Q_ASSERT(!e->account());
				e->setAccount(contact->account());

				PsiEvent* copy = e->copy();
				Q_ASSERT(copy);
				Q_ASSERT(copy->account());
				if (!e->jid().isEmpty()) {
					Q_ASSERT(!copy->jid().isEmpty());
				}
				events << copy;
			}
		}
		else {
			break;
		}

		if (id.isEmpty()) {
			break;
		}
	}

	QFile f(EDBFlatFile::File::jidToYaFileName(contact->account(), contact->jid()));
	if (!f.open(QIODevice::WriteOnly)) {
		return;
	}
	f.close();

	contact->account()->edb()->closeHandles();

	foreach(PsiEvent* event, events) {
		Q_ASSERT(event);
		QPointer<EDBHandle> h = new EDBHandle(contact->account()->edb());
		connect(h, SIGNAL(finished()), SLOT(edb_finished()));
		h->append(contact->account(), contact->jid(), event);

		while (h) {
			QCoreApplication::instance()->processEvents();
		}
	}

	qDeleteAll(events);
}

void YaPreferences::edb_finished()
{
	EDBHandle *h = (EDBHandle *)sender();
	delete h;
}

void YaPreferences::showLocalHistory()
{
	int numLogs = 0;
	int processedLogs = 0;
	foreach(PsiAccount* account, controller_->contactList()->accounts())
		numLogs += account->contactList().count();

	YaProgressDialog progress(tr("Converting history..."), tr("Abort"), 0, numLogs, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.show();

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		foreach(PsiContact* contact, account->contactList()) {
			convertHistory(contact);

			progress.setValue(processedLogs++);
			qApp->processEvents();

			if (progress.wasCanceled())
				break;
		}

		if (progress.wasCanceled())
			break;

		EDBFlatFile::File::removeAccountHistoryDir(account, true);
	}

	if (!progress.wasCanceled()) {
		DesktopUtil::openUrl(QString("file://%1").arg(ApplicationInfo::yahistoryDir()));
	}
}

const YaWindowTheme& YaPreferences::theme() const
{
	return theme_;
}
