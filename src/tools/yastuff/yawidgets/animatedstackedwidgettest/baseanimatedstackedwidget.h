/*
 * baseanimatedstackedwidget.h
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

#ifndef BASEANIMATEDSTACKEDWIDGET_H
#define BASEANIMATEDSTACKEDWIDGET_H

#include <QFrame>

class QTimeLine;
class QPainter;
class QStackedWidget;

#if QT_VERSION >= 0x040300
#	define ENABLE_YATOOLBOX_ANIMATION
#endif

class BaseAnimatedStackedWidgetAnimation : public QWidget
{
	Q_OBJECT
public:
	BaseAnimatedStackedWidgetAnimation();
	~BaseAnimatedStackedWidgetAnimation();

	void setStaticPixmap(const QPixmap& pix);
	void start();

signals:
	void finished();

protected:
	QTimeLine* animationTimeLine() const;
	// virtual int animationSize() const = 0;
	virtual void animationPaint(QPainter* p) = 0;

private:
	// reimplemented
	void paintEvent(QPaintEvent*);

private slots:
	void frameChanged(int);

private:
	QTimeLine* animationTimeLine_;
	QPixmap staticPixmap_;
};

class BaseAnimatedStackedWidget : public QFrame
{
	Q_OBJECT
public:
	BaseAnimatedStackedWidget(QWidget* parent);
	~BaseAnimatedStackedWidget();

	bool animating() const;

	virtual void init();

protected:
	BaseAnimatedStackedWidgetAnimation* animationPage() const;
	virtual void setCurrentWidget(QWidget* widget);

	virtual QWidget* normalPage() const = 0;
	virtual BaseAnimatedStackedWidgetAnimation* createAnimationPage() const = 0;

	virtual void setCurrentState(QWidget* widget) = 0;
	virtual void setNewState() = 0;

	virtual void setCurrentWidget_internal(QWidget* widget) = 0;
	virtual void activateNormalPageLayout() = 0;

private slots:
	void animationFinished();

private:
	QStackedWidget* stackedWidget_;
	BaseAnimatedStackedWidgetAnimation* animationPage_;
};

#endif
