/*
 * yastatusbarlayout.cpp
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

// based on qt/examples/layouts/flowlayout

#include "yastatusbarlayout.h"

#include <QtGui>

#include "yainformerbutton.h"

YaStatusBarLayout::YaStatusBarLayout(QWidget* parent, int margin, int spacing)
	: QLayout(parent)
{
	setMargin(margin);
	setSpacing(spacing);
}

YaStatusBarLayout::YaStatusBarLayout(int spacing)
{
	setSpacing(spacing);
}

YaStatusBarLayout::~YaStatusBarLayout()
{
	QLayoutItem* item;
	while ((item = takeAt(0)))
		delete item;
}

void YaStatusBarLayout::addItem(QLayoutItem* item)
{
	itemList.append(item);
	YaInformerButton* informerButton = dynamic_cast<YaInformerButton*>(item->widget());
	if ((informerButton && informerButton->isPersistent()) ||
	    dynamic_cast<QSizeGrip*>(item->widget()))
		persistentItems.append(item);
}

int YaStatusBarLayout::count() const
{
	return itemList.size();
}

QLayoutItem* YaStatusBarLayout::itemAt(int index) const
{
	return itemList.value(index);
}

QLayoutItem* YaStatusBarLayout::takeAt(int index)
{
	if (index >= 0 && index < itemList.size()) {
		QLayoutItem* item = itemList.takeAt(index);
		persistentItems.removeAll(item);
		return item;
	}
	else
		return 0;
}

Qt::Orientations YaStatusBarLayout::expandingDirections() const
{
	return 0;
}

bool YaStatusBarLayout::hasHeightForWidth() const
{
	return true;
}

int YaStatusBarLayout::heightForWidth(int width) const
{
	int height = doLayout(QRect(0, 0, width, 0), true);
	return height;
}

void YaStatusBarLayout::setGeometry(const QRect& rect)
{
	QLayout::setGeometry(rect);
	doLayout(rect, false);
}

QSize YaStatusBarLayout::sizeHint() const
{
	return minimumSize();
}

QSize YaStatusBarLayout::minimumSize() const
{
	QSize size;
	QLayoutItem* item;
	foreach(item, itemList)
		size = size.expandedTo(item->minimumSize());

	size += QSize(2 * margin(), 2 * margin());
	return size;
}

int YaStatusBarLayout::doLayout(const QRect& fullRect, bool testOnly) const
{
	QRect rect = fullRect;
	QLayoutItem* item;

	int x = rect.x();
	int y = rect.y();
	int lineHeight = 0;

	foreach(item, persistentItems) {
		rect.setRight(rect.right() - item->sizeHint().width());
		if (!testOnly)
			item->setGeometry(QRect(QPoint(rect.right(), rect.bottom() - item->sizeHint().height()), item->sizeHint()));
		rect.setRight(rect.right() - spacing());
	}

	foreach(item, itemList) {
		lineHeight = qMax(lineHeight, item->sizeHint().height());

		if (persistentItems.contains(item))
			continue;

		int nextX = x + item->sizeHint().width() + spacing();
		if (nextX - spacing() > rect.right() && lineHeight > 0) {
			x = rect.x();
			y = y + lineHeight + spacing();
			nextX = x + item->sizeHint().width() + spacing();
			lineHeight = 0;
		}

		if (!testOnly)
			item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

		x = nextX;
	}
	return y + lineHeight - rect.y();
}
