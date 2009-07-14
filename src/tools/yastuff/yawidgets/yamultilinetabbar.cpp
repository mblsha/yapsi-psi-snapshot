/*
 * yamultilinetabbar.cpp
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

#include "yamultilinetabbar.h"

#include <QPainter>
#include <QToolButton>
#include <QWheelEvent>
#include <QApplication>
#include <QTimer>
#include <QDir>

#include "yaflashingscrollbar.h"
#ifndef WIDGET_PLUGIN
#include "yavisualutil.h"
#endif
#include "psilogger.h"
#include "smoothscrollbar.h"

static const int buttonMargin = 14;

YaMultiLineTabBar::YaMultiLineTabBar(QWidget* parent)
	: YaTabBarBase(parent)
	, numRows_(0)
	, rowOffset_(0)
	, draggedTabIndex_(-1)
	, dragDestinationIndex_(-1)
	, autoScrollCount_(0)
	, closeButtonHit_(false)
{
	scrollUp_ = new QToolButton(this);
	scrollUp_->setText(QString::fromUtf8("↑"));
	connect(scrollUp_, SIGNAL(clicked()), SLOT(scrollUp()));
	scrollUp_->hide();

	scrollDown_ = new QToolButton(this);
	scrollDown_->setText(QString::fromUtf8("↓"));
	connect(scrollDown_, SIGNAL(clicked()), SLOT(scrollDown()));
	scrollDown_->hide();

	scrollBar_ = new YaFlashingScrollBar(this);
	scrollBar_->setPageStep(1);
	connect(scrollBar_, SIGNAL(valueChanged(int)), SLOT(scrollBarValueChanged(int)));
	scrollBar_->hide();

	{
		QDir dir(":images/chat/closetab_button");
		foreach(QString file, dir.entryList()) {
			closePixmaps_ << QPixmap(dir.absoluteFilePath(file));
		}
	}

	closeButtonAnimationTimer_ = new QTimer(this);
	closeButtonAnimationTimer_->setSingleShot(false);
	closeButtonAnimationTimer_->setInterval(20);
	connect(closeButtonAnimationTimer_, SIGNAL(timeout()), SLOT(closeButtonAnimation()));

	autoScrollTimer_ = new QTimer(this);
	autoScrollTimer_->setSingleShot(false);
	autoScrollTimer_->setInterval(50);
	connect(autoScrollTimer_, SIGNAL(timeout()), SLOT(autoScroll()));

	setMouseTracking(true);
}

YaMultiLineTabBar::~YaMultiLineTabBar()
{
}

void YaMultiLineTabBar::scrollUp()
{
	rowOffset_ -= 1;
	tabLayoutChange();
	update();
}

void YaMultiLineTabBar::scrollDown()
{
	rowOffset_ += 1;
	tabLayoutChange();
	update();
}

void YaMultiLineTabBar::scrollBarValueChanged(int value)
{
	rowOffset_ = value;
	tabLayoutChange();
	update();
}

QSize YaMultiLineTabBar::minimumSizeHint() const
{
	QSize sh = sizeHint();
	sh.setWidth(10);
	return sh;
}

QSize YaMultiLineTabBar::sizeHint() const
{
	return QSize(maximumWidth(), (numRows_ + 1) * rowHeight());
}

void YaMultiLineTabBar::paintEvent(QPaintEvent*)
{
#ifndef WIDGET_PLUGIN
	setFont(Ya::VisualUtil::normalFont());
#endif

	QPainter p(this);

	p.setClipRegion(Ya::VisualUtil::bottomAACornersMask(rect()));
	p.fillRect(rect(), tabBackgroundColor());

#ifdef WIDGET_PLUGIN
	QRect r = rect();
	r.setHeight((numRows_ + 1) * rowHeight() + 1);
	p.fillRect(r, Qt::blue);
#endif

	{
		p.save();
#ifndef WIDGET_PLUGIN
		p.setPen(Ya::VisualUtil::rosterTabBorderColor());
#endif
		p.drawLine(rect().topLeft(), rect().topRight());
		p.restore();
	}

	foreach(const int& i, tabDrawOrder_) {
		if (draggedTabIndex_ == i)
			continue;

		drawTab(&p, i, tabRect(i));
	}

	{
		// p.save();
#ifndef WIDGET_PLUGIN
		p.setPen(Ya::VisualUtil::rosterTabBorderColor());
#endif
		// p.fillRect(emptySpace_, Qt::red);
		p.drawLine(emptySpace_.topLeft(), emptySpace_.topRight());

		QPixmap shadow(tabShadow(false));
		QRect r(emptySpace_);
		r.setHeight(shadow.height());
		p.drawTiledPixmap(r, shadow);
		// p.restore();
	}

	if (draggedTabIndex_ != -1) {
		// p.setOpacity(0.35);
		// p.fillRect(dragEmptySpaceRect_, Ya::VisualUtil::tabHighlightColor());
		// p.setOpacity(1.0);
#ifndef WIDGET_PLUGIN
		p.setPen(Ya::VisualUtil::rosterTabBorderColor());
#endif
		p.drawLine(dragEmptySpaceRect_.topLeft(), dragEmptySpaceRect_.topRight());
		p.drawLine(dragEmptySpaceRect_.topRight(), dragEmptySpaceRect_.bottomRight());

		QPixmap shadow(tabShadow(false));
		QRect r(dragEmptySpaceRect_);
		r.setHeight(shadow.height());
		p.drawTiledPixmap(r, shadow);

		drawTab(&p, draggedTabIndex_, tabRect(draggedTabIndex_));
	}

	Ya::VisualUtil::drawAACorners(&p, theme(), rect(), rect().adjusted(0, -Ya::VisualUtil::windowCornerRadius() * 2, 0, 0));
}

QSize YaMultiLineTabBar::tabSizeHint(int index) const
{
	return QSize();
}

QRect YaMultiLineTabBar::tabRect(int index) const
{
	if (index < 0 || index >= tabRect_.count()) {
		return QRect();
	}

	return tabRect_[index];
}

void YaMultiLineTabBar::tabLayoutChange()
{
	if (!layoutUpdatesEnabled())
		return;

	relayoutTabs();
	updateTabDrawOrder();

	updateSendButton();
}

int YaMultiLineTabBar::rowHeight() const
{
	return preferredTabSize("", false).height();
}

void YaMultiLineTabBar::relayoutTabs(bool ensureCurrentVisible)
{
	clearCaches();
	tabRect_.clear();

	static const int maxVisibleRows = 3;

	int multilineWidth = (maximumWidth() - buttonMargin);
	int numRows = (minimumTabWidth() * count()) / multilineWidth;
	if ((minimumTabWidth() * count()) <= multilineWidth) {
		numRows = 0;
	}

	int tabWidth = minimumTabWidth();
	if (numRows < 1 && count()) {
		tabWidth = qMax(minimumTabWidth(), qMin(maximumTabWidth(), maximumWidth() / count()));
	}

	int realRows = 0;
	int currentRow = -1;
	int x = 0;
	int y = 0;
	int numTabsInRow = 0;
	for (int i = 0; i < count(); ++i) {
		if (numRows && (x + tabWidth) > multilineWidth) {
			x = 0;
			y += rowHeight();
			realRows++;
		}

		QRect r(x, y, tabWidth, rowHeight());
		tabRect_ << r;

		if (currentIndex() == i) {
			currentRow = realRows;
		}

		if (!y) {
			numTabsInRow++;
		}

		x += tabWidth;
	}

	bool scrollVisible = false;
	int maxScrollBarRange = 0;
	if (realRows >= maxVisibleRows) {
		scrollVisible = true;
		rowOffset_ = qMax(0, qMin(realRows - maxVisibleRows + 1, rowOffset_));
		maxScrollBarRange = realRows - maxVisibleRows + 1;

		if (ensureCurrentVisible) {
			rowOffset_ = qMax(0, qMin(currentRow, rowOffset_));
			if (rowOffset_ < currentRow - maxVisibleRows + 1) {
				rowOffset_ = currentRow - maxVisibleRows + 1;
			}
		}

		int shift = rowOffset_; // realRows - maxVisibleRows + 1;
		while (shift-- > 0) {
			for (int i = 0; i < count(); ++i) {
				tabRect_[i].translate(0, -tabRect_[i].height());
			}
		}

		realRows = maxVisibleRows - 1;
	}

	emptySpace_ = QRect();
	if (realRows && numTabsInRow) {
		int fullWidth = scrollVisible ? multilineWidth : maximumWidth();
		tabWidth = qMax(minimumTabWidth(), qMin(maximumTabWidth(), fullWidth / numTabsInRow));
		// emptySpace_.setRight(numTabsInRow * tabWidth - 1);

		for (int i = 0; i < count(); ++i) {
			if (tabRect_[i].left()) {
				tabRect_[i].moveLeft(tabRect_[i-1].right() + 1);
			}

			if (tabRect_[i].left() + tabWidth + 10 >= fullWidth) {
				tabRect_[i].setWidth(fullWidth - tabRect_[i].left());
			}
			else {
				tabRect_[i].setWidth(tabWidth);
			}
		}
	}
	else {
		scrollVisible = false;
		Q_ASSERT(!scrollVisible);
		for (int i = 0; i < count(); ++i) {
			if (tabRect_[i].left() + tabWidth + 10 >= maximumWidth()) {
				tabRect_[i].setWidth(maximumWidth() - tabRect_[i].left());
			}
		}
	}

	if (count()) {
		emptySpace_ = tabRect(count() - 1);
		emptySpace_.setLeft(emptySpace_.right() + 1);
		emptySpace_.setRight(rect().right());
	}

	bool doUpdateGeometry = numRows_ != realRows;
	numRows_ = realRows;
	if (doUpdateGeometry) {
		emit updateGeometry();
	}

	if (scrollVisible) {
		QRect buttonRect(width() - buttonMargin, 0, buttonMargin, maxVisibleRows * rowHeight() - 9);
		scrollBar_->setGeometry(buttonRect);
		scrollBar_->setRange(0, maxScrollBarRange);
		scrollBar_->setValue(rowOffset_);

		bool flashUp = false;
		bool flashDown = false;
		for (int i = 0; i < count(); ++i) {
			if (tabData(i).toBool()) {
				if (tabRect_[i].top() < 0)
					flashUp = true;
				if (tabRect_[i].top() >= height())
					flashDown = true;
			}
		}
		scrollBar_->setFlashUp(flashUp);
		scrollBar_->setFlashDown(flashDown);

		QRect r(buttonRect);
		r.setHeight(r.height() / 2);
		scrollUp_->setGeometry(r);
		r.moveTop(r.top() + r.height());
		scrollDown_->setGeometry(r);
	}
	scrollBar_->setVisible(scrollVisible);

	if (closeFrame_.count() != tabRect_.count()) {
		clearCloseButtonFrame();
	}
}

int YaMultiLineTabBar::minimumTabWidth() const
{
	return preferredTabSize("nnnnnn", true).width();
}

int YaMultiLineTabBar::maximumTabWidth() const
{
	return preferredTabSize("nnnnnnnnnnnnnnnnnn", true).width();
}

QSize YaMultiLineTabBar::preferredTabSize(const QString& text, bool current) const
{
	QSize sh;
	sh.setHeight(qMax(cachedTextHeight(), cachedIconSize().height()));
	sh += QSize(0, margin() * 2);

	sh.setWidth(0);
	sh += QSize(margin(), 0);
	sh += QSize(cachedIconSize().width(), 0);
	sh += QSize(margin(), 0);
	int textWidth = YaTabBarBase::textWidth(text);
	textWidth = qMax(textWidth, YaTabBarBase::textWidth("nnn"));
	sh += QSize(textWidth, 0);
	sh += QSize(margin(), 0);

	if (current) {
		sh += QSize(1, 0);
		sh += QSize(closePixmaps_.last().width(), 0);
		sh += QSize(margin(), 0);
	}

	return sh;
}

QRect YaMultiLineTabBar::closeButtonRect(int index, const QRect& currentTabRect) const
{
	if (index != currentIndex() && !tabHovered(index))
		return QRect();

	QRect result(currentTabRect);
	result.setLeft(result.right() /*- sizeGripMargin*/ - margin() - 1 - closePixmaps_.last().width());
	result.setSize(closePixmaps_.last().size());
	result.moveCenter(QPoint(result.center().x(), currentTabRect.center().y()));
	return result;
}

void YaMultiLineTabBar::drawTab(QPainter* painter, int index, const QRect& tabRect)
{
	YaTabBarBase::drawTab(painter, index, tabRect);

	QRect closeRect = closeButtonRect(index, tabRect);
	if (!closeRect.isNull()) {
		Q_ASSERT(index >= 0 && index < closeFrame_.size());
		const QPixmap& pix = closePixmaps_[closeFrame_[index]];
		painter->drawPixmap(closeRect.topLeft() +
		                    QPoint((closeRect.width() - pix.width()) / 2,
		                           (closeRect.height() - pix.height()) / 2),
		                    pix);
	}
}

bool YaMultiLineTabBar::isMultiLine() const
{
	return true;
}

void YaMultiLineTabBar::currentIndexChanged()
{
	relayoutTabs(true);
	YaTabBarBase::currentIndexChanged();
}

/**
 * We want to use wheel to scroll the scrollbar, not to select
 * previous / next tabs.
 */
void YaMultiLineTabBar::wheelEvent(QWheelEvent* event)
{
	// YaTabBarBase::wheelEvent(event);
	event->accept();

	int numDegrees = event->delta() / 8;
	int numSteps = numDegrees / 15;

	rowOffset_ -= numSteps;
	tabLayoutChange();
	update();
}

void YaMultiLineTabBar::enterEvent(QEvent* event)
{
	YaTabBarBase::enterEvent(event);
	closeButtonAnimationTimer_->start();
}

void YaMultiLineTabBar::leaveEvent(QEvent* event)
{
	YaTabBarBase::leaveEvent(event);
	closeButtonAnimationTimer_->stop();
	clearCloseButtonFrame();
	update();
}

void YaMultiLineTabBar::clearCloseButtonFrame()
{
	closeFrame_ = QVector<int>(tabRect_.count(), 0);
	unsetCursor();
}

void YaMultiLineTabBar::closeButtonAnimation()
{
	bool doUpdate = false;
	bool closeHovered = false;

	for (int i = 0; i < closeFrame_.count(); ++i) {
		QRect closeRect = closeButtonRect(i, tabRect(i));
		int val = closeFrame_[i];
		if (closeRect.contains(mapFromGlobal(QCursor::pos())) && draggedTabIndex_ == -1) {
			++val;
			closeHovered = true;
		}
		else {
			--val;
		}
		int newVal = qMax(0, qMin(val, closePixmaps_.count() - 1));
		if (newVal != closeFrame_[i]) {
			closeFrame_[i] = newVal;
			doUpdate = true;
		}
	}

	if (closeHovered) {
		setCursor(Qt::PointingHandCursor);
	}
	else {
		unsetCursor();
	}

	if (doUpdate) {
		update();
	}
}

void YaMultiLineTabBar::mousePressEvent(QMouseEvent* event)
{
	YaTabBarBase::mousePressEvent(event);
	autoScrollCount_ = 0;

	closeButtonHit_ = false;
	for (int i = 0; i < closeFrame_.count(); ++i) {
		if (closeButtonRect(i, tabRect(i)).contains(pressedPosition())) {
			closeButtonHit_ = true;
			break;
		}
	}
}

void YaMultiLineTabBar::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		if (autoScrollToBottom() || autoScrollToTop()) {
			autoScrollTimer_->start();
		}
		else {
			autoScrollTimer_->stop();
		}
	}

	if (draggedTabIndex_ != -1) {
		dragPosition_ = event->pos();
		{
			if (dragPosition_.x() < 0)
				dragPosition_.setX(0);
			if (scrollBar_->isVisible()) {
				if (dragPosition_.x() > width() - buttonMargin - 1)
					dragPosition_.setX(width() - buttonMargin - 1);
			}
			else {
				if (dragPosition_.x() > width() - 1)
					dragPosition_.setX(width() - 1);
			}
			if (dragPosition_.y() < 0)
				dragPosition_.setY(0);
			if (dragPosition_.y() > height() - 1)
				dragPosition_.setY(height() - 1);
		}
		updateTabDrawOrder();
		update();
		return;
	}

	if (!pressedPosition().isNull() &&
	    (pressedPosition() - event->pos()).manhattanLength() > QApplication::startDragDistance() &&
	    !closeButtonHit_)
	{
		dragPosition_ = event->pos();
		startDrag();
		return;
	}

	YaTabBarBase::mouseMoveEvent(event);

	update();
}

void YaMultiLineTabBar::mouseReleaseEvent(QMouseEvent* event)
{
	QPoint pos = pressedPosition();
	clearPressedPosition();
	autoScrollTimer_->stop();

	bool reorderedTabs = false;

	if (draggedTabIndex_ != -1 && dragDestinationIndex_ != -1) {
		TabbableWidget* tab = currentTab();
		emit reorderTabs(draggedTabIndex_, dragDestinationIndex_);
		setCurrentIndex(indexOf(tab));
		relayoutTabs(false);
		reorderedTabs = true;
	}

	draggedTabIndex_ = -1;
	dragDestinationIndex_ = -1;

	for (int i = 0; i < count(); ++i) {
		QRect tabRect = this->tabRect(i);
		QRect closeRect = closeButtonRect(i, tabRect);

		if (closeRect.contains(event->pos()) && closeRect.contains(pos)) {
			LOG_TRACE;
			emit closeTab(i);
			return;
		}

		// don't activate inactive tabs when we've clicked on close button
		// and moved the mouse away, but stayed on the same tab area
		if (closeRect.contains(pos)) {
			return;
		}

		// otherwise we could activate tab which weren't clicked in the first place
		if (tabRect.contains(pos) && tabRect.contains(event->pos()) && !reorderedTabs) {
			YaTabBarBase::mouseReleaseEvent(event);
			return;
		}
	}
}

void YaMultiLineTabBar::updateTabDrawOrder()
{
	tabDrawOrder_.clear();
	for (int i = 0; i < count(); ++i) {
		Q_ASSERT(i < tabRect_.count());

		if (i != draggedTabIndex_)
			tabDrawOrder_ << i;
	}

	dragEmptySpaceRect_ = QRect();
	if (draggedTabIndex_ != -1 && draggedTabIndex_ < tabRect_.count()) {
		tabDrawOrder_.append(draggedTabIndex_);

		QRect draggedTabRect;
		if (draggedTabIndex_ >= 0 && draggedTabIndex_ < tabRect_.count()) {
			QRect r = tabRect(draggedTabIndex_);
			r.moveTopLeft(dragPosition_ - dragOffset_);
			if (r.left() < 0)
				r.moveLeft(0);
			if (scrollBar_->isVisible()) {
				if (r.right() > width() - buttonMargin - 1)
					r.moveRight(width() - buttonMargin - 1);
			}
			else {
				if (r.right() > width() - 1)
					r.moveRight(width() - 1);
			}
			if (r.top() < 0)
				r.moveTop(0);
			if (r.bottom() > height() - 1)
				r.moveBottom(height() - 1);
			draggedTabRect = r;
		}

		{
			relayoutTabs(false);

			QList<QRect> oldTabRect = tabRect_;

			int copyRectFrom = 0;
			for (int i = 0; i < count(); ++i) {
				if (oldTabRect[i].contains(dragPosition_)) {
					dragDestinationIndex_ = i;
					dragEmptySpaceRect_ = oldTabRect[i];

					if (i <= draggedTabIndex_)
						copyRectFrom += 1;
				}

				if (i == draggedTabIndex_) {
					continue;
				}

				if (copyRectFrom >= count())
					break;
				int index = qMin(count() - 1, qMax(0, copyRectFrom));
				tabRect_[i] = oldTabRect[index];
				copyRectFrom++;

				if (oldTabRect[i].contains(dragPosition_)) {
					if (i > draggedTabIndex_)
						copyRectFrom += 1;
				}
			}

			if (dragEmptySpaceRect_.isNull()) {
				dragEmptySpaceRect_ = oldTabRect.last();
				dragDestinationIndex_ = oldTabRect.count() - 1;
			}
		}

		tabRect_[draggedTabIndex_] = draggedTabRect;
	}
}

void YaMultiLineTabBar::startDrag()
{
	draggedTabIndex_ = tabAt(pressedPosition());
	if (draggedTabIndex_ != -1) {
		dragOffset_ = pressedPosition() - tabRect(draggedTabIndex_).topLeft();
		updateTabDrawOrder();
		update();
	}
	else {
		draggedTabIndex_ = -1;
		clearPressedPosition();
	}
}

static const int AUTO_SCROLL_MARGIN = 5;

bool YaMultiLineTabBar::autoScrollToBottom() const
{
	return !pressedPosition().isNull() &&
	       dragPosition_.y() > (rect().bottom() - AUTO_SCROLL_MARGIN);
}

bool YaMultiLineTabBar::autoScrollToTop() const
{
	return !pressedPosition().isNull() &&
	       dragPosition_.y() < (rect().top() + AUTO_SCROLL_MARGIN);
}

void YaMultiLineTabBar::autoScroll()
{
	if (!autoScrollToBottom() && !autoScrollToTop()) {
		autoScrollTimer_->stop();
		return;
	}

	if (autoScrollCount_ >= 0 && autoScrollToBottom()) {
		autoScrollCount_ = qMin(autoScrollCount_ + 1, scrollBar_->pageStep());
	}
	else if (autoScrollCount_ <= 0 && autoScrollToTop()) {
		autoScrollCount_ = -qMin(qAbs(autoScrollCount_ - 1), scrollBar_->pageStep());
	}
	else {
		autoScrollCount_ = 0;
	}

	int value = scrollBar_->value() + autoScrollCount_;

	SmoothScrollBar* smoothScrollBar = dynamic_cast<SmoothScrollBar*>(scrollBar_);
	if (smoothScrollBar)
		smoothScrollBar->setValueImmediately(value);
	else
		scrollBar_->setValue(value);
}

bool YaMultiLineTabBar::tabHovered(int index) const
{
	if (draggedTabIndex_ == index)
		return true;

	return YaTabBarBase::tabHovered(index);
}
