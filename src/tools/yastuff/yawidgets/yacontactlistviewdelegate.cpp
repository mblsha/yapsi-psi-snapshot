/*
 * yacontactlistviewdelegate.cpp
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

#include "yacontactlistviewdelegate.h"

#include <QPainter>
#include <QTextLayout>
#include <QTextOption>
#include <QApplication>

#include "contactlistmodel.h"
#include "yacontactlistview.h"
#include "yacommon.h"
#include "xmpp_status.h"
#include "pixmaputil.h"
#include "yacontactlistview.h"
#include "yarostertooltip.h"
#include "yaemptytextlineedit.h"

YaContactListViewDelegate::YaContactListViewDelegate(YaContactListView* parent)
	: ContactListViewDelegate(parent)
	, margin_(3)
	, avatarSize_(31)
{
	setDrawStatusIcon(false);
}

const YaContactListView* YaContactListViewDelegate::contactList() const
{
	return static_cast<const YaContactListView*>(ContactListViewDelegate::contactList());
}

QSize YaContactListViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	if (index.isValid()) {
		if (ContactListModel::indexType(index) == ContactListModel::ContactType)
			return QSize(16, avatarSize());
		return QSize(16, 16 + 3);
	}

	return ContactListViewDelegate::sizeHint(option, index);
}

QString YaContactListViewDelegate::nameText(const QStyleOptionViewItem& o, const QModelIndex& index) const
{
	QString name = index.data(Qt::DisplayRole).toString();
	return name;
}

void YaContactListViewDelegate::drawName(QPainter* painter, const QStyleOptionViewItem& o, const QRect& rect, const QString& name, const QModelIndex& index) const
{
	if (contactList() && contactList()->indexWidget(index)) {
		return;
	}

	drawText(painter, o, rect, name, index);
}

void YaContactListViewDelegate::drawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawBackground(painter, option, index);
	realDrawContact(painter, option, index);
}

int YaContactListViewDelegate::margin() const
{
	return margin_;
}

void YaContactListViewDelegate::setMargin(int margin)
{
	margin_ = margin;
}

int YaContactListViewDelegate::nameFontSize(const QRect& nameRect) const
{
	return nameRect.height();
}

int YaContactListViewDelegate::statusTypeFontSize(const QRect& statusTypeRect) const
{
	return statusTypeRect.height();
}

int YaContactListViewDelegate::statusMessageFontSize(const QRect& statusRect) const
{
	return statusRect.height() - (margin() * 2);
}

QRect YaContactListViewDelegate::avatarRect(const QRect& visualRect) const
{
	int avatarSize = this->avatarSize() - (verticalMargin() * 2) - 1;
	QRect result(QPoint(0, 0), QPoint(avatarSize, avatarSize));
	result.moveTo(visualRect.topLeft());
	result.translate(horizontalMargin(), verticalMargin());
	return result;
}

void YaContactListViewDelegate::doAvatar(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QVariant avatarData = index.data(ContactListModel::PictureRole);
	QIcon avatar;
	if (avatarData.isValid()) {
		if (avatarData.type() == QVariant::Icon) {
			avatar = qvariant_cast<QIcon>(avatarData);
		}
	}

	Ya::VisualUtil::drawAvatar(painter,
	                           avatarRect(option.rect),
	                           statusType(index),
	                           XMPP::VCard::Gender(index.data(ContactListModel::GenderRole).toInt()),
	                           avatar, false);
}

void YaContactListViewDelegate::drawStatus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& statusRect, const QString& status, const QModelIndex& index) const
{
	drawText(painter, option, statusRect, status, index);
}

QPalette YaContactListViewDelegate::blackText() const
{
	QPalette result = opt().palette;
	result.setColor(QPalette::Text, Qt::black);
	result.setColor(QPalette::HighlightedText, result.color(QPalette::Text));
	return result;
}

QStyleOptionViewItemV2 YaContactListViewDelegate::nameStyle(bool selected, XMPP::Status::Type status, Ya::VisualUtil::RosterStyle rosterStyle, bool hovered) const
{
	QStyleOptionViewItemV2 n_o = opt();
	n_o.font = Ya::VisualUtil::contactNameFont(rosterStyle, status);
	n_o.fontMetrics = QFontMetrics(n_o.font);
	QPalette namePalette = blackText();
	if (!selected)
		namePalette.setColor(QPalette::Text, Ya::VisualUtil::statusColor(status, hovered));
	n_o.palette = namePalette;
	return n_o;
}

QStyleOptionViewItemV2 YaContactListViewDelegate::statusTextStyle(Ya::VisualUtil::RosterStyle rosterStyle, bool hovered) const
{
	QStyleOptionViewItemV2 sm_o = opt();
	sm_o.font = Ya::VisualUtil::contactStatusFont(rosterStyle);
	sm_o.fontMetrics = QFontMetrics(sm_o.font);
	QPalette grayText;
	grayText.setColor(QPalette::Text, Ya::VisualUtil::contactStatusColor(hovered));
	grayText.setColor(QPalette::HighlightedText, grayText.color(QPalette::Text));
	sm_o.palette = grayText;
	return sm_o;
}

QRect YaContactListViewDelegate::textRect(const QRect& visualRect) const
{
	QRect result(avatarRect(visualRect).right() + horizontalMargin() + 1, visualRect.top() + verticalMargin(), 0, 0);
	result.setWidth(visualRect.right()   - result.left() - horizontalMargin());
	result.setHeight(visualRect.bottom() - result.top()  - verticalMargin() + 2);
	return result;
}

bool YaContactListViewDelegate::drawStatusTypeText(const QModelIndex& index) const
{
	return false;
	// return statusType(index) == XMPP::Status::Away ||
	//        statusType(index) == XMPP::Status::XA;
}

void YaContactListViewDelegate::drawStatusTypeText(QPainter* painter, const QStyleOptionViewItem& option, QRect* rect, const QModelIndex& index) const
{
	QString statusTypeText = tr(" - %1").arg(Ya::statusName(
	                             statusType(index),
	                             XMPP::VCard::Gender(index.data(ContactListModel::GenderRole).toInt())
	                         ));
	QRect statusTypeRect(*rect);
	QStyleOptionViewItemV2 st_o = option;
	st_o.font.setPixelSize(9);
	st_o.fontMetrics = QFontMetrics(st_o.font);
	QPalette palette = blackText();
	palette.setColor(QPalette::Text, Ya::VisualUtil::statusColor(statusType(index), hovered()));
	st_o.palette = palette;
	statusTypeRect.setWidth(st_o.fontMetrics.width(statusTypeText));
	statusTypeRect.moveTop(statusTypeRect.top() + option.fontMetrics.ascent() - st_o.fontMetrics.ascent());

	if (!statusTypeText.isEmpty()) {
		if (option.fontMetrics.width(index.data(Qt::DisplayRole).toString()) > rect->width() - statusTypeRect.width()) {
			statusTypeRect.moveLeft(rect->right() - statusTypeRect.width() + 1);
			rect->setWidth(statusTypeRect.left() - rect->left());
		}
		else {
			rect->setWidth(option.fontMetrics.width(index.data(Qt::DisplayRole).toString()));
			statusTypeRect.moveLeft(rect->right() + 1);
		}

		if (painter) {
			drawText(painter, st_o, statusTypeRect, statusTypeText, index);
		}
	}
}

void YaContactListViewDelegate::drawEditorBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!contactList() || !contactList()->textInputInProgress())
		return;

	// reset clipping as if we have editor widget shown, the background is effectively drawn only
	// underneath of editor widget
	painter->setClipRect(option.rect);

	drawBackground(painter, option, index);
}

QRect YaContactListViewDelegate::nameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect textRect = this->textRect(option.rect);

	QRect nameRect(textRect);
	nameRect.setHeight(textRect.height() / 2);

	if (statusText(index).isEmpty()) {
		nameRect.moveTop(textRect.top() + (textRect.height() - nameRect.height()) / 2);
	}

	if (drawStatusIcon(statusType(index))) {
		QPixmap statusPixmap = Ya::VisualUtil::rosterStatusPixmap(statusType(index));
		nameRect.translate(statusPixmap.width() + 3, 0);
	}

	nameRect.setRight(option.rect.right() - horizontalMargin() - 1);
	return nameRect;
}

QRect YaContactListViewDelegate::editorRect(const QRect& nameRect) const
{
	return nameRect.adjusted(0, -3, 0, 3);
}

void YaContactListViewDelegate::realDrawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawEditorBackground(painter, option, index);
	doAvatar(painter, option, index);

	bool selected = option.state & QStyle::State_Selected;

	QRect textRect = this->textRect(option.rect);
	// painter->drawRect(textRect);

	QRect nameRect = this->nameRect(option, index);

	QStyleOptionViewItemV2 n_o = nameStyle(selected, statusType(index), Ya::VisualUtil::RosterStyleNormal, hovered());

	if (drawStatusIcon(statusType(index))) {
		QPixmap statusPixmap = Ya::VisualUtil::rosterStatusPixmap(statusType(index));
		QRect statusPixmapRect(nameRect);
		statusPixmapRect.setSize(statusPixmap.size());
		statusPixmapRect.moveTop(nameRect.top() + n_o.fontMetrics.ascent() - statusPixmapRect.height() + 1);
		statusPixmapRect.moveLeft(nameRect.left() - (statusPixmap.width() + 3));
		painter->drawPixmap(statusPixmapRect, statusPixmap);
	}

	if (drawStatusTypeText(index))
		drawStatusTypeText(painter, n_o, &nameRect, index);
	drawName(painter, n_o, nameRect, nameText(option, index), index);

	// painter->drawRect(nameRect);
	// painter->drawRect(statusTypeRect);

	// begin - status message
	QRect statusRect(nameRect);
	statusRect.moveTo(statusRect.left(), textRect.bottom() - statusRect.height());
	statusRect.setRight(textRect.right());
	statusRect.adjust(0, (int)(((float)statusRect.height()) * 0.2), 0, 0);
	// painter->drawRect(statusRect);

	if (!statusText(index).isEmpty()) {
		QStyleOptionViewItemV2 sm_o = statusTextStyle(Ya::VisualUtil::RosterStyleNormal, hovered());
		sm_o.palette.setColor(QPalette::Highlight, option.palette.color(QPalette::Highlight));
		drawStatus(painter, sm_o, statusRect, statusText(index), index);
	}
}

static int groupButtonMargin = 7;

QRect YaContactListViewDelegate::groupButtonRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect buttonRectDest(0, 0, 11, 11);
	buttonRectDest.moveTo(option.rect.left() + groupButtonMargin - 2,
	                      option.rect.top() + (option.rect.height() - buttonRectDest.height()) / 2);
	return buttonRectDest;
}

QRect YaContactListViewDelegate::groupNameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect buttonRectDest = this->groupButtonRect(option, index);

	// TODO: use groupFontSize
	QStyleOptionViewItemV2 o = opt();
	QRect groupNameRect;
	o.font.setBold(true);
	o.font.setPixelSize(11);
	o.fontMetrics = QFontMetrics(o.font);

	QString text = index.data(Qt::DisplayRole).toString();

	if (!index.data(ContactListModel::ExpandedRole).toBool())
		text += QString(" (%1)").arg(index.model()->rowCount(index));

	groupNameRect.setHeight(o.fontMetrics.height());
	int info_y = (option.rect.height() - o.fontMetrics.ascent()) / 2;
	groupNameRect.setLeft(buttonRectDest.right() + 6);
	groupNameRect.moveTop(option.rect.top() + info_y - 1);
	groupNameRect.setWidth(o.fontMetrics.width(text) + 5);

	if (groupNameRect.right() > option.rect.right() - groupButtonMargin)
		groupNameRect.setRight(option.rect.right() - groupButtonMargin);
	return groupNameRect;
}

void YaContactListViewDelegate::drawGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// painter->drawRect(option.rect);

	QRect buttonRectDest = this->groupButtonRect(option, index);
	// painter->drawRect(buttonRectDest);

	Ya::DecorationState decorationState = Ya::Normal;
	if (hovered() && buttonRectDest.contains(hoveredPosition())) {
		decorationState = Ya::Hover;
		setHovered(false); // don't draw hovered background
	}

	drawEditorBackground(painter, option, index);
	drawBackground(painter, opt(), index);

	QPixmap button = Ya::groupPixmap(buttonRectDest.size(),
	                                 index.data(ContactListModel::ExpandedRole).toBool(),
	                                 decorationState);
	QRect buttonRect(0, 0, button.width(), button.height());
	painter->drawPixmap(buttonRectDest, button, buttonRect);

	// TODO: use groupFontSize
	QStyleOptionViewItemV2 o = opt();
	o.font.setBold(true);
	o.font.setPixelSize(11);
	o.fontMetrics = QFontMetrics(o.font);

	QString text = index.data(Qt::DisplayRole).toString();

	if (!index.data(ContactListModel::ExpandedRole).toBool())
		text += QString(" (%1)").arg(index.model()->rowCount(index));

	o.rect = groupNameRect(option, index);

	QPalette groupNamePalette = blackText();
	groupNamePalette.setColor(QPalette::Text, Ya::VisualUtil::rosterGroupForeColor());
	o.palette = groupNamePalette;

	if (contactList() && contactList()->indexWidget(index)) {
		return;
	}

	drawText(painter, o, o.rect, text, index);
	// painter->drawRect(o.rect);

	QPixmap background = Ya::VisualUtil::dashBackgroundPixmap();
	QRect groupRect(o.rect.right() - 1, 0, option.rect.width(), background.height());
	groupRect.moveTo(groupRect.left(),
	                 option.rect.top() + (option.rect.height() - groupRect.height()) / 2 + 1);
	painter->setClipRect(groupRect);
	groupRect.setLeft(0);
	painter->drawTiledPixmap(groupRect, background);
}

void YaContactListViewDelegate::drawAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(index);
}

int YaContactListViewDelegate::avatarSize() const
{
	return avatarSize_; // ContactListViewDelegate::avatarSize();
}

void YaContactListViewDelegate::setAvatarSize(int avatarSize)
{
	avatarSize_ = avatarSize;
}

void YaContactListViewDelegate::drawText(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text, const QModelIndex& index) const
{
	QStyleOptionViewItem opt = option;
	// if (!(option.state & QStyle::State_Active))
	// 	opt.state &= ~QStyle::State_Selected;

	ContactListViewDelegate::drawText(painter, opt, rect, text, index);
}

void YaContactListViewDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
	// 	if (!(option.state & QStyle::State_Active) && parent()) {
	// 		QWidget* parentWidget = static_cast<QWidget*>(parent());
	// 		YaContactListView* contactListView = dynamic_cast<YaContactListView*>(parentWidget);
	// 		if (contactListView && !contactListView->drawSelectionBackground()) {
	// 			return;
	// 		}
	// 	}
	// }

	ContactListViewDelegate::drawBackground(painter, option, index);
}

bool YaContactListViewDelegate::drawStatusIcon(XMPP::Status::Type type) const
{
	return drawStatusIcon() ||
	       type == XMPP::Status::Away ||
	       type == XMPP::Status::XA ||
	       type == XMPP::Status::DND ||
	       type == XMPP::Status::Blocked ||
	       type == XMPP::Status::Reconnecting ||
	       type == XMPP::Status::NotAuthorizedToSeeStatus;
}

bool YaContactListViewDelegate::drawStatusIcon() const
{
	return drawStatusIcon_;
}

void YaContactListViewDelegate::setDrawStatusIcon(bool draw)
{
	drawStatusIcon_ = draw;
}

QRect YaContactListViewDelegate::rosterToolTipArea(const QRect& rect) const
{
	return rect.adjusted(0, 3, 0, 0);
}

QWidget* YaContactListViewDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	YaEmptyTextLineEdit* result = new YaEmptyTextLineEdit(parent);
	initEditor(result);

	contactList()->setEditingIndex(index, true);

	return result;
}

void YaContactListViewDelegate::initEditor(YaEmptyTextLineEdit* editor) const
{
	editor->setFrame(false);
	editor->setOkButtonVisible(true);
	editor->setCancelButtonVisible(true);
	// editor->setText(index.data(Qt::EditRole).toString());
	// editor->internalLineEdit()->setFocus();
	editor->installEventFilter(const_cast<YaContactListViewDelegate*>(this));
	editor->internalLineEdit()->installEventFilter(const_cast<YaContactListViewDelegate*>(this));
	editor->setDisableFocusChanging(true);
	connect(editor, SIGNAL(focusOut()), SLOT(editorFocusOut()));
}

void YaContactListViewDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	ContactListViewDelegate::updateEditorGeometry(editor, option, index);

	QRect widgetRect;
	QRect lineEditRect;
	getEditorGeometry(editor, option, index, &widgetRect, &lineEditRect);

	if (!widgetRect.isEmpty()) {
		YaEmptyTextLineEdit* lineEdit = dynamic_cast<YaEmptyTextLineEdit*>(editor);
		if (lineEdit) {
			lineEdit->setLineEditGeometry(lineEditRect);
		}
	}
}

void YaContactListViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	// same code is in ContactListViewDelegate::setEditorData
	YaEmptyTextLineEdit* yaLineEdit = dynamic_cast<YaEmptyTextLineEdit*>(editor);
	if (yaLineEdit) {
		if (yaLineEdit->text().isEmpty()) {
			yaLineEdit->setText(index.data(Qt::EditRole).toString());
			yaLineEdit->selectAll();
		}
		return;
	}

	ContactListViewDelegate::setEditorData(editor, index);
}

void YaContactListViewDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	// same code is in ContactListViewDelegate::setModelData
	YaEmptyTextLineEdit* yaLineEdit = dynamic_cast<YaEmptyTextLineEdit*>(editor);
	QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);
	if (yaLineEdit) {
		if (index.data(Qt::EditRole).toString() != yaLineEdit->text()) {
			model->setData(index, yaLineEdit->text(), Qt::EditRole);
		}
	}
	else {
		ContactListViewDelegate::setModelData(editor, model, index);
	}
}

void YaContactListViewDelegate::setEditorCursorPosition(QWidget* editor, int cursorPosition) const
{
	ContactListViewDelegate::setEditorCursorPosition(editor, cursorPosition);

	YaEmptyTextLineEdit* lineEdit = dynamic_cast<YaEmptyTextLineEdit*>(editor);
	if (lineEdit) {
		if (cursorPosition == -1)
			cursorPosition = lineEdit->text().length();
		lineEdit->setCursorPosition(cursorPosition);
	}
}

QColor YaContactListViewDelegate::backgroundColor(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (hovered()) {
		return Ya::VisualUtil::hoverHighlightColor();
	}
	return ContactListViewDelegate::backgroundColor(option, index);
}

void YaContactListViewDelegate::editorFocusOut()
{
	QWidget *editor = ::qobject_cast<QWidget*>(sender());
	if (editor) {
		emit closeEditor(editor, NoHint);
	}
}
