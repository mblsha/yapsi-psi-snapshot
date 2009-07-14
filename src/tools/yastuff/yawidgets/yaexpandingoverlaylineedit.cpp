/*
 * yaexpandingoverlaylineedit.cpp
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

#include "yaexpandingoverlaylineedit.h"

#include <QTimeLine>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>
#include <QPalette>

static const int expandDuration = 200;

YaExpandingOverlayLineEdit::YaExpandingOverlayLineEdit(QWidget* parent)
	: YaEmptyTextLineEdit(parent)
	, state_(Normal)
	, expandCollapseTimeLine_(0)
	, updateGeometryTimer_(0)
	, controller_(0)
{
	expandCollapseTimeLine_ = new QTimeLine(expandDuration, this);
	connect(expandCollapseTimeLine_, SIGNAL(frameChanged(int)), SLOT(animate()));

	updateGeometryTimer_ = new QTimer(this);
	updateGeometryTimer_->setSingleShot(true);
	updateGeometryTimer_->setInterval(0);
	connect(updateGeometryTimer_, SIGNAL(timeout()), SLOT(recalculateGeometry()));

	installEventFilter(this);
	hide();
}

YaExpandingOverlayLineEdit::~YaExpandingOverlayLineEdit()
{
}

void YaExpandingOverlayLineEdit::setVisible(bool visible)
{
	if (visible) {
		setState(Expanded); // Expanding
	}
	else {
		setState(Normal); // Collapsing
	}

	// if enabled, slows down very first expansion operation
	// if (visible) {
	// 	setFocus();
	// }
	// else {
	// 	clearFocus();
	// }
}

void YaExpandingOverlayLineEdit::setState(State state)
{
	if (state_ == Normal && state == Collapsing)
		state = Normal;

	state_ = state;
	YaEmptyTextLineEdit::setVisible(state_ != Normal);

	switch (state_) {
	case Normal:
		break;
	case Collapsing:
		startAnimation(0);
		break;
	case Expanding:
		startAnimation(groundingGeometry_.width());
	case Expanded:
		break;
	}

	animate();
}

void YaExpandingOverlayLineEdit::startAnimation(int toWidth)
{
	expandCollapseTimeLine_->setFrameRange(width(), toWidth);
	if (expandCollapseTimeLine_->state() != QTimeLine::Running)
		expandCollapseTimeLine_->start();
}

QWidget* YaExpandingOverlayLineEdit::controller() const
{
	return controller_;
}

void YaExpandingOverlayLineEdit::setController(QWidget* controller)
{
	controller_ = controller;
	groundingChanged();
}

QList<QWidget*> YaExpandingOverlayLineEdit::grounding() const
{
	return grounding_;
}

void YaExpandingOverlayLineEdit::addToGrounding(QWidget* widget)
{
	grounding_.append(widget);
	widget->installEventFilter(this);
	groundingChanged();
}

bool YaExpandingOverlayLineEdit::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::KeyPress && obj == this) {
		QKeyEvent* ke = static_cast<QKeyEvent*>(e);
		if (ke->key() == Qt::Key_Escape) {
			emit cancelled();
			setText(QString());
			return true;
		}

		if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
			emit enteredText(text());
			// setText(QString());
			return true;
		}
	}

	if ((e->type() == QEvent::Move || e->type() == QEvent::Resize || e->type() == QEvent::Show || e->type() == QEvent::Hide) &&
	    grounding_.contains((QWidget*)obj)) {
		groundingChanged();
	}

	return YaEmptyTextLineEdit::eventFilter(obj, e);
}

void YaExpandingOverlayLineEdit::groundingChanged()
{
	updateGeometryTimer_->start();
}

void YaExpandingOverlayLineEdit::recalculateGeometry()
{
	if (grounding_.isEmpty())
		return;

	QRect r(parentWidget()->rect().bottomRight(), parentWidget()->rect().topLeft());
	foreach(QWidget* w, grounding_) {
		QRect g = w->geometry();
		if (g.left() < r.left())
			r.setLeft(g.left());
		if (g.right() > r.right())
			r.setRight(g.right());
		if (g.top() < r.top())
			r.setTop(g.top());
		if (g.bottom() > r.bottom())
			r.setBottom(g.bottom());
	}

	groundingGeometry_ = r;
	animate();
}

QRect YaExpandingOverlayLineEdit::animatedRect() const
{
	QRect r(groundingGeometry_.topLeft(), QSize(expandCollapseTimeLine_->currentFrame(), groundingGeometry_.height()));
	if (!controller_)
		return r;

	if (controller_->geometry().left() > groundingGeometry_.right())
		r.moveLeft(groundingGeometry_.right() - r.width() + 1);

	return r;
}

void YaExpandingOverlayLineEdit::animate()
{
	switch (state_) {
	case Normal:
		setGeometry(QRect(groundingGeometry_.topLeft(), QSize(0, groundingGeometry_.height())));
		break;
	case Expanded:
		setGeometry(groundingGeometry_);
		break;
	case Collapsing:
	case Expanding:
		setGeometry(animatedRect());

		if (state_ == Expanding && expandCollapseTimeLine_->currentFrame() >= groundingGeometry_.width())
			setState(Expanded);

		if (state_ == Collapsing && expandCollapseTimeLine_->currentFrame() <= 0)
			setState(Normal);
		break;
	}
}
