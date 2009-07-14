/*
 * yalastmailinformerdelegate.cpp
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

#include "yalastmailinformerdelegate.h"

#include "yalastmailinformermodel.h"

YaLastMailInformerDelegate::YaLastMailInformerDelegate()
	: YaContactListViewDelegate(0)
{
	contactModel_ = new QStandardItemModel(this);
	contactItem_ = new QStandardItem();
	contactModel_->invisibleRootItem()->appendRow(contactItem_);
}

YaLastMailInformerDelegate::~YaLastMailInformerDelegate()
{
}

void YaLastMailInformerDelegate::doAvatar(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(index);
}

QRect YaLastMailInformerDelegate::avatarRect(const QRect& visualRect) const
{
	QRect avatarRect(visualRect);
	avatarRect.setWidth(0);
	return avatarRect;
}

void YaLastMailInformerDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	YaContactListViewDelegate::paint(painter, option, indexFor(option, index));
}

QSize YaLastMailInformerDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return YaContactListViewDelegate::sizeHint(option, indexFor(option, index));
}

const QModelIndex& YaLastMailInformerDelegate::indexFor(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	contactItem_->setText(index.data(YaLastMailInformerModel::FromRole).toString());
	contactItem_->setData(QVariant(index.data(YaLastMailInformerModel::SubjectRole).toString()), ContactListModel::StatusTextRole);
	contactItem_->setData(QVariant(ContactListModel::ContactType), ContactListModel::TypeRole);
	contactItem_->setData(QVariant(XMPP::Status::Online), ContactListModel::StatusTypeRole);
	contactItem_->setData(QVariant(QIcon()), ContactListModel::PictureRole);
	contactItemIndex_ = contactItem_->index();
	return contactItemIndex_;
}
