/*
 * yainformersmodel.cpp
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

#include "yainformersmodel.h"

#include <QtAlgorithms>

#include "psioptions.h"
#include "psiaccount.h"
#include "contactlistgroup.h"
#include "contactlistitemproxy.h"
#include "psicontactlist.h"
#include "psicontact.h"
#include "yacommon.h"
#include "common.h"
#include "xmpp_xmlcommon.h"
#include "userlist.h"

#include "yacommon.h"
extern const QString Ya::INFORMERS_GROUP_NAME;

YaInformersModel::YaInformersModel(PsiContactList* contactList)
	: ContactListModel(contactList)
{
}

QStringList YaInformersModel::filterContactGroups(QStringList groups) const
{
	QStringList result;
	foreach(QString group, groups)
		if (Ya::INFORMERS_GROUP_NAME == group)
			result << group; // TODO: make this somehow not ugly
	return result;
}
