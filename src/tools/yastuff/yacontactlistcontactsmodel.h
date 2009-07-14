/*
 * yacontactlistcontactsmodel.h
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

#ifndef YACONTACTLISTCONTACTSMODEL_H
#define YACONTACTLISTCONTACTSMODEL_H

#include "yacontactlistmodel.h"

#include <QList>

class ContactListItem;
class ContactListGroupItem;
class FakeGroupContact;

class YaContactListContactsModel : public YaContactListModel
{
	Q_OBJECT
public:
	YaContactListContactsModel(PsiContactList* contactList);

	// reimplemented
	virtual ContactListModel* clone() const;

	QModelIndex addGroup();

	QStringList virtualGroupList() const;
	static QStringList virtualUnremovableGroups();

private slots:
	void protectVirtualUnremovableGroups();
	void fakeContactDestroyed(PsiContact* fakeContact);

protected:
	// reimplemented
	virtual PsiAccount* getDropAccount(PsiAccount* account, const QModelIndex& parent) const;
	virtual void initializeModel();
	virtual void contactOperationsPerformed(const ContactListModelOperationList& operations, OperationType operationType, const QHash<ContactListGroup*, int>& groupContactCount);
	QList<PsiContact*> additionalContacts() const;
	virtual bool filterRemoveContact(PsiContact* psiContact, const QStringList& newGroups);

private:
	QList<PsiContact*> fakeGroupContacts_;

	FakeGroupContact* addFakeGroupContact(QStringList groups, QString specificGroupName);
	void protectVirtualUnremovableGroups(QStringList endangeredGroups);
};

#endif
