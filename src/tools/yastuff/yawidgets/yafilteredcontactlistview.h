/*
 * yafilteredcontactlistview.h - contact list filter
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

#ifndef YAFILTEREDCONTACTLISTVIEW_H
#define YAFILTEREDCONTACTLISTVIEW_H

#include "yacontactlistview.h"

class YaFilteredContactListView : public YaContactListView
{
	Q_OBJECT
public:
	YaFilteredContactListView(QWidget* parent);

	bool handleKeyPressEvent(QKeyEvent* e);

signals:
	void quitFilteringMode();

private:
	enum Direction {
		Forward,
		Backward
	};
	void moveSelection(uint delta, Direction direction);
	void selectIndex(int row);

	bool extendSelection() const;

protected:
	// reimplemented
	virtual void itemActivated(const QModelIndex& index);
	virtual ContactListItemMenu* createContextMenuFor(ContactListItem* item) const;
	virtual void paintNoContactsStub(QPainter* p);
};

#endif
