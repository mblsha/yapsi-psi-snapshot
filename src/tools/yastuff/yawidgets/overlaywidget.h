/*
 * overlaywidget.h - base class for doing overlay widgets
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

#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QTimer>
#include <QPointer>

class GOverlayWidget : public QObject
{
	Q_OBJECT
public:
	GOverlayWidget(QWidget* parent, QObject* extra)
		: QObject(parent)
	{
		delayedInvalidateExtraPosition();
		if (extra->metaObject()->indexOfSignal("invalidateExtraPosition()") != -1)
			connect(extra, SIGNAL(invalidateExtraPosition()), SLOT(invalidateExtraPosition()));
	}

	void delayedInvalidateExtraPosition()
	{
		QTimer::singleShot(0, this, SLOT(invalidateExtraPosition()));
	}

private slots:
	void invalidateExtraPosition()
	{
		QWidget* widget = static_cast<QWidget*>(parent());
		QResizeEvent event(widget->size(), widget->size());
		QCoreApplication::sendEvent(widget, &event);
	}
};

template <class BaseClass, class ExtraClass>
class OverlayWidget : public BaseClass
{
public:
	OverlayWidget(QWidget* parent, ExtraClass* extra, Qt::WindowFlags f = 0)
		: BaseClass(parent)
		, extra_(extra)
		, gOverlayWidget_(new GOverlayWidget(this, extra))
	{
		BaseClass::setWindowFlags(f);
		extra_->hide();

#if 0
		if (BaseClass::window() != this) {
			BaseClass::window()->installEventFilter(this);
		}
#endif
	}

	~OverlayWidget()
	{
		if (!extra_.isNull()) {
			delete extra_;
		}
	}

	void raiseExtraInWidgetStack()
	{
		extra()->raise();
	}

protected:
	// reimplemented
	void resizeEvent(QResizeEvent *e)
	{
		BaseClass::resizeEvent(e);
		moveExtra();
	}

	// reimplemented
	void moveEvent(QMoveEvent *e)
	{
		BaseClass::moveEvent(e);
		moveExtra();
		gOverlayWidget_->delayedInvalidateExtraPosition();
	}

	// reimplemented
	void showEvent(QShowEvent *e)
	{
		BaseClass::showEvent(e);
		extra()->setVisible(extraShouldBeVisible());
		raiseExtraInWidgetStack();
		moveExtra();
		gOverlayWidget_->delayedInvalidateExtraPosition();
	}

	// reimplemented
	void hideEvent(QHideEvent *e)
	{
		BaseClass::hideEvent(e);
		extra()->hide();
	}

	/**
	 * We're INVISIBLE!!
	 */
	// reimplemented
	void paintEvent(QPaintEvent*)
	{
	}

#if 0
	// reimplemented
	void changeEvent(QEvent* e)
	{
		BaseClass::changeEvent(e);

		if (e->type() == QEvent::ParentChange && BaseClass::window() != this) {
			BaseClass::window()->installEventFilter(this);
			gOverlayWidget_->delayedInvalidateExtraPosition();
		}
	}
#endif

protected:
	ExtraClass* extra() const
	{
		return extra_;
	}

	QRect globalRect(const QWidget* widget, QRect rect) const
	{
		const QWidget* parent = widget->parentWidget();
		parent = parent ? parent : widget;
		return QRect(parent->mapToGlobal(rect.topLeft()),
		             parent->mapToGlobal(rect.bottomRight()));
	}

	QRect globalRect(QRect rect) const
	{
		return globalRect(this, rect);
	}

	QRect globalRect() const
	{
		if (BaseClass::window() == this)
			return globalRect(BaseClass::rect());
		return globalRect(BaseClass::geometry());
	}

	void moveExtra()
	{
		QRect global = extraGeometry();
		const QWidget* parent = extra()->parentWidget();
		parent = parent ? parent : this;

		// FIXME: the following construction sometimes results in coordinates
		// increasted by QSize(4, 30) on Windows XP (size of the window's border)
		QRect local = QRect(parent->mapFromGlobal(global.topLeft()),
		                    parent->mapFromGlobal(global.bottomRight()));
		extra()->setGeometry(local);
	}

#if 0
	// reimplemented
	bool eventFilter(QObject* obj, QEvent* e)
	{
		// in future we should do something more reliable, like monitoring
		// move/resize events of all the parent widgets. should be extremely
		// helpful in case of YaChatSendButtonExtra
		if (e->type() == QEvent::ParentChange && BaseClass::window() != this) {
			BaseClass::window()->installEventFilter(this);
			gOverlayWidget_->delayedInvalidateExtraPosition();
		}

		if (obj == BaseClass::window() && BaseClass::window() != this && e->type() == QEvent::Resize) {
			gOverlayWidget_->delayedInvalidateExtraPosition();
		}

		return BaseClass::eventFilter(obj, e);
	}
#endif

	GOverlayWidget* gOverlayWidget() const
	{
		return gOverlayWidget_;
	}

private:
	/**
	 * Geometry of an \a extra(). Must be returned in global screen
	 * coordinates. Please make use of globalRect() functions.
	 */
	virtual QRect extraGeometry() const = 0;

	/**
	 * This function should return true if \a extra() should be
	 * visible, and false otherwise.
	 */
	virtual bool extraShouldBeVisible() const
	{
		return true;
	}

private:
	QPointer<ExtraClass> extra_;
	GOverlayWidget* gOverlayWidget_;
};

#endif
