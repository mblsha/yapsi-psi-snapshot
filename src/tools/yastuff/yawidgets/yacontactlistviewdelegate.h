/*
 * yacontactlistviewdelegate.h
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

#ifndef YACONTACTLISTVIEWDELEGATE_H
#define YACONTACTLISTVIEWDELEGATE_H

#include "contactlistviewdelegate.h"
#include "yacontactlistview.h"

#include "xmpp_status.h"
#include "yavisualutil.h"

class QColor;
class YaEmptyTextLineEdit;

class YaContactListViewDelegate : public ContactListViewDelegate
{
	Q_OBJECT
public:
	YaContactListViewDelegate(YaContactListView* parent);

	// reimplemented
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

	// reimplemented
	virtual const YaContactListView* contactList() const;
	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual int avatarSize() const;

	virtual QRect rosterToolTipArea(const QRect& rect) const;

	bool drawStatusIcon(XMPP::Status::Type type) const;
	bool drawStatusIcon() const;
	void setDrawStatusIcon(bool);

	virtual bool drawStatusTypeText(const QModelIndex& index) const;

// protected:
	// reimplemented
	virtual void drawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void drawGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void drawAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	virtual void doAvatar(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QRect avatarRect(const QRect& visualRect) const;
	QRect textRect(const QRect& visualRect) const;

	virtual int margin() const;
	virtual void setMargin(int margin);

	virtual void setAvatarSize(int avatarSize);

// protected:
	// reimplemented
	virtual void drawText(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text, const QModelIndex& index) const;
	virtual void drawName(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text, const QModelIndex& index) const;
	virtual void drawStatusTypeText(QPainter* painter, const QStyleOptionViewItem& option, QRect* rect, const QModelIndex& index) const;
	virtual void drawStatus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& statusRect, const QString& status, const QModelIndex& index) const;
	virtual void drawBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	virtual int nameFontSize(const QRect& nameRect) const;
	virtual int statusTypeFontSize(const QRect& statusTypeRect) const;
	virtual int statusMessageFontSize(const QRect& statusRect) const;

	virtual void realDrawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	// reimplemented
	virtual QRect nameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QRect groupNameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QRect editorRect(const QRect& nameRect) const;
	virtual void setEditorCursorPosition(QWidget* editor, int cursorPosition) const;
	virtual QColor backgroundColor(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	virtual void drawEditorBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	virtual QRect groupButtonRect(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	// reimplemented
	virtual QString nameText(const QStyleOptionViewItem& o, const QModelIndex& index) const;

	QPalette blackText() const;
	QStyleOptionViewItemV2 nameStyle(bool selected, XMPP::Status::Type status, Ya::VisualUtil::RosterStyle rosterStyle, bool hovered) const;
	QStyleOptionViewItemV2 statusTextStyle(Ya::VisualUtil::RosterStyle rosterStyle, bool hovered) const;

	void initEditor(YaEmptyTextLineEdit* editor) const;

protected slots:
	void editorFocusOut();

private:
	bool drawStatusIcon_;
	int margin_;
	int avatarSize_;
	bool preserveJidHosts_;
};

#endif
