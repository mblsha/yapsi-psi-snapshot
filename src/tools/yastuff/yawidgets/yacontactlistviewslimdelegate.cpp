/*
 * yacontactlistviewslimdelegate.cpp
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

#include "yacontactlistviewslimdelegate.h"

#include <QPainter>
#include <QTextLayout>
#include <QTextOption>

#include "xmpp_status.h"

YaContactListViewSlimDelegate::YaContactListViewSlimDelegate(YaContactListView* parent)
	: YaContactListViewDelegate(parent)
{
	setAvatarSize(22);
}

void YaContactListViewSlimDelegate::realDrawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawEditorBackground(painter, option, index);
	bool selected = option.state & QStyle::State_Selected;

	QPixmap statusPixmap = Ya::VisualUtil::rosterStatusPixmap(statusType(index));
	QRect typeRect = this->typeRect(option, index);
	if (typeRect.width()) {
		painter->drawPixmap(typeRect.topLeft(), statusPixmap);
	}
	// painter->drawRect(typeRect);

	QStyleOptionViewItemV2 n_o = nameStyle(selected, statusType(index), Ya::VisualUtil::RosterStyleSlim, hovered());
	QString name = nameText(option, index);

	QRect nameRect = this->nameRect(option, index);
	// painter->drawRect(nameRect);
	drawName(painter, n_o, nameRect, name, index);

	QRect statusRect(nameRect);
	statusRect.setLeft(nameRect.left() + n_o.fontMetrics.width(name) + 3);

	if (!statusText(index).isEmpty() && statusRect.width() > 10) {
		QStyleOptionViewItemV2 sm_o = statusTextStyle(Ya::VisualUtil::RosterStyleSlim, hovered());
		statusRect.setTop(nameRect.top() + n_o.fontMetrics.ascent() - sm_o.fontMetrics.ascent());
		sm_o.palette.setColor(QPalette::Highlight, option.palette.color(QPalette::Highlight));
		drawStatus(painter, sm_o, statusRect, statusText(index), index);
	}
	// painter->drawRect(statusRect);
}

QRect YaContactListViewSlimDelegate::typeRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QPixmap statusPixmap = Ya::VisualUtil::rosterStatusPixmap(statusType(index));
	QRect typeRect(option.rect);
	typeRect.setSize(statusPixmap.size());
	typeRect.moveTo(option.rect.left() + horizontalMargin(),
	                typeRect.top() + (option.rect.height() - typeRect.height()) / 2);

	if (statusType(index) == XMPP::Status::Online && !drawStatusIcon()) {
		typeRect.setWidth(0);
		typeRect.moveLeft(typeRect.left() - 2);
	}

	return typeRect;
}

QRect YaContactListViewSlimDelegate::nameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect typeRect = this->typeRect(option, index);
	QStyleOptionViewItemV2 n_o = nameStyle(true, statusType(index), Ya::VisualUtil::RosterStyleSlim, hovered());

	QRect nameRect(option.rect);
	nameRect.setHeight(n_o.fontMetrics.height());
	nameRect.setLeft(typeRect.right() + 4);
	nameRect.moveTop(nameRect.top() + (option.rect.height() - nameRect.height()) / 2);
	nameRect.setRight(option.rect.right() - horizontalMargin());
	return nameRect;
}

QRect YaContactListViewSlimDelegate::editorRect(const QRect& nameRect) const
{
	return nameRect.adjusted(0, -3, 0, 3);
}

void YaContactListViewSlimDelegate::drawStatus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& statusRect, const QString& status, const QModelIndex& index) const
{
	if (contactList() && contactList()->indexWidget(index)) {
		return;
	}

	YaContactListViewDelegate::drawStatus(painter, option, statusRect, status, index);
}
