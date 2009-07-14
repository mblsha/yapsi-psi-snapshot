/*
 * accountinformermodel.h - editable account list model
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

#ifndef ACCOUNTINFORMERMODEL_H
#define ACCOUNTINFORMERMODEL_H

#include <QStandardItemModel>
#include <QPointer>

#include "xmpp_status.h"

class PsiAccount;
class PsiContactList;

using namespace XMPP;

class AccountInformerModel : public QStandardItemModel
{
	Q_OBJECT
public:
	AccountInformerModel(QObject* parent);
	~AccountInformerModel();

	void setContactList(PsiContactList* contactList);

	PsiAccount* accountForIndex(const QModelIndex& index) const;
	PsiAccount* accountForItem(QStandardItem* item) const;

	XMPP::Status::Type accountStatus(PsiAccount* account) const;
	void setAccountEnabled(PsiAccount* account, bool enabled) const;

	enum {
		EnabledRole = Qt::UserRole + 0,
		StatusTypeRole = Qt::UserRole + 1, // XMPP::Status::Type
		PictureRole    = Qt::UserRole + 2, // QIcon, avatar
		PasswordRole   = Qt::UserRole + 3, // QString
		CurrentConnectionErrorRole = Qt::UserRole + 4, // QString

		ChangeEnabledRole = Qt::UserRole + 5
	};

	enum EditMode {
		EditJID = 0,
		EditPassword,
		EditDisabled
	};

	EditMode editMode(const QStandardItem* item) const;
	EditMode editMode(const QModelIndex& index) const;
	bool indexEditable(const QModelIndex& index) const;

	// reimplemented
	QVariant data(const QModelIndex& index, int role) const;
	bool setData(const QModelIndex& index, const QVariant& data, int role);

signals:
	void editPassword();
	void accountError(const QModelIndex& index);
	void activateToShowError();

private slots:
	void accountCountChanged();
	void updatedActivity();
	void updatedAccount();
	void accountDestroyed(QObject* obj);
	void avatarChanged(const Jid&);
	void connectionError(const QString& errorInfo);

private:
	QPointer<PsiContactList> contactList_;
	QMap<PsiAccount*, QStandardItem*> accounts_;

	void addAccount(PsiAccount* account);
	void updateAccount(PsiAccount* account);
};

#endif
