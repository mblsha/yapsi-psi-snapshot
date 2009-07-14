/*
 * yamanageaccounts.cpp - account editor
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

#include "yamanageaccounts.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QTimer>
#include <QDomElement>

#include "psicon.h"
#include "yastyle.h"
#include "accountinformermodel.h"
#include "psiaccount.h"
#include "yaremoveconfirmationmessagebox.h"
#include "shortcutmanager.h"
#include "yapushbutton.h"
#include "xmpp_xmlcommon.h"
#include "psicontactlist.h"
#include "psiaccount.h"
#include "profiles.h"
#include "yavisualutil.h"
#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif
#include "yacommon.h"

YaManageAccounts::YaManageAccounts(QWidget* parent)
	: QWidget(parent)
	, controller_(0)
	, removeAction_(0)
	, theme_(YaWindowTheme::Roster)
	, modelChangedTimer_(0)
	, sendModelUpdates_(false)
{
	// setStyle(YaStyle::defaultStyle());

	model_ = new AccountInformerModel(this);
	connect(model_, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateRemoveAction()));
	connect(model_, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateRemoveAction()));

	modelChangedTimer_ = new QTimer(this);
	modelChangedTimer_->setSingleShot(false);
	modelChangedTimer_->setInterval(100);
	connect(modelChangedTimer_, SIGNAL(timeout()), SLOT(modelChanged()));
	QTimer::singleShot(100, this, SLOT(modelChanged()));

	Q_ASSERT(modelChangedTimer_);
	connect(model_, SIGNAL(rowsInserted(const QModelIndex&, int, int)), modelChangedTimer_, SLOT(start()));
	connect(model_, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), modelChangedTimer_, SLOT(start()));
	connect(model_, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), modelChangedTimer_, SLOT(start()));
	connect(model_, SIGNAL(layoutChanged()), modelChangedTimer_, SLOT(start()));
	connect(model_, SIGNAL(modelReset()), modelChangedTimer_, SLOT(start()));

	xmlConsoleAction_ = new QAction(tr("XML Console"), this);
	connect(xmlConsoleAction_, SIGNAL(triggered()), SLOT(xmlConsole()));

	ui_.setupUi(this);
	ui_.treeView->installEventFilter(this);
	connect(ui_.treeView, SIGNAL(deleteAccount()), SLOT(deleteAccount()));

	removeAction_ = new QAction(this);
	removeAction_->setShortcuts(ShortcutManager::instance()->shortcuts("contactlist.delete"));
	connect(removeAction_, SIGNAL(activated()), SLOT(deleteAccount()));
	ui_.treeView->addAction(removeAction_);

	ui_.treeView->setModel(model_);
	connect(ui_.treeView, SIGNAL(selectionChanged(const QItemSelection&)), SLOT(updateRemoveAction()));

	YaPushButton* closeButton = new YaPushButton(this);
	closeButton->setText(tr("Close"));
	ui_.buttonBox->addButton(closeButton, QDialogButtonBox::RejectRole);
	connect(closeButton, SIGNAL(clicked()), window(), SLOT(close()));

	connect(ui_.addButton, SIGNAL(clicked()), SLOT(addAccount()));
	connect(ui_.deleteButton, SIGNAL(clicked()), SLOT(deleteAccount()));
	ui_.deleteButton->setButtonStyle(YaPushButton::ButtonStyle_Destructive);
	updateRemoveAction();

	resize(482, 340);

	ui_.treeView->setFocus();
	if (model_->rowCount(QModelIndex()) > 0) {
		QModelIndex index = model_->index(0, 0, QModelIndex());
		if (index.isValid()) {
			ui_.treeView->setCurrentIndex(index);
		}
	}

	YaPushButton::initAllButtons(this);
}

YaManageAccounts::~YaManageAccounts()
{
}

void YaManageAccounts::setController(PsiCon* controller)
{
	controller_ = controller;
	Q_ASSERT(controller_);
	model_->setContactList(controller_->contactList());

#ifdef YAPSI_ACTIVEX_SERVER
	connect(controller_->yaOnline(), SIGNAL(doApplyImmediatePreferences(const QString&)), SLOT(applyImmediatePreferences(const QString&)));
	connect(controller_->yaOnline(), SIGNAL(doStopAccountUpdates()), SLOT(stopAccountUpdates()));
#endif
}

bool YaManageAccounts::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::ContextMenu && obj == ui_.treeView) {
		QContextMenuEvent* contextMenuEvent = static_cast<QContextMenuEvent*>(e);

		// this is necessary as the actions operate on the selected item
		QModelIndex index = ui_.treeView->indexAt(contextMenuEvent->pos());
		ui_.treeView->setCurrentIndex(index);

		if (!contextMenuEvent->modifiers().testFlag(Qt::AltModifier))
			return true;

		QMenu menu;
		menu.addAction(xmlConsoleAction_);
		menu.exec(ui_.treeView->mapToGlobal(contextMenuEvent->pos()));

		ui_.treeView->repairMouseTracking();
		return true;
	}

	return QWidget::eventFilter(obj, e);
}

void YaManageAccounts::addAccount()
{
	Q_ASSERT(controller_);
	PsiAccount* account = controller_->createAccount();
	account->setEnabled(false);
	Q_ASSERT(account);
	Q_UNUSED(account);

	ui_.treeView->editNewAccount();
}

void YaManageAccounts::deleteAccount()
{
	QModelIndex index = ui_.treeView->selectedIndex();
	if (index.isValid()) {
		selectedAccount_ = index.data(Qt::DisplayRole).toString();
		PsiAccount* account = model_->accountForIndex(index);
		Q_ASSERT(account);
		if (model_->editMode(index) != AccountInformerModel::EditPassword) {
			confirmationDelete();
		}
		else if (account) {
			QDomDocument doc;
			QDomElement root = doc.createElement("action");
			doc.appendChild(root);
			root.setAttribute("type", "account_remove");

			QDomElement a = doc.createElement("account");
			a.setAttribute("id", account->id());
			root.appendChild(a);

			deleteAccount(doc.toString());
		}
	}
}

void YaManageAccounts::deleteAccount(const QString& xml)
{
	QDomDocument doc;
	if (!doc.setContent(xml))
		return;

	QDomElement root = doc.documentElement();
	if (root.tagName() != "action" || root.attribute("type") != "account_remove")
		return;

	QDomElement e = findSubTag(root, "account", 0);
	PsiAccount* account = controller_->contactList()->getAccount(e.attribute("id"));

	if (account) {
		YaRemoveConfirmationMessageBoxManager::instance()->
			removeConfirmation(xml, this, "deleteAccountConfirmation",
			                   tr("Deleting account"),
			                   tr("Do you really want to delete <b>%1</b>?")
							   .arg(account->name()),
			                   this);
	}
}

void YaManageAccounts::deleteAccountConfirmation(const QString& id, bool confirmed)
{
	if (confirmed) {
		applyImmediatePreferencesHelper(id, true);
	}
}

void YaManageAccounts::updateRemoveAction()
{
	QModelIndexList selected = ui_.treeView->selectedIndexes();
	bool enableDelete = !selected.isEmpty() && (model_->rowCount(QModelIndex()) > 1);
#ifdef YAPSI_ACTIVEX_SERVER
	if (!selected.isEmpty()) {
		PsiAccount* account = model_->accountForIndex(selected.first());
		Q_ASSERT(account);
		enableDelete = account ? account->userAccount().saveable : false;
	}
#endif
	ui_.deleteButton->setEnabled(enableDelete);
	removeAction_->setEnabled(enableDelete);
	xmlConsoleAction_->setEnabled(!selected.isEmpty());
}

void YaManageAccounts::xmlConsole()
{
	QModelIndex index = ui_.treeView->selectedIndex();
	PsiAccount* account = index.isValid() ? model_->accountForIndex(index) : 0;
	if (account) {
		account->showXmlConsole();
	}
}

void YaManageAccounts::confirmationDelete()
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);
	QModelIndex index = ui_.treeView->selectedIndex();
	if (index.isValid()) {
		if (selectedAccount_ == index.data(Qt::DisplayRole).toString()) {
			PsiAccount* account = model_->accountForIndex(index);
			if (!account)
				return;
#ifdef YAPSI_ACTIVEX_SERVER
			Q_ASSERT(account->userAccount().saveable);
#endif
			delete account;
		}
	}
	setUpdatesEnabled(updatesEnabled);
}

void YaManageAccounts::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	theme_.paintBackground(&p, rect(), isActiveWindow());
}

void YaManageAccounts::selectFirstAccount()
{
	if (model_->rowCount() > 0) {
		ui_.treeView->setCurrentIndex(model_->index(0, 0));
	}
}

QDomDocument YaManageAccounts::getAccountsXml() const
{
	QDomDocument doc;
	if (!controller_)
		return doc;

	modelChangedTimer_->stop();

	QDomElement root = doc.createElement("action");
	root.setAttribute("type", "accounts");
	doc.appendChild(root);

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		Q_ASSERT(account);
		bool saveable = true;
#ifdef YAPSI_ACTIVEX_SERVER
		saveable = account->userAccount().saveable;
#endif

		QDomElement tag = textTag(&doc, "account", QString());
		tag.setAttribute("id", account->id());
		tag.setAttribute("avatar", Ya::VisualUtil::scaledAvatarPath(account, account->jid(), 50));
		tag.setAttribute("status", Ya::statusToFlash(model_->accountStatus(account)));
		tag.setAttribute("builtin", (int)!saveable);
		tag.setAttribute("error_message", account->currentConnectionError());
		tag.setAttribute("jid", account->jid().bare());
		tag.setAttribute("password", saveable ? account->userAccount().pass : QString());

		tag.setAttribute("jabber_service", account->userAccount().jabber_service);
		tag.setAttribute("ssl", account->userAccount().ssl != UserAccount::SSL_No);
		// tag.setAttribute("local_history", account->userAccount().opt_log ? "1" : "0");
		tag.setAttribute("server_history", account->userAccount().opt_log_on_server ? "1" : "0");

		tag.setAttribute("use_manual_host", account->userAccount().opt_host ? "1" : "0");
		tag.setAttribute("manual_host", account->userAccount().host);
		tag.setAttribute("manual_port", account->userAccount().port);
		root.appendChild(tag);
	}

	return doc;
}

void YaManageAccounts::modelChanged()
{
	if (!controller_)
		return;

	QDomDocument doc;
	QDomElement docRoot = doc.createElement("action");
	docRoot.setAttribute("type", "accounts");
	doc.appendChild(docRoot);
	int docCount = 0;

	QDomDocument newAccounts = getAccountsXml();
	QStringList processedAccounts;

	{
		QDomElement root = newAccounts.documentElement();
		for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if (e.isNull())
				continue;

			if (e.tagName() == "account") {
				// qWarning("id = %s", qPrintable(e.attribute("id")));

				Q_ASSERT(!e.attribute("id").isEmpty());
				processedAccounts << e.attribute("id");
				bool changedAttributes = false;
				bool found = false;
				QDomElement oldRoot = accountsXml_.documentElement();
				for (QDomNode n2 = oldRoot.firstChild(); !n2.isNull() && !found; n2 = n2.nextSibling()) {
					QDomElement e2 = n2.toElement();
					if (e2.isNull())
						continue;

					if (e2.tagName() == "account" && e2.attribute("id") == e.attribute("id")) {
						found = true;
						// qWarning("01: e2 e");
						for (int i = 0; i < e.attributes().count(); ++i) {
							QString attrName = e.attributes().item(i).nodeName();
							if (e.attribute(attrName) != e2.attribute(attrName)) {
								changedAttributes = true;
								// qWarning("02: e2 e");
								break;
							}
						}
					}
				}

				// qWarning("changedAttributes = %d, found = %d", changedAttributes, found);
				if (!changedAttributes && found)
					continue;

				{
					QDomElement tag = textTag(&doc, "account", QString());

					QDomElement oldTag;
					for (QDomNode n2 = oldRoot.firstChild(); !n2.isNull() && oldTag.isNull(); n2 = n2.nextSibling()) {
						QDomElement e2 = n2.toElement();
						if (e2.isNull())
							continue;

						if (e2.tagName() == "account" && e2.attribute("id") == e.attribute("id")) {
							oldTag = e2;
							// qWarning("111oldTag.isNull = %d", oldTag.isNull());
							break;
						}
					}

					// qWarning("oldTag.isNull = %d", oldTag.isNull());

					for (int i = 0; i < e.attributes().count(); ++i) {
						QString attrName = e.attributes().item(i).nodeName();
						bool setAttr = attrName == "id" || !found || e.attribute(attrName) != oldTag.attribute(attrName);
						// qWarning("> %s : %d %d", qPrintable(attrName), !found, e.attribute(attrName) != oldTag.attribute(attrName));
						if (setAttr) {
							tag.setAttribute(attrName, e.attribute(attrName));
						}
					}

					docRoot.appendChild(tag);
					docCount++;
				}
			}
		}
	}

	{
		QDomElement root = accountsXml_.documentElement();
		for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if (e.isNull())
				continue;

			if (e.tagName() == "account") {
				Q_ASSERT(!e.attribute("id").isEmpty());
				if (!processedAccounts.contains(e.attribute("id"))) {
					QDomDocument doc2;
					QDomElement doc2Root = doc2.createElement("action");
					doc2Root.setAttribute("type", "account_removed");
					doc2.appendChild(doc2Root);

					QDomElement tag = textTag(&doc2, "account", QString());
					tag.setAttribute("id", e.attribute("id"));
					doc2Root.appendChild(tag);

					// qWarning("%s", qPrintable(doc2.toString()));
#ifdef YAPSI_ACTIVEX_SERVER
					controller_->yaOnline()->setImmediatePreferences(doc2.toString());
#endif
				}
			}
		}
	}

	accountsXml_ = newAccounts;

	if (docCount && sendModelUpdates_) {
		// qWarning("%s", qPrintable(doc.toString()));

#ifdef YAPSI_ACTIVEX_SERVER
		controller_->yaOnline()->setImmediatePreferences(doc.toString());
#endif
	}
}

void YaManageAccounts::applyImmediatePreferences(const QString& xml)
{
	applyImmediatePreferencesHelper(xml, false);
}

void YaManageAccounts::applyImmediatePreferencesHelper(const QString& xml, bool confirmation)
{
	if (!controller_)
		return;

	QDomDocument doc;
	if (!doc.setContent(xml))
		return;

	QDomElement root = doc.documentElement();
	if (root.tagName() != "action" || !root.attribute("type").startsWith("account_"))
		return;

	enum ActionType {
		Account_Connect = 0,
		Account_Disconnect,
		Account_Remove,
		Account_Change,
		Account_Add,

		Account_Invalid = -1
	};
	ActionType actionType = Account_Invalid;

	if (root.attribute("type") == "account_connect") {
		actionType = Account_Connect;
	}
	else if (root.attribute("type") == "account_disconnect") {
		actionType = Account_Disconnect;
	}
	else if (root.attribute("type") == "account_remove") {
		actionType = Account_Remove;
		if (!confirmation) {
			deleteAccount(xml);
			return;
		}
	}
	else if (root.attribute("type") == "account_change") {
		actionType = Account_Change;
	}
	else if (root.attribute("type") == "account_add") {
		actionType = Account_Add;
	}
	else {
		qWarning("YaManageAccounts::applyImmediatePreferences(): invalid type: %s", qPrintable(root.attribute("type")));
		return;
	}

	for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;

		if (e.tagName() == "account") {
			PsiAccount* account = controller_->contactList()->getAccount(e.attribute("id"));
			if (account && (actionType == Account_Connect || actionType == Account_Disconnect)) {
				model_->setAccountEnabled(account, actionType == Account_Connect);
			}
			else if (account && actionType == Account_Remove) {
#ifdef YAPSI_ACTIVEX_SERVER
				Q_ASSERT(account->userAccount().saveable);
#endif
				delete account;
			}
			else if (actionType == Account_Change || actionType == Account_Add) {
				if (!account) {
					account = controller_->createAccount();
					account->setEnabled(false);
				}

				updateUserAccount(account, e, actionType == Account_Add);
			}
		}
	}

	modelChangedTimer_->start();
}

bool YaManageAccounts::updateUserAccount(PsiAccount* account, const QDomElement& e, bool forceReconnect) const
{
	Q_ASSERT(account);
	bool saveable = true;
	bool reconnect = false;
#ifdef YAPSI_ACTIVEX_SERVER
	saveable = account->userAccount().saveable;
#endif
	if (!account)
		return false;
	UserAccount ua = account->userAccount();
	ua.id = e.attribute("id");
	if (e.hasAttribute("jid") && saveable) {
		if (ua.jid != e.attribute("jid"))
			reconnect = true;

		ua.jid = e.attribute("jid");
	}
	if (e.hasAttribute("password") && saveable) {
		if (ua.pass != e.attribute("password"))
			reconnect = true;

		ua.pass = e.attribute("password");
	}
	if (e.hasAttribute("jabber_service") && saveable) {
		if (ua.jabber_service != e.attribute("jabber_service"))
			reconnect = true;

		ua.jabber_service = e.attribute("jabber_service");
	}
	if (e.hasAttribute("ssl") && saveable) {
		UserAccount::SSLFlag ssl = e.attribute("ssl") == "1" ? UserAccount::SSL_Yes : UserAccount::SSL_No;
		if (ua.ssl != ssl)
			reconnect = true;

		ua.ssl = ssl;
	}
	// if (e.hasAttribute("local_history")) {
	//	ua.opt_log = e.attribute("local_history") == "1";
	// }
	if (e.hasAttribute("server_history")) {
		bool enable = e.attribute("server_history") == "1";
		Message m("history.ya.ru");
		m.setBody(enable ? "s" : "u");
		PsiAccount* baseAccount = 0;
		if (account->isOnlineAccount()) {
			baseAccount = account;
		}
		else {
#ifdef YAPSI_ACTIVEX_SERVER
			baseAccount = controller_->contactList()->onlineAccount();
#else
			baseAccount = controller_->contactList()->yaServerHistoryAccount();
#endif
			m.setTwin(account->jid().bare());
		}
		if (baseAccount && baseAccount->isYaAccount()) {
			baseAccount->dj_sendMessage(m, false);
		}
		ua.opt_log_on_server = enable;
	}
	if (e.hasAttribute("use_manual_host") && saveable) {
		bool useManualHost = e.attribute("use_manual_host") == "1";
		if (ua.opt_host != useManualHost)
			reconnect = true;

		ua.opt_host = useManualHost;
	}
	if (e.hasAttribute("manual_host") && saveable) {
		if (ua.host != e.attribute("manual_host"))
			reconnect = true;

		ua.host = e.attribute("manual_host");
	}
	if (e.hasAttribute("manual_port") && e.attribute("manual_port").toInt() > 0 && saveable) {
		if (ua.port != e.attribute("manual_port").toInt())
			reconnect = true;

		ua.port = e.attribute("manual_port").toInt();
	}

	account->setUserAccount(ua);
	account->clearCurrentConnectionError();
	if (forceReconnect || (saveable && reconnect && account->enabled())) {
		account->setEnabled(false);
		account->setEnabled(!ua.pass.isEmpty());
		account->autoLogin();
	}

	return true;
}

bool YaManageAccounts::sendModelUpdates() const
{
	return sendModelUpdates_;
}

void YaManageAccounts::setSendModelUpdates(bool enable)
{
	sendModelUpdates_ = enable;
}

void YaManageAccounts::stopAccountUpdates()
{
	setSendModelUpdates(false);
}
