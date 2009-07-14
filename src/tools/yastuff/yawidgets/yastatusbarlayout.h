/*
 * yastatusbarlayout.h
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

#ifndef YASTATUSBARLAYOUT_H
#define YASTATUSBARLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QWidgetItem>

class YaStatusBarLayout : public QLayout
{
public:
	YaStatusBarLayout(QWidget* parent, int margin = 0, int spacing = -1);
	YaStatusBarLayout(int spacing = -1);
	~YaStatusBarLayout();

	void addItem(QLayoutItem* item);
	Qt::Orientations expandingDirections() const;
	bool hasHeightForWidth() const;
	int heightForWidth(int) const;
	int count() const;
	QLayoutItem* itemAt(int index) const;
	QSize minimumSize() const;
	void setGeometry(const QRect& rect);
	QSize sizeHint() const;
	QLayoutItem* takeAt(int index);

private:
	int doLayout(const QRect& rect, bool testOnly) const;

	QList<QLayoutItem*> itemList;
	QList<QLayoutItem*> persistentItems;
};


#endif
