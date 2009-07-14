/*
 * yacontactlistviewlargedelegate.h
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

#ifndef YACONTACTLISTVIEWLARGEDELEGATE_H
#define YACONTACTLISTVIEWLARGEDELEGATE_H

#include "yacontactlistviewdelegate.h"

class YaContactListViewLargeDelegate : public YaContactListViewDelegate
{
	Q_OBJECT
public:
	YaContactListViewLargeDelegate(YaContactListView* parent);

protected:
	// reimplemented
	virtual void realDrawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual int avatarSize() const;
	virtual QRect rosterToolTipArea(const QRect& rect) const;
	virtual QRect editorRect(const QRect& nameRect) const;
};

#endif
