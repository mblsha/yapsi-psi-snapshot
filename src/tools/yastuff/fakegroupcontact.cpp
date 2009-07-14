/*
 * fakegroupcontact.cpp - fake contact used to make sure certain contact list groups stay alive
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

#include "fakegroupcontact.h"

#include "yacommon.h"

FakeGroupContact::FakeGroupContact(QObject* parent, QStringList existingGroups)
	: PsiContact()
{
	setParent(parent);
	groups_ << Ya::createUniqueName(defaultGroupName(), existingGroups);
}

FakeGroupContact::FakeGroupContact(QObject* parent, QString groupName)
	: PsiContact()
{
	setParent(parent);
	groups_ << groupName;
}

FakeGroupContact::~FakeGroupContact()
{
}

QString FakeGroupContact::defaultGroupName()
{
	return tr("New group");
}

bool FakeGroupContact::isFake() const
{
	return true;
}

bool FakeGroupContact::isEditable() const
{
	return true;
}

bool FakeGroupContact::isRemovable() const
{
	return true;
}

bool FakeGroupContact::inList() const
{
	return false;
}

bool FakeGroupContact::isOnline() const
{
	return true;
}

bool FakeGroupContact::isHidden() const
{
	return true;
}

QStringList FakeGroupContact::groups() const
{
	return groups_;
}

void FakeGroupContact::setGroups(QStringList groups)
{
	groups_ = groups;
	emit groupsChanged();
}

void FakeGroupContact::remove()
{
	deleteLater();
}
