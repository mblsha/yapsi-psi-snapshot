/*
 * yatabbar.cpp
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

#include "yatabbar.h"

#include <QApplication>
#include <QEvent>
#include <QStyleOptionTabV2>
#include <QPainter>
#include <QHelpEvent>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>
#include <QSysInfo>

#include "yatabwidget.h"
#include "yaclosebutton.h"
#include "yachevronbutton.h"
#include "yachatsendbutton.h"
#ifndef WIDGET_PLUGIN
#include "tabbablewidget.h"
#include "yavisualutil.h"
#endif
#include "psilogger.h"

YaTabBar::YaTabBar(QWidget* parent)
	: OverlayWidget<YaTabBarBase, YaChevronButton>(parent,
		new YaChevronButton(parent->window()))
	, previousCurrentIndex_(-1)
	, draggedTabIndex_(-1)
{
	closeButton_ = new YaCloseButton(this);
	closeButton_->setChatButton(true);
	connect(closeButton_, SIGNAL(clicked()), SLOT(closeButtonClicked()));

	activateTabMapper_ = new QSignalMapper(this);
	connect(activateTabMapper_, SIGNAL(mapped(int)), SLOT(setCurrentIndex(int)));

	setAcceptDrops(true);
}

YaTabBar::~YaTabBar()
{
}

void YaTabBar::closeButtonClicked()
{
	LOG_TRACE;
	emit closeTab(currentIndex());
}

int YaTabBar::maximumWidth() const
{
	return OverlayWidget<YaTabBarBase, YaChevronButton>::maximumWidth() - 10 - extra()->sizeHint().width();
}

QSize YaTabBar::preferredTabSize(const QString& text, bool current) const
{
	QSize sh;
	sh.setHeight(qMax(cachedTextHeight(), cachedIconSize().height()));
	sh += QSize(0, margin() * 2);

	sh.setWidth(0);
	sh += QSize(margin(), 0);
	sh += QSize(cachedIconSize().width(), 0);
	sh += QSize(margin(), 0);
	int textWidth = this->textWidth(text);
	textWidth = qMax(textWidth, this->textWidth("nnn"));
	sh += QSize(textWidth, 0);
	sh += QSize(margin(), 0);

	if (current) {
		sh += QSize(1, 0);
		sh += QSize(closeButton_->sizeHint().width(), 0);
		sh += QSize(margin(), 0);
	}

	return sh;
}

int YaTabBar::minimumTabWidth() const
{
	return preferredTabSize(QString(), false).width();
}

int YaTabBar::maximumTabWidth() const
{
	return preferredTabSize(QString(20, QChar('n')), true).width();
}

QSize YaTabBar::tabSizeHint(int index) const
{
	if (index >= 0 && index < tabSizeHint_.count())
		return tabSizeHint_[index];
	return QSize(0, 0);
}

bool YaTabBar::needToRelayoutTabs()
{
	QStringList tabNames;
	for (int i = 0; i < count(); ++i)
		tabNames += tabText(i);

	if (previousTabNames_ == tabNames &&
	    previousCurrentIndex_ == currentIndex() &&
	    previousSize_.width() == maximumWidth() &&
	    previousFont_ == font().toString())
	{
		return false;
	}

	previousTabNames_ = tabNames;
	previousCurrentIndex_ = currentIndex();
	previousSize_.setWidth(maximumWidth());
	previousFont_ = font().toString();
	return true;
}

QList<int> YaTabBar::visibleTabs()
{
	QList<int> result;
	for (int i = 0; i < count(); ++i) {
		if (!hiddenTabs_.contains(i))
			result << i;
	}
	return result;
}

QSize YaTabBar::relayoutTab(int index, int totalTabWidth, const QList<QSize>& relayoutTabData, bool rightmostTab)
{
	QSize sh = relayoutTabData[index];

	if (rightmostTab && totalTabWidth >= maximumWidth()) {
		int restWidth = 0;
		foreach(int i, visibleTabs()) {
			if (i != index)
				restWidth += relayoutTab(i, totalTabWidth, relayoutTabData, i == visibleTabs().last()).width();
		}

		sh.setWidth(qMin(sh.width(), maximumWidth() - restWidth));
	}

	sh.setWidth(qMax(minimumTabWidth(), qMin(sh.width(), maximumTabWidth())));

	return sh;
}

void YaTabBar::relayoutTabsHelper()
{
	tabSizeHint_.clear();
	QList<int> visibleTabs = this->visibleTabs();

	int totalTabWidth = 0;
	foreach(int i, visibleTabs) {
		totalTabWidth += YaTabBarBase::preferredTabSize(i).width();
	}

	QList<QSize> relayoutTabData;
	foreach(int index, visibleTabs) {
		QSize sh = YaTabBarBase::preferredTabSize(index);

		if (index != currentIndex()) {
			if (totalTabWidth > maximumWidth()) {
				int w = 0;
				int n = visibleTabs.count() - 1;
				if (n > 0)
					w = ((maximumWidth() - YaTabBarBase::preferredTabSize(currentIndex()).width()) / n);
				sh.setWidth(qMax(minimumTabWidth(), qMin(w, sh.width())));
			}
		}

		relayoutTabData << sh;
	}

	foreach(int index, visibleTabs) {
		tabSizeHint_ << relayoutTab(index, totalTabWidth, relayoutTabData, index == visibleTabs.last());
	}
}

void YaTabBar::relayoutTabs()
{
	clearCaches();
	hiddenTabs_.clear();

	while (1) {
		relayoutTabsHelper();

		if (visibleTabs().isEmpty())
			break;

		int totalTabWidth = 0;
		foreach(QSize s, tabSizeHint_)
			totalTabWidth += s.width();
		if (totalTabWidth <= maximumWidth())
			break;

		Q_ASSERT(!visibleTabs().isEmpty());
		hiddenTabs_ << visibleTabs().last();
	}

	updateHiddenTabActions();
	updateLayout();
}

void YaTabBar::updateHiddenTabActions()
{
	qDeleteAll(hiddenTabsActions_);
	hiddenTabsActions_.clear();

	qSort(hiddenTabs_);

	bool useCheckBoxes = true;
#ifdef Q_WS_WIN
	// on XP when icons in menus are enabled, checkbox is not shown,
	// the black square gets drawn outside of icon
	if (QSysInfo::WindowsVersion == QSysInfo::WV_XP) {
		useCheckBoxes = false;
	}
#endif

	QMenu* menu = new QMenu(extra());
	foreach(int index, hiddenTabs_) {
		QString text = tabText(index);
		if (tabData(index).toBool())
			text = "* " + text;
		QAction* action = new QAction(text, this);
		action->setIcon(tabIcon(index));
		if (useCheckBoxes) {
			action->setCheckable(true);
			action->setChecked(index == currentIndex());
		}
		menu->addAction(action);

		connect(action, SIGNAL(triggered()), activateTabMapper_, SLOT(map()));
		activateTabMapper_->setMapping(action, index);

		hiddenTabsActions_ << action;
	}

	if (extra()->menu())
		extra()->menu()->deleteLater();
	extra()->setMenu(menu);
}

void YaTabBar::updateHiddenTabs()
{
	updateHiddenTabActions();
}

QRect YaTabBar::closeButtonRect(int index, const QRect& currentTabRect) const
{
	if (currentTabRect.width() < 10 || index != currentIndex())
		return QRect();
	QRect result(currentTabRect);
	// static const int sizeGripWidth = 12;
	// int sizeGripMargin = qMax(sizeGripWidth - 6 - (maximumWidth() - currentTabRect.right()), 0);
	result.setLeft(result.right() /*- sizeGripMargin*/ - margin() - 1 - closeButton_->sizeHint().width());
	result.setSize(closeButton_->sizeHint());
	result.moveCenter(QPoint(result.center().x(), currentTabRect.center().y()));
	return result;
}

QRect YaTabBar::closeButtonRect() const
{
	int x = 0;
	foreach(int i, tabDrawOrder_) {
		QStyleOptionTabV2 tab = getStyleOption(i);
		QRect tabRect(x, 0, tab.rect.width(), tab.rect.height());
		x += tabRect.width();
		if (i == currentIndex())
			return closeButtonRect(i, tabRect);
	}
	return QRect();
}

QRect YaTabBar::chevronButtonRect() const
{
	QRect r = tabRect(0).adjusted(0, 1, 0, 0);
	r.setWidth(extra()->sizeHint().width());
	r.setHeight(height() - 1);
	r.moveLeft(maximumWidth() + 1);
	return QRect(mapToGlobal(r.topLeft()),
	             mapToGlobal(r.bottomRight()));
}

QRect YaTabBar::extraGeometry() const
{
	return chevronButtonRect();
}

bool YaTabBar::extraShouldBeVisible() const
{
	return !hiddenTabs_.isEmpty();
}

bool YaTabBar::showTabs() const
{
	return count() > 1;
}

void YaTabBar::tabLayoutChange()
{
	if (!layoutUpdatesEnabled())
		return;

	if (needToRelayoutTabs()) {
		relayoutTabs();
		return;
	}

	updateTabDrawOrder();
	updateCloseButtonPosition();
	extra()->setVisible(extraShouldBeVisible());
	extra()->raise();
	setVisible(showTabs());

	updateSendButton();
}

void YaTabBar::updateCloseButtonPosition(const QRect& closeButtonRect)
{
	if (showTabs()) {
		closeButton_->setGeometry(closeButtonRect);
	}
	closeButton_->setVisible(showTabs());
}


void YaTabBar::updateCloseButtonPosition()
{
	updateCloseButtonPosition(closeButtonRect());
}

bool YaTabBar::drawHighlightChevronBackground() const
{
	foreach(int index, hiddenTabs_) {
		if (tabData(index).toBool())
			return true;
	}
	return false;
}

QRect YaTabBar::draggedTabRect() const
{
	if (draggedTabIndex_ != -1) {
		QStyleOptionTabV2 tab = getStyleOption(draggedTabIndex_);
		QRect tabRect(0, 0, tab.rect.width(), tab.rect.height());
		tabRect.moveLeft(qMin(width() - tabRect.width(),
		                      qMax(0, dragPosition_.x() - dragOffset_.x())));
		return tabRect;
	}
	return QRect();
}

void YaTabBar::paintEvent(QPaintEvent*)
{
#ifndef WIDGET_PLUGIN
	setFont(Ya::VisualUtil::normalFont());
#endif

	QStyleOptionTab tabOverlap;
	tabOverlap.shape = shape();

	QPainter p(this);

	// extra()->setHighlightColor(drawHighlightChevronBackground() ? highlightColor() : QColor());
	extra()->setEnableBlinking(drawHighlightChevronBackground());

	// updateTabDrawOrder();

	int x = 0;
	foreach(int i, tabDrawOrder_) {
		QStyleOptionTabV2 tab = getStyleOption(i);
		QRect tabRect(x, 0, tab.rect.width(), tab.rect.height());
		x += tabRect.width();

		if (draggedTabIndex_ == i)
			continue;

		drawTab(&p, i, tabRect);
	}

	if (draggedTabIndex_ != -1) {
		QRect tabRect = draggedTabRect();
		drawTab(&p, draggedTabIndex_, tabRect);
		updateCloseButtonPosition(closeButtonRect(draggedTabIndex_, tabRect));
	}
}

QSize YaTabBar::minimumSizeHint() const
{
	QSize sh = sizeHint();
	sh.setWidth(10);
	return sh;
}

void YaTabBar::mouseReleaseEvent(QMouseEvent* event)
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	if (draggedTabIndex_ != -1) {
		emit reorderTabs(draggedTabIndex_, tabDrawOrder_.indexOf(draggedTabIndex_));
	}

	draggedTabIndex_ = -1;
	// updateTabDrawOrder();
	update();
	updateCloseButtonPosition();

	OverlayWidget<YaTabBarBase, YaChevronButton>::mouseReleaseEvent(event);

	setUpdatesEnabled(updatesEnabled);
}

void YaTabBar::mouseMoveEvent(QMouseEvent* event)
{
	if (draggedTabIndex_ != -1) {
		dragPosition_ = event->pos();
		update();
		return;
	}

	if (!pressedPosition().isNull() &&
	    (pressedPosition() - event->pos()).manhattanLength() > QApplication::startDragDistance())
	{
		dragPosition_ = event->pos();
		startDrag();
		return;
	}
	OverlayWidget<YaTabBarBase, YaChevronButton>::mouseMoveEvent(event);
}

void YaTabBar::updateTabDrawOrder()
{
	tabDrawOrder_.clear();
	for (int i = 0; i < count(); ++i) {
		if (i >= tabSizeHint_.count())
			break;

		if (i != draggedTabIndex_)
			tabDrawOrder_ << i;
	}

	if (draggedTabIndex_ != -1) {
		int x = 0;
		int index = 0;
		foreach(int i, tabDrawOrder_) {
			QStyleOptionTabV2 tab = getStyleOption(i);
			QRect tabRect(x, 0, tab.rect.width(), tab.rect.height());
			x += tabRect.width();

			if (draggedTabRect().x() <= tabRect.center().x()) {
				tabDrawOrder_.insert(index, draggedTabIndex_);
				return;
			}

			index++;
		}

		tabDrawOrder_.append(draggedTabIndex_);
	}
}

void YaTabBar::startDrag()
{
	draggedTabIndex_ = tabAt(pressedPosition());
	if (draggedTabIndex_ != -1 && draggedTabIndex_ == currentIndex()) {
		dragOffset_ = pressedPosition() - tabRect(draggedTabIndex_).topLeft();
		update();
	}
	else {
		draggedTabIndex_ = -1;
		clearPressedPosition();
	}
}

int YaTabBar::draggedTabIndex() const
{
	return draggedTabIndex_;
}
