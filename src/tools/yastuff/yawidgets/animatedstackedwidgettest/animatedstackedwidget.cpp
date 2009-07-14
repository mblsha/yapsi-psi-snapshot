/*
 * animatedstackedwidget.cpp - stacked widget that does nifty animations
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

#include "animatedstackedwidget.h"

#include <QTimeLine>
#include <QPainter>
#include <QStackedWidget>
#include <QLayout>

//----------------------------------------------------------------------------
// AnimatedStackedWidgetAnimation
//----------------------------------------------------------------------------

AnimatedStackedWidgetAnimation::AnimatedStackedWidgetAnimation()
	: BaseAnimatedStackedWidgetAnimation()
	, type_(MoveInBottomToTop)
{
}

AnimatedStackedWidgetAnimation::~AnimatedStackedWidgetAnimation()
{
}

AnimatedStackedWidgetAnimation::Type AnimatedStackedWidgetAnimation::type() const
{
	return type_;
}

void AnimatedStackedWidgetAnimation::setType(AnimatedStackedWidgetAnimation::Type type)
{
	type_ = type;
}

void AnimatedStackedWidgetAnimation::setState(State state, QWidget* widget)
{
	if (state == State_Current) {
		fullSize_ = widget->size();
	}
	else {
		widget->resize(fullSize_);
		widget->layout()->invalidate();
		widget->layout()->activate();
		foreach(QWidget* w, widget->findChildren<QWidget*>()) {
			if (w->layout()) {
				w->layout()->invalidate();
				w->layout()->activate();
			}
		}
	}

	QPixmap pix(widget->size());
	widget->render(&pix);
	states_[state] = pix;

	animationTimeLine()->setFrameRange(0, animationSize());
}

int AnimatedStackedWidgetAnimation::animationSize() const
{
	switch (type_) {
	case MoveInBottomToTop:
	case MoveInTopToBottom:
	case MoveOutTopToBottom:
		return states_[0].height();
	case PushLeftToRight:
	case PushRightToLeft:
		return states_[0].width();
	default:
		Q_ASSERT(false);
	}

	return 0;
}

void AnimatedStackedWidgetAnimation::animationPaint(QPainter* p)
{
	int ox = 0;
	int oy = 0;
	int nx = 0;
	int ny = 0;
	switch (type_) {
	case MoveInBottomToTop:
		ny = height() - animationTimeLine()->currentFrame();
		break;
	case MoveInTopToBottom:
		ny = animationTimeLine()->currentFrame() - height();
		break;
	case MoveOutTopToBottom:
		ny = animationTimeLine()->currentFrame();
		break;
	case PushLeftToRight:
		ox = animationTimeLine()->currentFrame();
		nx = animationTimeLine()->currentFrame() - animationSize();
		break;
	case PushRightToLeft:
		ox = -animationTimeLine()->currentFrame();
		nx = animationSize() - animationTimeLine()->currentFrame();
		break;
	default:
		Q_ASSERT(false);
	}
	p->drawPixmap(ox, oy, states_[State_Current]);
	p->drawPixmap(nx, ny, states_[State_New]);
}

//----------------------------------------------------------------------------
// AnimatedStackedWidget
//----------------------------------------------------------------------------

AnimatedStackedWidget::AnimatedStackedWidget(QWidget* parent)
	: BaseAnimatedStackedWidget(parent)
	, animationStyle_(Animation_Default)
	, mainWidget_(0)
	, prevWidget_(0)
{
	normalPage_ = new QStackedWidget();
}

AnimatedStackedWidget::~AnimatedStackedWidget()
{
}

QWidget* AnimatedStackedWidget::normalPage() const
{
	return normalPage_;
}

BaseAnimatedStackedWidgetAnimation* AnimatedStackedWidget::createAnimationPage() const
{
	return new AnimatedStackedWidgetAnimation();
}

void AnimatedStackedWidget::setCurrentState(QWidget* widget)
{
	AnimatedStackedWidgetAnimation* anim = static_cast<AnimatedStackedWidgetAnimation*>(animationPage());
	QWidget* w = normalPage_->currentWidget();

	switch (animationStyle_) {
	case Animation_Default:
		if (mainWidget_) {
			prevWidget_ = (w == mainWidget_) ? widget : w;
			w = mainWidget_;
		}
		break;
	case Animation_Push_Horizontal:
		anim->setType(widgetPriority(widget) < widgetPriority(w) ?
		              AnimatedStackedWidgetAnimation::PushLeftToRight :
		              AnimatedStackedWidgetAnimation::PushRightToLeft);
	}

	anim->setState(AnimatedStackedWidgetAnimation::State_Current, w);
}

void AnimatedStackedWidget::setNewState()
{
	AnimatedStackedWidgetAnimation* anim = static_cast<AnimatedStackedWidgetAnimation*>(animationPage());
	QWidget* w = normalPage_->currentWidget();

	switch (animationStyle_) {
	case Animation_Default:
		if (mainWidget_) {
			anim->setType(mainWidget_ == normalPage_->currentWidget() ?
			              AnimatedStackedWidgetAnimation::MoveOutTopToBottom :
			              AnimatedStackedWidgetAnimation::MoveInBottomToTop);

			w = prevWidget_;
		}
	case Animation_Push_Horizontal:
		;
	}

	anim->setState(AnimatedStackedWidgetAnimation::State_New, w);
}

void AnimatedStackedWidget::setCurrentWidget_internal(QWidget* widget)
{
	normalPage_->setCurrentWidget(widget);
}

void AnimatedStackedWidget::activateNormalPageLayout()
{
}

void AnimatedStackedWidget::setCurrentIndex(int index)
{
	setCurrentWidget(normalPage_->widget(index));
}

void AnimatedStackedWidget::addWidget(QWidget* widget)
{
	widget->setAutoFillBackground(true);
	normalPage_->addWidget(widget);
}

QWidget* AnimatedStackedWidget::currentWidget() const
{
	return normalPage_->currentWidget();
}

void AnimatedStackedWidget::setCurrentWidget(QWidget* widget)
{
	if (normalPage_->currentWidget() == widget)
		return;
	BaseAnimatedStackedWidget::setCurrentWidget(widget);
}

QWidget* AnimatedStackedWidget::mainWidget() const
{
	return mainWidget_;
}

void AnimatedStackedWidget::setMainWidget(QWidget* mainWidget)
{
	mainWidget_ = mainWidget;
}

AnimatedStackedWidget::AnimationStyle AnimatedStackedWidget::animationStyle() const
{
	return animationStyle_;
}

void AnimatedStackedWidget::setAnimationStyle(AnimatedStackedWidget::AnimationStyle animationStyle)
{
	animationStyle_ = animationStyle;
}

int AnimatedStackedWidget::widgetPriority(const QWidget* widget) const
{
	Q_ASSERT(widget);
	if (widgetPriority_.contains(widget))
		return widgetPriority_[widget];
	return -1;
}

void AnimatedStackedWidget::setWidgetPriority(const QWidget* widget, int priority)
{
	Q_ASSERT(widget);
	Q_ASSERT(priority != -1);
	widgetPriority_[widget] = priority;
}

