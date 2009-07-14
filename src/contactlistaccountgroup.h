/*
 * contactlistaccountgroup.h
 * Copyright (C) 2009  Michail Pishchagin
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

#ifndef CONTACTLISTACCOUNTGROUP_H
#define CONTACTLISTACCOUNTGROUP_H

#include "contactlistnestedgroup.h"

class PsiAccount;

class ContactListAccountGroup : public ContactListNestedGroup
{
	Q_OBJECT
public:
	ContactListAccountGroup(ContactListModel* model, ContactListGroup* parent, PsiAccount* account);
	~ContactListAccountGroup();

	PsiAccount* account() const;
	ContactListAccountGroup* findAccount(PsiAccount* account) const;

	// reimplemented
	virtual ContactListModel::Type type() const;
	virtual const QString& displayName() const;
	virtual QString internalGroupName() const;
	virtual ContactListItemMenu* contextMenu(ContactListModel* model);

	// reimplemented
	virtual void addContact(PsiContact* contact, QStringList contactGroups);
	virtual void contactUpdated(PsiContact* contact);
	virtual void contactGroupsChanged(PsiContact* contact, QStringList contactGroups);

protected:
	void removeAccount(ContactListAccountGroup* accountGroup);
	ContactListItemProxy* findGroup(ContactListGroup* group) const;
	bool isRoot() const;

	// reimplemented
	virtual void clearGroup();

private slots:
	void accountUpdated();

private:
	PsiAccount* account_;
	QList<ContactListAccountGroup*> accounts_;
};

#endif
