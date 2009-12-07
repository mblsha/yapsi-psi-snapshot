/*
 * psicontactlistviewdelegate.h
 * Copyright (C) 2009  Yandex LLC (Michail Pishchagin)
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

#ifndef PSICONTACTLISTVIEWDELEGATE_H
#define PSICONTACTLISTVIEWDELEGATE_H

#include "contactlistviewdelegate.h"

class PsiContactListViewDelegate : public ContactListViewDelegate
{
	Q_OBJECT
public:
	PsiContactListViewDelegate(ContactListView* parent);
	~PsiContactListViewDelegate();

	// reimplemented
	virtual int avatarSize() const;

protected:
	// reimplemented
	virtual void drawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void drawGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void drawAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	// reimplemented
	virtual void drawText(QPainter* painter, const QStyleOptionViewItem& o, const QRect& rect, const QString& text, const QModelIndex& index) const;

	// reimplemented
	virtual QRect nameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QRect groupNameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QRect editorRect(const QRect& nameRect) const;

	virtual QPixmap statusPixmap(const QModelIndex& index) const;

private slots:
	void optionChanged(const QString& option);

private:
	QFont* font_;
	QFontMetrics* fontMetrics_;
	bool showStatusMessages_;
};

#endif
