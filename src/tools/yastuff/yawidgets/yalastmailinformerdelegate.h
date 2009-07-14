/*
 * yalastmailinformerdelegate.h
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

#ifndef YALASTMAILINFORMERDELEGATE_H
#define YALASTMAILINFORMERDELEGATE_H

#include "yacontactlistviewdelegate.h"

#include <QStandardItemModel>
#include <QItemSelection>

class YaLastMailInformerDelegate : public YaContactListViewDelegate
{
	Q_OBJECT
public:
	YaLastMailInformerDelegate();
	~YaLastMailInformerDelegate();

	// reimplemented
	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	// reimplemented
	virtual void doAvatar(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QRect avatarRect(const QRect& visualRect) const;

private:
	mutable QStandardItemModel* contactModel_;
	mutable QStandardItem* contactItem_;
	mutable QModelIndex contactItemIndex_;

	const QModelIndex& indexFor(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif
