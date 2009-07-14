/*
 * accountinformermodel.cpp - editable account list model
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

#include "accountinformermodel.h"

#include <QTimer>

#include "psiaccount.h"
#include "psicontactlist.h"
#include "avatars.h"
#include "xmpp_clientstream.h"
#include "psicon.h"
#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif
#include "accountinformerdelegate.h"

AccountInformerModel::AccountInformerModel(QObject* parent)
	: QStandardItemModel(parent)
{
}

AccountInformerModel::~AccountInformerModel()
{
}

// QModelIndex AccountInformerModel::addAccount(const QString& jid, XMPP::Status::Type status, bool enabled)
// {
// 	QStandardItem* item = new QStandardItem(jid);
// 	item->setData(QVariant(enabled), EnabledRole);
// 	item->setData(QVariant(status), StatusTypeRole);
// 	item->setData(QVariant(QString("123")), PasswordRole);
// 
// 	int rowNum = invisibleRootItem()->rowCount();
// 	invisibleRootItem()->insertRow(rowNum, item);
// 
// 	return item->index();
// }

QVariant AccountInformerModel::data(const QModelIndex& index, int role) const
{
	QStandardItem* item = itemFromIndex(index);

	switch (role) {
	case Qt::DisplayRole:
		if (item) {
			if (editMode(index) == EditDisabled)
				return QVariant(tr("%1 (Ya.Online)").arg(item->data(Qt::DisplayRole).toString()));
			else if (editMode(index) == EditPassword)
				return item->data(Qt::DisplayRole);
			else
				return QVariant(tr("Enter JID to add account"));
		}
		break;
	case Qt::EditRole:
		if (item) {
			if (editMode(index) == EditDisabled)
				return QVariant();
			else if (editMode(index) == EditPassword)
				return item->data(PasswordRole);
			else
				return item->data(Qt::DisplayRole);
		}
		break;
	default:
		; // nothing
	}

	return QStandardItemModel::data(index, role);
}

bool AccountInformerModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
	QStandardItem* item = itemFromIndex(index);
	PsiAccount* account = accountForItem(item);
	if (!account)
		return false;

	switch (role) {
	case Qt::EditRole:
		if (item) {
			UserAccount ua = account->userAccount();
			if (editMode(index) == EditDisabled) {
				// do nothing
			}
			else if (editMode(index) == EditPassword) {
				if (account->isActive()) {
					// TODO: need some sure way to disconnect
					account->setStatus(XMPP::Status::Offline);
				}
				ua.pass = data.toString();
			}
			else {
				XMPP::Jid jid(data.toString());
				if (!jid.user().isEmpty() && !jid.host().isEmpty() && jid.resource().isEmpty()) {
					ua.jid = jid.userHost();
				}
			}
			account->setUserAccount(ua);
			account->clearCurrentConnectionError();
			account->setEnabled(!ua.pass.isEmpty());
			account->autoLogin();
			return true;
		}
		break;
	case ChangeEnabledRole:
		if (item) {
			if (account->userAccount().pass.isEmpty()) {
				account->setEnabled(false);
				emit editPassword();
			}
			else {
				setAccountEnabled(account, !account->enabled());
			}
			return true;
		}
		break;
	default:
		; // nothing
	}

	return QStandardItemModel::setData(index, data, role);
}

void AccountInformerModel::setAccountEnabled(PsiAccount* account, bool enabled) const
{
	Q_ASSERT(account);
	account->clearCurrentConnectionError();
	account->setEnabled(enabled);
	account->autoLogin();
}

AccountInformerModel::EditMode AccountInformerModel::editMode(const QStandardItem* item) const
{
	PsiAccount* account = accountForItem(const_cast<QStandardItem*>(item));
	if (account) {
#ifdef YAPSI_ACTIVEX_SERVER
		if (!account->userAccount().saveable) {
			return EditDisabled;
		}
#endif

		if (!item->data(Qt::DisplayRole).toString().isEmpty()) {
			return EditPassword;
		}
	}

	return EditJID;
}

AccountInformerModel::EditMode AccountInformerModel::editMode(const QModelIndex& index) const
{
	return editMode(itemFromIndex(index));
}

bool AccountInformerModel::indexEditable(const QModelIndex& index) const
{
	return editMode(index) == EditPassword;
}

void AccountInformerModel::setContactList(PsiContactList* contactList)
{
	if (!contactList_.isNull()) {
		disconnect(contactList_, SIGNAL(accountCountChanged()), this, SLOT(accountCountChanged()));
	}
	contactList_ = contactList;
	if (!contactList_.isNull()) {
		connect(contactList_, SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
		accountCountChanged();
	}
}

void AccountInformerModel::accountCountChanged()
{
	Q_ASSERT(!contactList_.isNull());
	foreach(PsiAccount* account, contactList_->accounts()) {
		disconnect(account, SIGNAL(updatedActivity()), this, SLOT(updatedActivity()));
		connect(account,    SIGNAL(updatedActivity()), this, SLOT(updatedActivity()));

		disconnect(account, SIGNAL(updatedAccount()), this, SLOT(updatedAccount()));
		connect(account,    SIGNAL(updatedAccount()), this, SLOT(updatedAccount()));

		disconnect(account, SIGNAL(destroyed(QObject*)), this, SLOT(accountDestroyed(QObject*)));
		connect(account,    SIGNAL(destroyed(QObject*)), this, SLOT(accountDestroyed(QObject*)));

		disconnect(account->avatarFactory(), SIGNAL(avatarChanged(const Jid&)), this, SLOT(avatarChanged(const Jid&)));
		connect(account->avatarFactory(),    SIGNAL(avatarChanged(const Jid&)), this, SLOT(avatarChanged(const Jid&)));

		// we should queue this connection, because when the signal is emitted,
		// the account is still somewhat active, and we'd better disable when it's
		// completely inactive
		disconnect(account, SIGNAL(connectionError(const QString&)), this, SLOT(connectionError(const QString&)));
		connect(account,    SIGNAL(connectionError(const QString&)), this, SLOT(connectionError(const QString&)), Qt::QueuedConnection);

		if (!accounts_.contains(account))
			addAccount(account);
	}
}

void AccountInformerModel::connectionError(const QString& errorInfo)
{
	Q_UNUSED(errorInfo);
	PsiAccount* account = static_cast<PsiAccount*>(sender());
	if (!account) {
		return;
	}

	// if (!account->currentConnectionError().isEmpty() &&
	//     (account->currentConnectionErrorCondition() == XMPP::ClientStream::NotAuthorized ||
	//      account->currentConnectionErrorCondition() == XMPP::ClientStream::GenericAuthError))
	// {
	// 	account->setEnabled(false);
	// }

	updateAccount(account);
}

void AccountInformerModel::updatedActivity()
{
	updateAccount(static_cast<PsiAccount*>(sender()));
}

void AccountInformerModel::updatedAccount()
{
	updateAccount(static_cast<PsiAccount*>(sender()));
}

void AccountInformerModel::avatarChanged(const XMPP::Jid& jid)
{
	QMap<PsiAccount*, QStandardItem*>::ConstIterator it = accounts_.constBegin();
	for ( ; it != accounts_.constEnd(); ++it) {
		PsiAccount* account = it.key();
		if (account->jid().compare(jid, false))
			updateAccount(account);
	}
}

void AccountInformerModel::accountDestroyed(QObject* obj)
{
	PsiAccount* account = static_cast<PsiAccount*>(obj);
	Q_ASSERT(accounts_.contains(account));
	QModelIndex index = accounts_[account]->index();
	accounts_.remove(account);
	removeRow(index.row(), index.parent());
}

void AccountInformerModel::addAccount(PsiAccount* account)
{
	Q_ASSERT(!accounts_.contains(account));

	QStandardItem* item = new QStandardItem();
	int rowNum = invisibleRootItem()->rowCount();
	invisibleRootItem()->insertRow(rowNum, item);

	accounts_[account] = item;
	updateAccount(account);
}

XMPP::Status::Type AccountInformerModel::accountStatus(PsiAccount* account) const
{
	XMPP::Status::Type status = account->status().type();
	if (!account->isAvailable())
		status = XMPP::Status::Offline;
	if (account->isActive() && !account->isAvailable())
		status = XMPP::Status::Reconnecting;
	return status;
}

void AccountInformerModel::updateAccount(PsiAccount* account)
{
	Q_ASSERT(accounts_.contains(account));
	QStandardItem* item = accounts_[account];
	item->setText(account->jid().userHost());
	item->setData(QVariant(account->enabled()), EnabledRole);
	item->setData(QVariant(accountStatus(account)), StatusTypeRole);
	item->setData(QVariant(QString(account->userAccount().pass)), PasswordRole);
	item->setData(QVariant(QIcon(account->avatarFactory()->getAvatar(account->jid()))), PictureRole);
	item->setData(QVariant(account->currentConnectionError()), CurrentConnectionErrorRole);
	// item->setData(account->currentConnectionError().isEmpty() ? QVariant() : QVariant(AccountInformerDelegate::errorColor()), Qt::BackgroundRole);

	if (!account->currentConnectionError().isEmpty() &&
	     account->status().type() == XMPP::Status::Offline)
	{
		emit accountError(indexFromItem(item));
		emit activateToShowError();
	}
}

PsiAccount* AccountInformerModel::accountForItem(QStandardItem* item) const
{
	QList<PsiAccount*> keys = accounts_.keys(item);
	if (!keys.isEmpty())
		return keys.first();
	return 0;
}

PsiAccount* AccountInformerModel::accountForIndex(const QModelIndex& index) const
{
	return accountForItem(itemFromIndex(index));
}
