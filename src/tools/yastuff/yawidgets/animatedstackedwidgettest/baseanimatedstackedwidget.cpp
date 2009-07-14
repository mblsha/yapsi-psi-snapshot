/*
 * baseanimatedstackedwidget.cpp
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

#include "baseanimatedstackedwidget.h"

#include <QTimeLine>
#include <QPainter>
#include <QStackedWidget>
#include <QVBoxLayout>

//----------------------------------------------------------------------------
// BaseAnimatedStackedWidgetAnimation
//----------------------------------------------------------------------------

static const int animationDuration = 500;

BaseAnimatedStackedWidgetAnimation::BaseAnimatedStackedWidgetAnimation()
	: QWidget()
	, animationTimeLine_(0)
{
	animationTimeLine_ = new QTimeLine(animationDuration, this);
	connect(animationTimeLine_, SIGNAL(frameChanged(int)), SLOT(frameChanged(int)));
	connect(animationTimeLine_, SIGNAL(finished()), SIGNAL(finished()));
}

BaseAnimatedStackedWidgetAnimation::~BaseAnimatedStackedWidgetAnimation()
{
}

QTimeLine* BaseAnimatedStackedWidgetAnimation::animationTimeLine() const
{
	return animationTimeLine_;
}

void BaseAnimatedStackedWidgetAnimation::start()
{
	if (animationTimeLine_->startFrame() == animationTimeLine_->endFrame()) {
		emit finished();
		return;
	}

	animationTimeLine_->stop();
	animationTimeLine_->start();
}

void BaseAnimatedStackedWidgetAnimation::setStaticPixmap(const QPixmap& pix)
{
	staticPixmap_ = pix;
}

void BaseAnimatedStackedWidgetAnimation::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	if (animationTimeLine_->state() == QTimeLine::Running) {
		animationPaint(&p);
	}
	else {
		p.drawPixmap(0, 0, staticPixmap_);
	}
}

void BaseAnimatedStackedWidgetAnimation::frameChanged(int)
{
	update();
}

//----------------------------------------------------------------------------
// BaseAnimatedStackedWidget
//----------------------------------------------------------------------------

BaseAnimatedStackedWidget::BaseAnimatedStackedWidget(QWidget* parent)
	: QFrame(parent)
	, stackedWidget_(0)
	, animationPage_(0)
{
	stackedWidget_ = new QStackedWidget(this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(stackedWidget_);
}

BaseAnimatedStackedWidget::~BaseAnimatedStackedWidget()
{
}

void BaseAnimatedStackedWidget::init()
{
	animationPage_ = createAnimationPage();
	connect(animationPage_, SIGNAL(finished()), SLOT(animationFinished()));

	stackedWidget_->addWidget(normalPage());
	stackedWidget_->addWidget(animationPage_);
	stackedWidget_->setCurrentWidget(normalPage());
}

BaseAnimatedStackedWidgetAnimation* BaseAnimatedStackedWidget::animationPage() const
{
	return animationPage_;
}

void BaseAnimatedStackedWidget::setCurrentWidget(QWidget* widget)
{
	if (!widget)
		return;

#ifdef ENABLE_YATOOLBOX_ANIMATION
	if (isVisible()) {
		QPixmap pix(normalPage()->size());
		normalPage()->render(&pix);
		animationPage_->setStaticPixmap(pix);
		stackedWidget_->setCurrentWidget(animationPage_);
		setCurrentState(widget);
	}
#endif

#ifndef ENABLE_YATOOLBOX_ANIMATION
	setUpdatesEnabled(false);
#endif
	setCurrentWidget_internal(widget);
#ifndef ENABLE_YATOOLBOX_ANIMATION
	if (isVisible()) {
		activateNormalPageLayout();
		setUpdatesEnabled(true);
	}
#endif

#ifdef ENABLE_YATOOLBOX_ANIMATION
	if (isVisible()) {
		setNewState();
		animationPage_->start();
	}
#endif
}

void BaseAnimatedStackedWidget::animationFinished()
{
	if (stackedWidget_->currentWidget() == normalPage())
		return;

	setUpdatesEnabled(false);
	activateNormalPageLayout();
	stackedWidget_->setCurrentWidget(normalPage());
	setUpdatesEnabled(true);
}

bool BaseAnimatedStackedWidget::animating() const
{
	return stackedWidget_->currentWidget() != normalPage();
}
