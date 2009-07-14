/*
 * animatedstackedwidget.h - stacked widget that does nifty animations
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

#ifndef ANIMATEDSTACKEDWIDGET_H
#define ANIMATEDSTACKEDWIDGET_H

#include "baseanimatedstackedwidget.h"

#include <QMap>

class AnimatedStackedWidgetAnimation : public BaseAnimatedStackedWidgetAnimation
{
	Q_OBJECT
public:
	AnimatedStackedWidgetAnimation();
	~AnimatedStackedWidgetAnimation();

	enum State {
		State_Current = 0,
		State_New     = 1
	};

	enum Type {
		MoveInBottomToTop,
		MoveInTopToBottom,
		MoveOutTopToBottom,

		PushLeftToRight,
		PushRightToLeft
	};

	Type type() const;
	void setType(Type type);

	void setState(State state, QWidget* widget);

private:
	QSize fullSize_;
	Type type_;
	QPixmap states_[2];

	int animationSize() const;

	// reimplemented
	void animationPaint(QPainter* p);
};

class AnimatedStackedWidget : public BaseAnimatedStackedWidget
{
	Q_OBJECT
public:
	AnimatedStackedWidget(QWidget* parent = 0);
	~AnimatedStackedWidget();

	enum AnimationStyle {
		Animation_Default = 0,
		Animation_Push_Horizontal
	};

	AnimationStyle animationStyle() const;
	void setAnimationStyle(AnimationStyle animationStyle);

	int widgetPriority(const QWidget* widget) const;
	void setWidgetPriority(const QWidget* widget, int priority);

	QWidget* currentWidget() const;
	void setCurrentIndex(int index);
	void addWidget(QWidget* widget);
	void setCurrentWidget(QWidget* widget);

	QWidget* mainWidget() const;
	void setMainWidget(QWidget* mainWidget);

protected:
	// reimplemented
	virtual QWidget* normalPage() const;
	virtual BaseAnimatedStackedWidgetAnimation* createAnimationPage() const;
	virtual void setCurrentState(QWidget* widget);
	virtual void setNewState();
	virtual void setCurrentWidget_internal(QWidget* widget);
	virtual void activateNormalPageLayout();

private:
	QMap<const QWidget*, int> widgetPriority_;
	AnimationStyle animationStyle_;
	QStackedWidget* normalPage_;
	QWidget* mainWidget_;
	QWidget* prevWidget_;
};

#endif
