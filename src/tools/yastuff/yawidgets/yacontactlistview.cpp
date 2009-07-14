/*
 * yacontactlistview.cpp - custom contact list widget
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

#include <QStack>
#include <QString>
#include <QAbstractItemView>
#include <QTreeView>

#include "yacontactlistview.h"

#include <QScrollBar>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QDragMoveEvent>
#include <QSortFilterProxyModel>

#include "contactlistgroup.h"
#include "yarostertooltip.h"
#include "contactlistmodel.h"
#include "contactlistitemproxy.h"
#include "psioptions.h"
#include "psicontact.h"
#include "yacontactlistviewdelegate.h"
#include "smoothscrollbar.h"
#include "yavisualutil.h"
#include "yacontactlistmodel.h"
#include "iconaction.h"
#include "shortcutmanager.h"
#include "contactlistitemmenu.h"
#include "yaofficebackgroundhelper.h"
#include "contactlistgroupstate.h"
#include "yawindow.h"
#include "yaexception.h"

#include "yacontactlistviewdelegate.h"
#include "yacontactlistviewslimdelegate.h"
#include "yacontactlistviewlargedelegate.h"
#include "contactlistmodelselection.h"

static const int SCROLL_SINGLE_STEP = 32;
static const QString optionPathTemplate = QString::fromUtf8("options.ya.roster.%1");
static const QString altRowColorOptionPath = optionPathTemplate.arg("altColor");

YaContactListView::YaContactListView(QWidget* parent)
	: ContactListDragView(parent)
	, invalidateDelegateTimer_(0)
	, slimDelegate_(0)
	, normalDelegate_(0)
	, largeDelegate_(0)
	, avatarMode_(AvatarMode_Auto)
{
	QAbstractItemDelegate* delegate = itemDelegate();
	delete delegate;

	slimDelegate_   = new YaContactListViewSlimDelegate(this);
	normalDelegate_ = new YaContactListViewDelegate(this);
	largeDelegate_  = new YaContactListViewLargeDelegate(this);
	invalidateDelegate();

	setAlternatingRowColors(PsiOptions::instance()->getOption(altRowColorOptionPath).toBool());
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	verticalScrollBar()->setSingleStep(SCROLL_SINGLE_STEP);
	setIndentation(8);

	invalidateDelegateTimer_ = new QTimer(this);
	invalidateDelegateTimer_->setInterval(0);
	invalidateDelegateTimer_->setSingleShot(true);
	connect(invalidateDelegateTimer_, SIGNAL(timeout()), SLOT(invalidateDelegate()));

	YaOfficeBackgroundHelper::instance()->registerWidget(this);

	SmoothScrollBar::install(this);
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scrollbarValueChanged()));
}

YaContactListView::~YaContactListView()
{
	PsiOptions::instance()->setOption(altRowColorOptionPath, alternatingRowColors());
}

void YaContactListView::resizeEvent(QResizeEvent* e)
{
	ContactListDragView::resizeEvent(e);
	startInvalidateDelegate();
}

/**
 * This slot is called by YaToolBoxPage when current page is being
 * changed. The purpose of this slot is to avoid visual glitches
 * like scrollbars appearing in the middle of contact list and
 * group header lines being not wide enough.
 */
void YaContactListView::resized()
{
	setViewportMargins(0, 0, 0, 0);
	modelChanged();
	doItemsLayout();
}

bool YaContactListView::updateCursor(const QModelIndex& index, UpdateCursorOrigin origin, bool force)
{
	int value = (origin == UC_MouseClick) ? verticalScrollBar()->value() : -1;

	bool result = ContactListDragView::updateCursor(index, origin, force);

	if (result && value != -1) {
		dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setValueImmediately(value);
	}
}

void YaContactListView::showToolTip(const QModelIndex& index, const QPoint& globalPos) const
{
	Q_UNUSED(globalPos);
	// if (ContactListModel::indexType(index) != ContactListModel::ContactType) {
	// 	ContactListDragView::showToolTip(index, globalPos);
	// 	return;
	// }

	ContactListItemProxy* item = itemProxy(index);

	if (item && !extendedSelectionAllowed() && ContactListModel::indexType(index) == ContactListModel::ContactType)
	{
		QRect rect = dynamic_cast<YaContactListViewDelegate*>(itemDelegate())->rosterToolTipArea(visualRect(index));
		QRect itemRect = QRect(viewport()->mapToGlobal(rect.topLeft()),
		                       viewport()->mapToGlobal(rect.bottomRight()));
		YaRosterToolTip::instance()->showText(itemRect,
		                                      dynamic_cast<PsiContact*>(item->item()),
		                                      this,
		                                      model()->mimeData(QModelIndexList() << index));
	}
	else if (ContactListModel::indexType(index) == ContactListModel::GroupType) {
		item = 0;
	}

	if (!item) {
		YaRosterToolTip::instance()->hide();
	}
}

void YaContactListView::itemExpanded(const QModelIndex& index)
{
	ContactListDragView::itemExpanded(index);
	startInvalidateDelegate();
}

void YaContactListView::itemCollapsed(const QModelIndex& index)
{
	ContactListDragView::itemCollapsed(index);
	startInvalidateDelegate();
}

void YaContactListView::startInvalidateDelegate()
{
	invalidateDelegateTimer_->start();
}

void YaContactListView::invalidateDelegate()
{
	switch (avatarMode()) {
	case AvatarMode_Disable:
		setItemDelegate(slimDelegate_);
		break;
	case AvatarMode_Big:
		setItemDelegate(largeDelegate_);
		break;
	case AvatarMode_Small:
		setItemDelegate(normalDelegate_);
		break;
	case AvatarMode_Auto:
	default:
		if (indexCombinedHeight(QModelIndex(), largeDelegate_) <= viewport()->height())
			setItemDelegate(largeDelegate_);
		else
			setItemDelegate(normalDelegate_);
		break;
	}
}

void YaContactListView::setModel(QAbstractItemModel* newModel)
{
	if (model()) {
		disconnect(model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
		disconnect(model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
	}

	// it's critical that we hook on signals prior to selectionModel,
	// otherwise it would be pretty hard to maintain consistent selection
	if (newModel) {
		connect(newModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
		connect(newModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
	}

	ContactListDragView::setModel(newModel);
	startInvalidateDelegate();

	if (newModel) {
		YaContactListModel* yaContactListModel = dynamic_cast<YaContactListModel*>(realModel());
		Q_ASSERT(yaContactListModel);
		connect(yaContactListModel->contactList(), SIGNAL(firstErrorMessageChanged(const QString&)), SLOT(update()));
	}
}

YaContactListView::AvatarMode YaContactListView::avatarMode() const
{
	return avatarMode_;
}

void YaContactListView::setAvatarMode(YaContactListView::AvatarMode avatarMode)
{
	avatarMode_ = avatarMode;
	invalidateDelegate();
}

void YaContactListView::paintEvent(QPaintEvent* e)
{
	try {
		ContactListDragView::paintEvent(e);

		{
			QPainter p(viewport());
			YaWindow* yaWindow = dynamic_cast<YaWindow*>(window());
			Q_ASSERT(yaWindow);
			Ya::VisualUtil::drawAACorners(&p, yaWindow->theme(), rect(), rect().adjusted(0, -Ya::VisualUtil::windowCornerRadius() * 2, 0, 0));
		}
	}
	catch (...) {
		LOG_EXCEPTION;
	}
}

void YaContactListView::dragEnterEvent(QDragEnterEvent* e)
{
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(false);

	ContactListDragView::dragEnterEvent(e);
}

void YaContactListView::dragLeaveEvent(QDragLeaveEvent* e)
{
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(true);

	ContactListDragView::dragLeaveEvent(e);
}

void YaContactListView::dropEvent(QDropEvent* e)
{
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(true);
	ContactListDragView::dropEvent(e);
}

void YaContactListView::startDrag(Qt::DropActions supportedActions)
{
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(false);

	ContactListDragView::startDrag(supportedActions);
}

ContactListItemMenu* YaContactListView::createContextMenuFor(ContactListItem* item) const
{
	ContactListItemMenu* menu = ContactListDragView::createContextMenuFor(item);
	if (menu) {
		if (menu->metaObject()->indexOfSignal("addSelection()") != -1)
			connect(menu, SIGNAL(addSelection()), SLOT(addSelection()));
		if (menu->metaObject()->indexOfSignal("addGroup()") != -1)
			connect(menu, SIGNAL(addGroup()), SIGNAL(addGroup()));
	}
	return menu;
}

void YaContactListView::addSelection()
{
	QMimeData* mimeData = selection();
	emit addSelection(mimeData);
	delete mimeData;
}

YaContactListViewDelegate* YaContactListView::yaContactListViewDelegate() const
{
	YaContactListViewDelegate* delegate = dynamic_cast<YaContactListViewDelegate*>(itemDelegate());
	Q_ASSERT(delegate);
	return delegate;
}

bool YaContactListView::drawSelectionBackground() const
{
	return ContactListDragView::drawSelectionBackground() ||
	       YaRosterToolTip::instance()->isVisible();
}

void YaContactListView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
	ContactListDragView::closeEditor(editor, hint);

#if 0
	if (ContactListModel::indexType(currentIndex()) == ContactListModel::GroupType &&
	    hint != QAbstractItemDelegate::SubmitModelCache) {
		ContactListItemProxy* item = itemProxy(currentIndex());
		ContactListGroup* group = item ? dynamic_cast<ContactListGroup*>(item->item()) : 0;
		if (group->isFake()) {
			QModelIndexList indexes;
			indexes << currentIndex();

			YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
			Q_ASSERT(model);
			QMimeData* selection = model->mimeData(realIndexes(indexes));
			model->removeIndexes(selection);
			delete selection;
		}
	}
#endif
}

void YaContactListView::doItemsLayoutStart()
{
	SmoothScrollBar* smoothScrollBar = dynamic_cast<SmoothScrollBar*>(verticalScrollBar());
	// on Qt 4.3.5 even if we disable the updates on widget, range updates will make the
	// scrollbar show / hide on Windows only (this bug is fixed in Qt 4.4)
	smoothScrollBar->setRangeUpdatesEnabled(false);
}

void YaContactListView::doItemsLayoutFinish()
{
	SmoothScrollBar* smoothScrollBar = dynamic_cast<SmoothScrollBar*>(verticalScrollBar());
	smoothScrollBar->setRangeUpdatesEnabled(true);
	if (backedUpVerticalScrollBarValue() != -1) {
		if (smoothScrollBar) {
			smoothScrollBar->setValueImmediately(backedUpVerticalScrollBarValue());
		}
	}
}
