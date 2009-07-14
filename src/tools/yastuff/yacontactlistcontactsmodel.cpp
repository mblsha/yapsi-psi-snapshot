/*
 * yacontactlistcontactsmodel.cpp
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

#include "yacontactlistcontactsmodel.h"

#include <QTimer>

#include "psiaccount.h"
#include "psicontactlist.h"
#include "yacommon.h"
#include "fakegroupcontact.h"
#include "contactlistgroup.h"
#include "contactlistmodelupdater.h"
#include "contactlistgroupcache.h"

YaContactListContactsModel::YaContactListContactsModel(PsiContactList* contactList)
	: YaContactListModel(contactList)
{
}

ContactListModel* YaContactListContactsModel::clone() const
{
	return new YaContactListContactsModel(contactList());
}

PsiAccount* YaContactListContactsModel::getDropAccount(PsiAccount* account, const QModelIndex& parent) const
{
	if (parent.isValid())
		return YaContactListModel::getDropAccount(account, parent);

	return account;
}

void YaContactListContactsModel::initializeModel()
{
	YaContactListModel::initializeModel();
	protectVirtualUnremovableGroups();
	QTimer::singleShot(0, this, SLOT(protectVirtualUnremovableGroups()));
}

void YaContactListContactsModel::protectVirtualUnremovableGroups()
{
	protectVirtualUnremovableGroups(virtualUnremovableGroups());
}

void YaContactListContactsModel::protectVirtualUnremovableGroups(QStringList endangeredGroups)
{
	if (!contactList() || !contactList()->accountsLoaded())
		return;

	foreach(QString group, virtualUnremovableGroups()) {
		if (endangeredGroups.contains(group)) {
			bool createNew = true;
			foreach(PsiContact* contact, fakeGroupContacts_) {
				if (contact->groups().contains(group)) {
					createNew = false;
					break;
				}
			}

			if (!createNew)
				continue;

			FakeGroupContact* contact = addFakeGroupContact(QStringList(), group);
		}
	}
}

QStringList YaContactListContactsModel::virtualGroupList() const
{
	QStringList result;
	foreach(PsiContact* contact, fakeGroupContacts_) {
		foreach(const QString& name, contact->groups()) {
			if (!result.contains(name)) {
				result << name;
			}
		}
	}
	return result;
}

QStringList YaContactListContactsModel::virtualUnremovableGroups()
{
	QStringList result;
	result += PsiContact::generalGroupName();
	return result;
}

void YaContactListContactsModel::contactOperationsPerformed(const ContactListModelOperationList& operations, OperationType operationType, const QHash<ContactListGroup*, int>& groupContactCount)
{
	Q_UNUSED(operations);
	QHashIterator<ContactListGroup*, int> it(groupContactCount);
	while (it.hasNext()) {
		it.next();

		if (!it.value() && operationType == Operation_DragNDrop) {
			// qWarning("group = %s; %d", qPrintable(it.key()->fullName()), it.value());
			FakeGroupContact* contact = addFakeGroupContact(QStringList(), it.key()->fullName());
		}
	}
}

FakeGroupContact* YaContactListContactsModel::addFakeGroupContact(QStringList groups, QString specificGroupName)
{
	Q_ASSERT(contactList() && contactList()->accountsLoaded());
	if (!contactList() || !contactList()->accountsLoaded())
		return 0;

	FakeGroupContact* contact = 0;
	if (specificGroupName.isEmpty())
		contact = new FakeGroupContact(this, groups);
	else
		contact = new FakeGroupContact(this, specificGroupName);

	connect(contact, SIGNAL(destroyed(PsiContact*)), SLOT(fakeContactDestroyed(PsiContact*)));
	fakeGroupContacts_ << contact;

	updater()->addContact(contact);

	return contact;
}

QModelIndex YaContactListContactsModel::addGroup()
{
	if (groupsEnabled()) {
		FakeGroupContact* contact = addFakeGroupContact(groupCache()->groups(), QString());
		updater()->commit();

		QList<ContactListGroup*> groups = groupCache()->groupsFor(contact);
		if (!groups.isEmpty()) {
			ContactListGroup* group = groups.first();
			group->updateOnlineContactsFlag();
			QModelIndex result = groupToIndex(group);
			Q_ASSERT(result.isValid());
			return result;
		}
	}
	return QModelIndex();
}

QList<PsiContact*> YaContactListContactsModel::additionalContacts() const
{
	QList<PsiContact*> result = ContactListModel::additionalContacts();
	result += fakeGroupContacts_;
	return result;
}

void YaContactListContactsModel::fakeContactDestroyed(PsiContact* fakeContact)
{
	fakeGroupContacts_.removeAll(fakeContact);
}

bool YaContactListContactsModel::filterRemoveContact(PsiContact* psiContact, const QStringList& newGroups)
{
	FakeGroupContact* fakeContact = dynamic_cast<FakeGroupContact*>(psiContact);
	if (fakeContact) {
		foreach(QString group, psiContact->groups()) {
			if (virtualUnremovableGroups().contains(group)) {
				return true;
			}
		}

		delete fakeContact;
		return true;
	}

	return false;
}
