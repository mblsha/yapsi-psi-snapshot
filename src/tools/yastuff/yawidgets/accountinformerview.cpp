/*
 * accountinformerview.cpp - editable account list widget
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

#include "accountinformerview.h"

#include <QHeaderView>
#include <QMouseEvent>
#include <QTimer>

#include "accountinformerdelegate.h"
#include "yavisualutil.h"
#include "smoothscrollbar.h"
#include "accountinformermodel.h"
#include "psitooltip.h"
#include "yaofficebackgroundhelper.h"

//----------------------------------------------------------------------------
// AccountInformerView
//----------------------------------------------------------------------------

AccountInformerView::AccountInformerView(QWidget* parent)
	: HoverableTreeView(parent)
	, delegate_(0)
{
	// TODO: it would look better without frame, but current background color is
	// the same as selection color
	// setFrameShape(QFrame::NoFrame);
	setUniformRowHeights(false);
	setAlternatingRowColors(false);
	setRootIsDecorated(false);
	setIndentation(0);
	header()->hide();

	// on Macs Enter key is the default EditKey, so we can't use EditKeyPressed
	setEditTriggers(QAbstractItemView::EditKeyPressed); // lesser evil, otherwise no editing will be possible at all due to Qt bug
	// setEditTriggers(QAbstractItemView::NoEditTriggers);

	QAbstractItemDelegate* delegate = itemDelegate();
	delete delegate;

	delegate_ = new AccountInformerDelegate(this);
	delegate_->setParent(this);
	setItemDelegate(delegate_);

	SmoothScrollBar::install(this);

	setMouseTracking(true);
	connect(this, SIGNAL(entered(const QModelIndex&)), SLOT(updateSelection(const QModelIndex&)));
	connect(this, SIGNAL(viewportEntered()), SLOT(updateSelection()));
	connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
	connect(this, SIGNAL(clicked(const QModelIndex&)), SLOT(clicked(const QModelIndex&)));

	YaOfficeBackgroundHelper::instance()->registerWidget(this);
}

void AccountInformerView::setModel(QAbstractItemModel* model)
{
	HoverableTreeView::setModel(model);
	connect(model, SIGNAL(editPassword()), SLOT(editPassword()));
	connect(model, SIGNAL(accountError(const QModelIndex&)), SLOT(accountError(const QModelIndex&)));
}

AccountInformerDelegate* AccountInformerView::itemDelegate() const
{
	return static_cast<AccountInformerDelegate*>(HoverableTreeView::itemDelegate());
}

void AccountInformerView::leaveEvent(QEvent* e)
{
	HoverableTreeView::leaveEvent(e);
	updateSelection();
}

void AccountInformerView::updateSelection()
{
	updateSelection(QModelIndex());
}

void AccountInformerView::updateSelection(const QModelIndex& index)
{
	// FIXME: currently it's hard to delete an account with selection-follows-mouse enabled
	return;

	if (state() == EditingState)
		return;

	int value = verticalScrollBar()->value();
	if (index.isValid())
		setCurrentIndex(index);
	else
		clearSelection();
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setValueImmediately(value);

	viewport()->update();
	updateCursor();
}

QModelIndex AccountInformerView::selectedIndex() const
{
	if (!selectedIndexes().isEmpty())
		return selectedIndexes().first();
	return QModelIndex();
}

QModelIndexList AccountInformerView::selectedIndexes() const
{
	return HoverableTreeView::selectedIndexes();
}

bool AccountInformerView::isHoveringEnabledCheckbox(const QModelIndex& index, const QPoint& cursorPos)
{
	if (index.isValid() && itemDelegate()->isHoveringEnabledCheckbox(getStyleOptionViewItem(index), index, cursorPos))
		return true;

	return false;
}

bool AccountInformerView::isHoveringChangePassword(const QModelIndex& index, const QPoint& cursorPos)
{
	if (index.isValid() && itemDelegate()->isHoveringChangePassword(getStyleOptionViewItem(index), index, cursorPos))
		return true;

	return false;
}

void AccountInformerView::updateCursor()
{
	Qt::CursorShape cursorShape = Qt::ArrowCursor;
	if (isHoveringEnabledCheckbox(selectedIndex(), cursorPosition_) || isHoveringChangePassword(selectedIndex(), cursorPosition_))
		cursorShape = Qt::PointingHandCursor;
	setCursor(cursorShape);
}

void AccountInformerView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	static bool closingEditors = false;
	if (closingEditors)
		return;

	if (selected.indexes().isEmpty()) {
		closingEditors = true;
		foreach(QModelIndex i, deselected.indexes()) {
			QWidget* w = indexWidget(i);
			if (!w)
				continue;

			setCurrentIndex(i);
			closeEditor(w, QAbstractItemDelegate::NoHint);
		}
		closingEditors = false;
		clearSelection();
	}

	itemDelegate()->setSelection(selected);
	if (selectedIndex().isValid()) {
		if (!model()->indexEditable(selectedIndex()))
			edit(selectedIndex());
	}
	scheduleDelayedItemsLayout();
	emit selectionChanged(selected);
}

void AccountInformerView::mouseMoveEvent(QMouseEvent* e)
{
	HoverableTreeView::mouseMoveEvent(e);

	if (e->type() == QEvent::MouseMove) {
		cursorPosition_ = e->pos();
		updateCursor();
	}
}

void AccountInformerView::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Space:
	case Qt::Key_Enter:
	case Qt::Key_Return:
		if (state() != EditingState || hasFocus()) {
			if (currentIndex().isValid())
				emit activated(currentIndex());
		}
		else {
			event->ignore();
		}
		break;
	default:
		HoverableTreeView::keyPressEvent(event);
	}
}

void AccountInformerView::mouseDoubleClickEvent(QMouseEvent*)
{
}

void AccountInformerView::itemActivated(const QModelIndex& index)
{
	if (model()->editMode(selectedIndex()) == AccountInformerModel::EditJID) {
		edit(index);
	}
	else if (model()->editMode(index) != AccountInformerModel::EditDisabled) {
		changeEnabledRole(index);
	}
}

void AccountInformerView::mousePressEvent(QMouseEvent* e)
{
	QPersistentModelIndex index = indexAt(e->pos());
	if ((index == selectedIndex()) &&
	    (isHoveringEnabledCheckbox(index, e->pos()) ||
	     isHoveringChangePassword(index, e->pos())))
	{
		pressedPosition_ = e->pos();
	}
	else {
		pressedPosition_ = QPoint();
	}

	if (!index.isValid()) {
		clearSelection();
	}

	HoverableTreeView::mousePressEvent(e);
}

void AccountInformerView::mouseReleaseEvent(QMouseEvent* e)
{
	if (isHoveringEnabledCheckbox(selectedIndex(), e->pos()) ||
	    isHoveringChangePassword(selectedIndex(), e->pos()))
	{
		// good
	}
	else {
		pressedPosition_ = QPoint();
	}

	HoverableTreeView::mouseReleaseEvent(e);
}

void AccountInformerView::clicked(const QModelIndex& index)
{
	if (state() == EditingState)
		return;

	if (isHoveringEnabledCheckbox(selectedIndex(), pressedPosition_)) {
		itemActivated(index);
	}
	else if (isHoveringChangePassword(selectedIndex(), pressedPosition_)) {
		editPassword();
	}
	else if (model()->editMode(selectedIndex()) == AccountInformerModel::EditJID) {
		itemActivated(index);
	}
}

void AccountInformerView::changeEnabledRole(const QModelIndex& index)
{
	if (state() == EditingState)
		return;

	model()->setData(index, QVariant(true), AccountInformerModel::ChangeEnabledRole);
}

void AccountInformerView::editPassword()
{
	if (model()->indexEditable(currentIndex()))
		edit(currentIndex());
}

void AccountInformerView::editNewAccount()
{
	Q_ASSERT(model()->rowCount() > 0);
	QModelIndex index = model()->index(model()->rowCount()-1, 0);
	setCurrentIndex(index);
	edit(index);
}

AccountInformerModel* AccountInformerView::model() const
{
	return static_cast<AccountInformerModel*>(HoverableTreeView::model());
}

void AccountInformerView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
	HoverableTreeView::closeEditor(editor, hint);

	if (model()->editMode(selectedIndex()) == AccountInformerModel::EditJID) {
		emit deleteAccount();
	}
}

void AccountInformerView::commitData(QWidget* editor)
{
	bool enteringJid = !model()->indexEditable(currentIndex());

	HoverableTreeView::commitData(editor);

	if (enteringJid && model()->indexEditable(currentIndex()))
		QTimer::singleShot(0, this, SLOT(editPassword()));
}

bool AccountInformerView::viewportEvent(QEvent* event)
{
	if (event->type() == QEvent::ToolTip &&
	    (isActiveWindow() || window()->testAttribute(Qt::WA_AlwaysShowToolTips))) {
		QHelpEvent* he = static_cast<QHelpEvent*>(event);
		PsiToolTip::showText(he->globalPos(), itemDelegate()->tooltip(indexAt(he->pos())), this);
		return true;
	}

	return HoverableTreeView::viewportEvent(event);
}

void AccountInformerView::accountError(const QModelIndex& index)
{
	scrollTo(index);
	setState(QAbstractItemView::NoState);
	setCurrentIndex(QModelIndex());
}

QStyleOptionViewItem AccountInformerView::getStyleOptionViewItem(const QModelIndex& index) const
{
	QStyleOptionViewItem opt;
	opt.font = delegate_->defaultFont();
	opt.fontMetrics = QFontMetrics(opt.font);
	opt.rect = visualRect(index);
	return opt;
}
