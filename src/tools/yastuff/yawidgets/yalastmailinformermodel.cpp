/*
 * yalastmailinformermodel.cpp
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

#include "yalastmailinformermodel.h"

#include "psicontactlist.h"
#include "psiaccount.h"
#include "xmpp_yalastmail.h"

YaLastMailInformerModel::YaLastMailInformerModel(QObject* parent)
	: QStandardItemModel(parent)
	, contactList_(0)
{
}

YaLastMailInformerModel::~YaLastMailInformerModel()
{
}

void YaLastMailInformerModel::setContactList(PsiContactList* contactList)
{
	contactList_ = contactList;
	Q_ASSERT(contactList_);
	connect(contactList_, SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
	accountCountChanged();
}

void YaLastMailInformerModel::accountCountChanged()
{
	Q_ASSERT(contactList_);
	foreach(PsiAccount* account, contactList_->accounts()) {
		disconnect(account, SIGNAL(lastMailNotify(const YaLastMail&)), this, SLOT(lastMailNotify(const YaLastMail&)));
		connect(account,    SIGNAL(lastMailNotify(const YaLastMail&)), this, SLOT(lastMailNotify(const YaLastMail&)));
	}
}

void YaLastMailInformerModel::lastMailNotify(const YaLastMail& lastMail)
{
	QStandardItem* item = new QStandardItem();
	item->setData(QVariant(lastMail.from), FromRole);
	item->setData(QVariant(lastMail.subject), SubjectRole);
	item->setData(QVariant(lastMail.timeStamp), DateRole);
	item->setData(QVariant(lastMail.mid), MessageID);

	int rowNum = invisibleRootItem()->rowCount();
	invisibleRootItem()->insertRow(rowNum, item);
}

QString YaLastMailInformerModel::getIncomingMailUrl() const
{
	return "http://mail.yandex.ru/?yasoft=online";
}

QString YaLastMailInformerModel::getMailUrlFor(const QModelIndex& index) const
{
	if (!index.isValid() || index.data(MessageID).toString().isEmpty())
		return getIncomingMailUrl();

	return QString("http://mail.yandex.ru/msg?yasoft=online&ids=%1").arg(index.data(MessageID).toString());
}
