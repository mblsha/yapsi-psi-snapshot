/*
 * accountinformerdelegate.cpp - draws an editable account list
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

#include "accountinformerdelegate.h"

#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QCheckBox>

#include "accountinformermodel.h"
#include "contactlistmodel.h"
#include "yacontactlistviewslimdelegate.h"
#include "yavisualutil.h"
#include "yaemptytextlineedit.h"

// TODO: look at QAbstractItemView::setItemDelegateForRow() as an alternative
// to simplify the code

static QStyle* style()
{
	return qApp->style();
}

AccountInformerDelegate::AccountInformerDelegate(QAbstractItemView* itemView)
	: YaContactListViewDelegate(0)
	, itemView_(itemView)
{
	checkBox_ = new QCheckBox(0);

	normalDelegate_ = new YaContactListViewSlimDelegate(0);
	normalDelegate_->setDrawStatusIcon(true);
	normalDelegate_->setHorizontalMargin(10);

	selectionDelegate_ = new YaContactListViewDelegate(0);
	selectionDelegate_->setDrawStatusIcon(true);
	selectionDelegate_->setHorizontalMargin(10);
	selectionDelegate_->setVerticalMargin(10);
	selectionDelegate_->setAvatarSize(45);

	contactModel_ = new QStandardItemModel(this);
	contactItem_ = new QStandardItem();
	contactModel_->invisibleRootItem()->appendRow(contactItem_);
}

AccountInformerDelegate::~AccountInformerDelegate()
{
	delete checkBox_;
	delete normalDelegate_;
	delete selectionDelegate_;
}

QFont AccountInformerDelegate::defaultFont()
{
	QFont result("Arial");
	result.setPixelSize(12);
	return result;
}

void AccountInformerDelegate::setSelection(const QItemSelection& selection)
{
	selection_ = selection;
}

QRect AccountInformerDelegate::checkBoxRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionButton opt = checkBoxOption(index);

	QSize sz = style()->itemTextRect(opt.fontMetrics, QRect(0, 0, 1, 1), Qt::TextShowMnemonic, false, opt.text).size();
	QSize sizeHint; // = style()->sizeFromContents(QStyle::CT_CheckBox, &opt, sz);
	sizeHint.setWidth(opt.fontMetrics.width(checkBoxText(index.data(AccountInformerModel::EnabledRole).toBool())));
	sizeHint.setHeight(opt.fontMetrics.height());

	QRect textRect = selectionDelegate_->textRect(option.rect);
	QRect nameRect = selectionDelegate_->nameRect(option, index);
	QRect statusRect(nameRect);
	statusRect.moveTo(statusRect.left(), textRect.bottom() - statusRect.height());
	statusRect.setRight(textRect.right());
	statusRect.adjust(0, (int)(((float)statusRect.height()) * 0.2), 0, 0);

	QRect r;
	r.setSize(sizeHint);
	r.moveTo(statusRect.topLeft());
	r.moveLeft(textRect.left());
	return r;
}

QString AccountInformerDelegate::editPasswordText() const
{
	return tr("Edit password");
}

QString AccountInformerDelegate::checkBoxText(bool enabled) const
{
	if (enabled)
		return tr("Disable");
	else
		return tr("Enable");
}

QRect AccountInformerDelegate::changePasswordRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect checkBoxRect = this->checkBoxRect(option, index);

	QRect textRect = selectionDelegate_->textRect(option.rect);
	QRect result(checkBoxRect);

	// dirty hack for windows
	QFontMetrics fm(defaultFont());
	result.setWidth(fm.width(editPasswordText()));
	result.moveLeft(textRect.right() - result.width());

	if (passwordEmpty(index)) {
		result.moveLeft(checkBoxRect.left());
	}

	return result;
}

QStyleOptionButton AccountInformerDelegate::checkBoxOption(const QModelIndex& index) const
{
	QStyleOptionViewItemV2 statusTextOption = statusTextStyle(Ya::VisualUtil::RosterStyleNormal, hovered());

	QStyleOptionButton result;
	// result.font = defaultFont();
	result.fontMetrics = QFontMetrics(defaultFont());
	// result.rect = checkBoxRect(option, index);
	result.text = checkBoxText(index.data(AccountInformerModel::EnabledRole).toBool());
	result.state |= QStyle::State_Enabled;
	if (index.data(AccountInformerModel::EnabledRole).toBool())
		result.state |= QStyle::State_On;
	return result;
}

QColor AccountInformerDelegate::errorColor()
{
	return QColor(0xFF, 0x66, 0x66);
}

bool AccountInformerDelegate::passwordEmpty(const QModelIndex& index) const
{
	return index.data(Qt::EditRole).toString().isEmpty();
}

void AccountInformerDelegate::paint(QPainter* painter, const QStyleOptionViewItem& _option, const QModelIndex& index) const
{
	QStyleOptionViewItem option(_option);
	// reset clipping as if we have editor widget shown, the background is effectively drawn only
	// underneath of editor widget
	painter->setClipRect(option.rect);

	// if (!index.data(AccountInformerModel::CurrentConnectionErrorRole).toString().isEmpty()) {
	// 	painter->fillRect(option.rect, errorColor());
	// 	option.palette.setColor(QPalette::Highlight, errorColor());
	// }

	YaContactListViewDelegate* delegate = delegateFor(option, index);
	delegate->doSetOptions(option, index);
	delegate->drawContact(painter, option, indexFor(option, index));

	if (delegate == selectionDelegate_ && model(index)->editMode(index) == AccountInformerModel::EditPassword) {
		if (itemView_->indexWidget(index)) {
			return;
		}

		QFont font = defaultFont();
		// painter->setPen(QColor(0x00,0x00,0xCC));
		// painter->setPen(QColor(0xE2,0x33,0x00));
		painter->setPen(Ya::VisualUtil::linkButtonColor());
		font.setUnderline(true);
		painter->setFont(font);

		// painter->drawRect(checkBoxRect(option, index));

		if (!passwordEmpty(index)) {
			QString text = checkBoxText(index.data(AccountInformerModel::EnabledRole).toBool());
			painter->drawText(checkBoxRect(option, index),
			                  Qt::AlignLeft | Qt::AlignVCenter, text);
		}

		{
			QString text = editPasswordText();
			painter->drawText(changePasswordRect(option, index),
			                  Qt::AlignLeft | Qt::AlignVCenter, text);
		}
	}
}

QSize AccountInformerDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return delegateFor(option, index)->sizeHint(option, indexFor(option, index));
}

QWidget* AccountInformerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	YaEmptyTextLineEdit* result = new YaEmptyTextLineEdit(parent);
	initEditor(result);

	YaEmptyTextLineEdit* lineEdit = dynamic_cast<YaEmptyTextLineEdit*>(result);
	if (lineEdit) {
		if (model(index)->editMode(index) == AccountInformerModel::EditDisabled) {
			delete result;
			return 0;
		}
		else if (model(index)->editMode(index) == AccountInformerModel::EditPassword) {
			lineEdit->setEmptyText(tr("Password"));
			lineEdit->setEchoMode(QLineEdit::Password);
			lineEdit->deselect();
		}
		else {
			Q_ASSERT(model(index)->editMode(index) == AccountInformerModel::EditJID);
			lineEdit->setEmptyText(tr("Login"));
		}
	}

	return result;
}

void AccountInformerDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect widgetRect;
	QRect lineEditRect;
	switch (model(index)->editMode(index)) {
	case AccountInformerModel::EditPassword: {
		QRect textRect = selectionDelegate_->textRect(option.rect);
		widgetRect = changePasswordRect(option, index);
		widgetRect.setLeft(textRect.left());
		widgetRect.setRight(textRect.right());
		lineEditRect = widgetRect;
		widgetRect.adjust(0, -3, 0, 3);
		break;
	}
	case AccountInformerModel::EditJID:
		widgetRect = selectionDelegate_->textRect(option.rect);
		widgetRect.adjusted(0, -3, 0, 3);
		lineEditRect = widgetRect;
		break;
	}

	if (!widgetRect.isEmpty()) {
		editor->setGeometry(widgetRect);

		YaEmptyTextLineEdit* lineEdit = dynamic_cast<YaEmptyTextLineEdit*>(editor);
		if (lineEdit) {
			lineEdit->setLineEditGeometry(lineEditRect);
		}
	}
}

YaContactListViewDelegate* AccountInformerDelegate::delegateFor(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (option.state & QStyle::State_Selected || selection_.contains(index)) {
		return selectionDelegate_;
	}

	return normalDelegate_;
}

const QModelIndex& AccountInformerDelegate::indexFor(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	contactItem_->setText(index.data().toString());
	contactItem_->setData(QVariant(index.data().toString()), ContactListModel::JidRole);
	contactItem_->setData(QVariant(ContactListModel::ContactType), ContactListModel::TypeRole);
	contactItem_->setData(index.data(AccountInformerModel::StatusTypeRole), ContactListModel::StatusTypeRole);
	contactItem_->setData(QVariant(QString(" ")), ContactListModel::StatusTextRole);
	contactItem_->setData(index.data(AccountInformerModel::PictureRole), ContactListModel::PictureRole);
	contactItemIndex_ = contactItem_->index();
	return contactItemIndex_;
}

bool AccountInformerDelegate::isHoveringChangePassword(const QStyleOptionViewItem& option, const QModelIndex& index, const QPoint& pos)
{
	if (!model(index)->indexEditable(index))
		return false;
	return changePasswordRect(option, index).contains(pos);
}

bool AccountInformerDelegate::isHoveringEnabledCheckbox(const QStyleOptionViewItem& option, const QModelIndex& index, const QPoint& pos)
{
	if (!model(index)->indexEditable(index))
		return false;
	return checkBoxRect(option, index).contains(pos);
}

const AccountInformerModel* AccountInformerDelegate::model(const QModelIndex& index) const
{
	return static_cast<const AccountInformerModel*>(index.model());
}

QString AccountInformerDelegate::tooltip(const QModelIndex& index) const
{
	return index.data(AccountInformerModel::CurrentConnectionErrorRole).toString();
}
