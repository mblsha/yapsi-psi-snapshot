/*
 * yacontactlistviewlargedelegate.cpp
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

#include "yacontactlistviewlargedelegate.h"

#include <QPainter>
#include <QTextLayout>
#include <QTextOption>

YaContactListViewLargeDelegate::YaContactListViewLargeDelegate(YaContactListView* parent)
	: YaContactListViewDelegate(parent)
{}

void YaContactListViewLargeDelegate::realDrawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawEditorBackground(painter, option, index);
	doAvatar(painter, option, index);

	QRect textRect(avatarRect(option.rect).right() + horizontalMargin() + 1, option.rect.top() + verticalMargin(), 0, 0);
	textRect.setWidth(option.rect.right()   - textRect.left() - horizontalMargin());
	textRect.setHeight(option.rect.bottom() - textRect.top()  - verticalMargin());
	// painter->drawRect(textRect);

	bool selected = option.state & QStyle::State_Selected;

	// if (!selected) {
	// 	painter->save();
	// 	painter->setPen(QColor(0xdd, 0xdd, 0xdd));
	// 	painter->drawLine(textRect.topLeft(), textRect.topRight());
	// 	painter->restore();
	// }

	textRect.adjust(0, 4, 0, -4);

	QRect nameRect(textRect);
	nameRect.setHeight(textRect.height() / 2);

	if (statusText(index).isEmpty()) {
		nameRect.moveTop(textRect.top() + (textRect.height() - nameRect.height()) / 2);
	}

	QStyleOptionViewItemV2 n_o = nameStyle(selected, statusType(index), Ya::VisualUtil::RosterStyleLarge, hovered());

	if (drawStatusIcon(statusType(index))) {
		QPixmap statusPixmap = Ya::VisualUtil::rosterStatusPixmap(statusType(index));
		QRect statusPixmapRect(nameRect);
		statusPixmapRect.setSize(statusPixmap.size());
		statusPixmapRect.moveTop(nameRect.top() + n_o.fontMetrics.ascent() - statusPixmapRect.height());
		nameRect.translate(statusPixmap.width() + 3, 0);
		painter->drawPixmap(statusPixmapRect, statusPixmap);
	}

	nameRect.setRight(option.rect.right() - horizontalMargin() - 1);
	if (drawStatusTypeText(index))
		drawStatusTypeText(painter, n_o, &nameRect, index);
	drawName(painter, n_o, nameRect, nameText(option, index), index);

	// painter->drawRect(nameRect);
	// painter->drawRect(statusTypeRect);

	// begin - status message
	QRect statusRect(nameRect);
	statusRect.moveTo(statusRect.left(), textRect.bottom() - statusRect.height());
	statusRect.setRight(textRect.right());
	statusRect.adjust(0, (int)(((float)statusRect.height()) * 0.2), 0, 2);
	// painter->drawRect(statusRect);

	if (!statusText(index).isEmpty()) {
		QStyleOptionViewItemV2 sm_o = statusTextStyle(Ya::VisualUtil::RosterStyleLarge, hovered());
		sm_o.palette.setColor(QPalette::Highlight, option.palette.color(QPalette::Highlight));
		drawStatus(painter, sm_o, statusRect, statusText(index), index);
	}
}

int YaContactListViewLargeDelegate::avatarSize() const
{
	return 46;
}

QRect YaContactListViewLargeDelegate::rosterToolTipArea(const QRect& rect) const
{
	return rect.adjusted(0, 3, 0, 0);
}

QRect YaContactListViewLargeDelegate::editorRect(const QRect& nameRect) const
{
	return nameRect;
}
