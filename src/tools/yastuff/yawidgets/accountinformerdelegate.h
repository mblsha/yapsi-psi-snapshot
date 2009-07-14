/*
 * accountinformerdelegate.h - draws an editable account list
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

#ifndef ACCOUNTINFORMERDELEGATE_H
#define ACCOUNTINFORMERDELEGATE_H

#include "yacontactlistviewdelegate.h"

#include <QStandardItemModel>
#include <QItemSelection>

class AccountInformerModel;
class QCheckBox;

class AccountInformerDelegate : public YaContactListViewDelegate
{
	Q_OBJECT
public:
	AccountInformerDelegate(QAbstractItemView* itemView);
	~AccountInformerDelegate();

	void setSelection(const QItemSelection& selection);

	bool isHoveringChangePassword(const QStyleOptionViewItem& option, const QModelIndex& index, const QPoint& pos);
	bool isHoveringEnabledCheckbox(const QStyleOptionViewItem& option, const QModelIndex& index, const QPoint& pos);

	QString tooltip(const QModelIndex& index) const;

	// reimplemented
	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	static QFont defaultFont();
	static QColor errorColor();

private:
	QAbstractItemView* itemView_;
	QCheckBox* checkBox_;
	YaContactListViewDelegate* normalDelegate_;
	YaContactListViewDelegate* selectionDelegate_;
	mutable QStandardItemModel* contactModel_;
	mutable QStandardItem* contactItem_;
	mutable QModelIndex contactItemIndex_;
	QItemSelection selection_;

	// reimplemented
	const AccountInformerModel* model(const QModelIndex& index) const;

	YaContactListViewDelegate* delegateFor(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	const QModelIndex& indexFor(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	QRect changePasswordRect(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	QStyleOptionButton checkBoxOption(const QModelIndex& index) const;
	QRect checkBoxRect(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	QString editPasswordText() const;
	QString checkBoxText(bool enabled) const;

	bool passwordEmpty(const QModelIndex& index) const;
};

#endif
