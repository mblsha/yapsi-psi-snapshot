/*
 * accountinformerview.h - editable account list widget
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

#ifndef ACCOUNTINFORMERVIEW_H
#define ACCOUNTINFORMERVIEW_H

#include "hoverabletreeview.h"

class AccountInformerDelegate;
class AccountInformerModel;
class QStyleOptionViewItem;

class AccountInformerView : public HoverableTreeView
{
	Q_OBJECT
public:
	AccountInformerView(QWidget* parent);

	// reimplemented
	void setModel(QAbstractItemModel* model);

	void editNewAccount();

	QModelIndex selectedIndex() const;
	QModelIndexList selectedIndexes() const;

signals:
	void selectionChanged(const QItemSelection& selected);
	void deleteAccount();

protected:
	// reimplemented
	void leaveEvent(QEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseDoubleClickEvent(QMouseEvent*);
	void keyPressEvent(QKeyEvent* event);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);
	void commitData(QWidget* editor);
	bool viewportEvent(QEvent* event);

private slots:
	void updateSelection(const QModelIndex& index);
	void updateSelection();
	void updateCursor();

	void itemActivated(const QModelIndex& index);
	void clicked(const QModelIndex& index);

	void editPassword();
	void accountError(const QModelIndex& index);

private:
	AccountInformerDelegate* delegate_;
	QPoint cursorPosition_;
	QPoint pressedPosition_;

	AccountInformerDelegate* itemDelegate() const;
	bool isHoveringEnabledCheckbox(const QModelIndex& index, const QPoint& cursorPos);
	bool isHoveringChangePassword(const QModelIndex& index, const QPoint& cursorPos);

	void changeEnabledRole(const QModelIndex& index);

	AccountInformerModel* model() const;
	QStyleOptionViewItem getStyleOptionViewItem(const QModelIndex& index) const;
};

#endif
