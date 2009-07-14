/*
 * yacontactlistview.h - custom contact list widget
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

#ifndef YACONTACTLISTVIEW_H
#define YACONTACTLISTVIEW_H

#include "contactlistdragview.h"

class PsiContact;
class YaContactListViewDelegate;
class QPainter;

class YaContactListView : public ContactListDragView
{
	Q_OBJECT

public:
	YaContactListView(QWidget* parent);
	~YaContactListView();

	// reimplemented
	void setModel(QAbstractItemModel* model);

	enum AvatarMode {
		AvatarMode_Disable = 0,
		AvatarMode_Auto = 1,
		AvatarMode_Big = 2,
		AvatarMode_Small = 3
	};

	AvatarMode avatarMode() const;
	void setAvatarMode(AvatarMode avatarMode);

	// reimplemented
	virtual bool drawSelectionBackground() const;

signals:
	void addSelection(QMimeData* selection);
	void addGroup();

public slots:
	// called by YaToolBoxPage
	void resized();

protected:
	// reimplemented
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);
	void dropEvent(QDropEvent*);
	void dragEnterEvent(QDragEnterEvent*);
	void dragLeaveEvent(QDragLeaveEvent*);
	void startDrag(Qt::DropActions supportedActions);
	void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);

	YaContactListViewDelegate* yaContactListViewDelegate() const;

protected slots:
	// reimplemented
	virtual void itemExpanded(const QModelIndex&);
	virtual void itemCollapsed(const QModelIndex&);

protected:
	// reimplemented
	virtual void showToolTip(const QModelIndex& index, const QPoint& globalPos) const;
	virtual ContactListItemMenu* createContextMenuFor(ContactListItem* item) const;

	// reimplemented
	virtual bool updateCursor(const QModelIndex& index, UpdateCursorOrigin origin, bool force);
	virtual void doItemsLayoutStart();
	virtual void doItemsLayoutFinish();

private slots:
	void startInvalidateDelegate();
	void invalidateDelegate();
	void addSelection();

private:
	QTimer* invalidateDelegateTimer_;
	QAbstractItemDelegate* slimDelegate_;
	QAbstractItemDelegate* normalDelegate_;
	QAbstractItemDelegate* largeDelegate_;
	AvatarMode avatarMode_;
};

#endif
