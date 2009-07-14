/*
 * yaselflabel.cpp
 * Copyright (C) 2008  Yandex LLC (Alexei Matiouchkine)
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

#include <QTimer>

#include "yaselflabel.h"

#include "psiaccount.h"
#include "psicontactlist.h"
#include "psitooltip.h"
#include "textutil.h"

#include "yacommon.h"

static const QList<PsiAccount*> EMPTY_LIST;

YaSelfLabel::YaSelfLabel(QWidget* parent)
	: YaLabel(parent)
	, contactList_(0)
{
	setOpenExternalLinks(false);
	// connect(this, SIGNAL(labelChanged(const QString &)), SLOT(nameChanged(const QString &)));
}

QString YaSelfLabel::text() const
{
	PsiAccount* a;
	if (!contactList_ || !(a = contactList_->defaultAccount())) {
		if (contactList_ && contactList_->enabledAccounts().isEmpty())
			return tr("Ya.Online");
		return Ya::ellipsis();
	}

	disconnect(a, SIGNAL(nickChanged()), this, SLOT(update()));
	connect(a,    SIGNAL(nickChanged()), this, SLOT(update()));

	return a->nick();
}

void YaSelfLabel::setContactList(const PsiContactList* contactList)
{
	if (contactList_ != contactList) {
		contactList_ = (PsiContactList*)contactList;
		connect(contactList_, SIGNAL(accountActivityChanged()), SLOT(update()));
		connect(contactList_, SIGNAL(accountCountChanged()), SLOT(update()));
		update();
	}
}

const PsiContactList* YaSelfLabel::contactList() const
{
	return contactList_;
}

const QList<PsiAccount*>& YaSelfLabel::accounts() const
{
	return contactList_ ? contactList_->enabledAccounts() : EMPTY_LIST;
}

// void YaSelfLabel::nameChanged(const QString& s)
// {
// 	if (contactList_) {
// 		foreach(const PsiAccount* a, accounts()) {
// 			VCard* vcard = (VCard*) VCardFactory::instance()->vcard(a->jid());
// 			vcard->setNickName(s);
// 			const VCard v = *vcard;
// 			VCardFactory::instance()->setVCard(a, v);
// 		}
// 	}
// }
