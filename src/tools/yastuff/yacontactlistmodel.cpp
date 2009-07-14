/*
 * yacontactlistmodel.cpp - custom ContactListModel
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

#include "yacontactlistmodel.h"

#include <QtAlgorithms>
#include <QTimer>

#include "psioptions.h"
#include "psiaccount.h"
#include "contactlistgroup.h"
#include "contactlistitemproxy.h"
#include "psicontactlist.h"
#include "psicontact.h"
#include "yacommon.h"
#include "common.h"
#include "contactlistnestedgroup.h"
#include "contactlistmodelselection.h"
#include "contactlistgroupcache.h"
#include "contactlistmodelupdater.h"
#include "contactlistgroup.h"

//----------------------------------------------------------------------------
// YaContactListModel
//----------------------------------------------------------------------------

YaContactListModel::YaContactListModel(PsiContactList* contactList)
	: ContactListDragModel(contactList)
{
}

ContactListModel* YaContactListModel::clone() const
{
	return new YaContactListModel(contactList());
}

// This should remain as YaContactListModel
QStringList YaContactListModel::filterContactGroups(QStringList groups) const
{
	QStringList grs = ContactListDragModel::filterContactGroups(groups);

	foreach (QString grp, Ya::BOTS_GROUP_NAMES) {
		if (grs.contains(grp)) {
			grs.remove(grp);
		}
	}
	return grs;
}
