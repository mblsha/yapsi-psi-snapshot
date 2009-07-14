/*
 * smoothscrollbar.cpp - a QScrollBar with smooth scrolling
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

#include "smoothscrollbar.h"

#include <QAbstractScrollArea>
#include <QTimeLine>
#ifdef YAPSI
#include <QStyle>
#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QCursor>
#endif

static const int duration = 100;
static const int updateInterval = 10;

void SmoothScrollBar::install(QAbstractScrollArea* scrollArea, Mode mode)
{
	if (dynamic_cast<SmoothScrollBar*>(scrollArea->verticalScrollBar()))
		return;

	SmoothScrollBar* newBar = new SmoothScrollBar(0);
	newBar->setPreferScrollToBottom(mode == PreferScrollToBottom);
	QRect geometry = scrollArea->verticalScrollBar()->geometry();
	scrollArea->setVerticalScrollBar(newBar);
	newBar->setGeometry(geometry);
	newBar->show();
}

SmoothScrollBar::SmoothScrollBar(QWidget* parent)
	: QScrollBar(parent)
	, timeLine_(new QTimeLine(duration, this))
	, preferScrollToBottom_(false)
	, enableSmoothScrolling_(true)
	, rangeUpdatesEnabled_(true)
	, savedMin_(-1)
	, savedMax_(-1)
{
#ifdef YAPSI
	setMouseTracking(true);
#endif

	timeLine_->setCurveShape(QTimeLine::EaseOutCurve);
	timeLine_->setUpdateInterval(updateInterval);
	connect(timeLine_, SIGNAL(frameChanged(int)), SLOT(timeLineFrameChanged(int)));
}

/**
 * Qt needs to be patched in order to contain virtual method:
 * virtual void QAbstractSlider::setValue(int), otherwise
 * all our magic is powerless.
 */
void SmoothScrollBar::setValue(int val)
{
	// immediately set value in case the scrollbar is already
	// at maximum and we are reducing the maximum value,
	// as in case of widget resize
	int bound = boundValue(val);
	if (bound != val || isSliderDown() || !enableSmoothScrolling_) {
		timeLine_->stop();
		internalSetValue(bound);
		return;
	}

	if (value() == bound)
		return;

	timeLine_->setFrameRange(value(), val);
	if (timeLine_->state() != QTimeLine::Running)
		timeLine_->start();
}

void SmoothScrollBar::setValueImmediately(int val)
{
	timeLine_->setFrameRange(val, val);
	internalSetValue(val);
}

void SmoothScrollBar::timeLineFrameChanged(int val)
{
	internalSetValue(val);
}

bool SmoothScrollBar::preferScrollToBottom() const
{
	return preferScrollToBottom_;
}

void SmoothScrollBar::setPreferScrollToBottom(bool enable)
{
	preferScrollToBottom_ = enable;
}

/**
 * Qt needs to be patched in order to contain virtual method:
 * virtual void QAbstractSlider::setRange(int, int), otherwise
 * all our magic is powerless.
 */
void SmoothScrollBar::setRange(int min, int max)
{
	savedMin_ = min;
	savedMax_ = max;

	if (min == minimum() && max == maximum())
		return;

	if (!rangeUpdatesEnabled()) {
		return;
	}

	bool atEnd   = value() == maximum();
	bool atStart = value() == minimum();
	double val = 0;
	if (!atEnd && !atStart)
		val = (double)maximum() / (double)value();

	QScrollBar::setRange(min, max);

	if (atEnd && preferScrollToBottom())
		setValueImmediately(maximum());
	else if (val != 0)
		setValueImmediately((int) ((double)maximum() / val));
}

bool SmoothScrollBar::enableSmoothScrolling() const
{
	return enableSmoothScrolling_;
}

void SmoothScrollBar::setEnableSmoothScrolling(bool enable)
{
	enableSmoothScrolling_ = enable;
}

bool SmoothScrollBar::rangeUpdatesEnabled() const
{
	return rangeUpdatesEnabled_;
}

void SmoothScrollBar::setRangeUpdatesEnabled(bool rangeUpdatesEnabled)
{
	if (rangeUpdatesEnabled_ != rangeUpdatesEnabled) {
		rangeUpdatesEnabled_ = rangeUpdatesEnabled;
		if (rangeUpdatesEnabled_) {
			setRange(savedMin_, savedMax_);
		}
		else {
			savedMin_ = minimum();
			savedMax_ = maximum();
		}
	}
}

void SmoothScrollBar::internalSetValue(int value)
{
	updateCursor();
	QScrollBar::setValue(value);
}

#ifdef YAPSI
void SmoothScrollBar::mouseMoveEvent(QMouseEvent* event)
{
	updateCursor();
	QScrollBar::mouseMoveEvent(event);
}

void SmoothScrollBar::updateCursor()
{
	QList<QStyle::SubControl> pointingHandControls;
	pointingHandControls
		<< QStyle::SC_ScrollBarAddLine
		<< QStyle::SC_ScrollBarSubLine
		<< QStyle::SC_ScrollBarSlider;
	bool found = false;
	QStyleOptionSlider option;
	initStyleOption(&option);

	QPoint pos = mapFromGlobal(QCursor::pos());
	foreach(const QStyle::SubControl& sc, pointingHandControls) {
		if (style()->subControlRect(QStyle::CC_ScrollBar, &option, sc, this).contains(pos)) {
			found = true;
			break;
		}
	}

	if (found)
		setCursor(Qt::PointingHandCursor);
	else
		unsetCursor();
}
#endif
