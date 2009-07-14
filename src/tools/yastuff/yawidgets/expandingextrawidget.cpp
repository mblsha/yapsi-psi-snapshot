/*
 * expandingextrawidget.cpp - base class for cool expanding widgets
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

#include "expandingextrawidget.h"

#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QMenu>

#include "yaselfmoodaction.h"

static const int expandDuration = 100;
static const int collapseDuration = 200;
static const int margin = 6;

ExpandingExtraWidget::ExpandingExtraWidget(QWidget* parent, QWidget* placeholderParent)
	: QWidget(parent)
	, placeholderParent_(placeholderParent)
{
	expandCollapseTimeLine_ = new CustomTimeLine<QSize>(expandDuration, this);
	connect(expandCollapseTimeLine_, SIGNAL(frameChanged()), SLOT(animate()));
	connect(expandCollapseTimeLine_, SIGNAL(finished()), SLOT(animationFinished()));

	// setAttribute(Qt::WA_OpaquePaintEvent, true);

	installEventFilter(this);
	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);
}

ExpandingExtraWidget::~ExpandingExtraWidget()
{
}

void ExpandingExtraWidget::setExpandedState()
{
	qApp->installEventFilter(this);
	// raise();
}

void ExpandingExtraWidget::setNormalState()
{
	qApp->removeEventFilter(this);
	if (hasFocus()) {
		focusNextChild();
	}
}

void ExpandingExtraWidget::startAnimation(ExpandingExtraWidget::AnimationType type, const QSize& toSize)
{
	expandCollapseTimeLine_->setFrameRange(QSize(width(), height()), toSize);
	if (expandCollapseTimeLine_->state() != QTimeLine::Running) {
		expandCollapseTimeLine_->setDuration(type == ExpandingAnimation ? expandDuration : collapseDuration);
		expandCollapseTimeLine_->start();
	}

	repaint();
}

void ExpandingExtraWidget::stopAnimation()
{
	if (expandCollapseTimeLine_->state() != QTimeLine::NotRunning)
		expandCollapseTimeLine_->stop();
}

QWidget* ExpandingExtraWidget::placeholderParent() const
{
	return placeholderParent_;
}

QSize ExpandingExtraWidget::currentAnimationFrame() const
{
	return expandCollapseTimeLine_->currentFrame();
}

bool ExpandingExtraWidget::isAnimating() const
{
	return ((expandCollapseTimeLine_->state() == QTimeLine::Running
	         && expandCollapseTimeLine_->currentFrame() != expandCollapseTimeLine_->endFrame())
	       );
}

void ExpandingExtraWidget::createButtonFor(QAction* action)
{
	YaSelfMoodButton* button = new YaSelfMoodButton(this);
	button->addAction(action);
	button->installEventFilter(this);
	buttons_ << button;
}

void ExpandingExtraWidget::invalidateButtons()
{
	qDeleteAll(buttons_);
	buttons_.clear();
}

void ExpandingExtraWidget::updateMenuFromButtons(QMenu* menu)
{
	if (menu->isVisible())
		return;

	menu->clear();
	foreach(YaSelfMoodButton* button, buttons_) {
		if (button->actionVisible() && button->height() != 0) {
			menu->addAction(button->action());
		}
	}
}

void ExpandingExtraWidget::updateButtonsVisibility(bool visible)
{
	foreach(YaSelfMoodButton* button, buttons_)
		button->setVisible(visible && button->actionVisible());
}

void ExpandingExtraWidget::updateButtonsPositions(const QRect& topButtonsPosition)
{
	QList<YaSelfMoodButton*> visibleButtons;
	QList<YaSelfMoodButton*> hideButtons;

	foreach(YaSelfMoodButton* button, buttons_) {
		if (button->actionVisible()) {
			if (button->isSeparator() && (visibleButtons.isEmpty() || visibleButtons.last()->isSeparator())) {
				hideButtons << button;
				continue;
			}
			visibleButtons << button;
		}
		else {
			hideButtons << button;
		}
	}

	if (!visibleButtons.isEmpty() && visibleButtons.last()->isSeparator()) {
		hideButtons << visibleButtons.last();
		visibleButtons.remove(visibleButtons.last());
	}

	QRect buttonRect = topButtonsPosition;
	buttonRect.setTop(buttonRect.bottom());
	foreach(YaSelfMoodButton* button, visibleButtons) {
		buttonRect.setHeight(button->sizeHint().height());
		button->setGeometry(buttonRect);
		buttonRect.translate(0, buttonRect.height());
	}

	foreach(YaSelfMoodButton* button, hideButtons) {
		button->setGeometry(QRect(0, 0, 0, 0));
	}
}

/**
 * Returns the bottom coordinate of the initialized buttons. If there are currently
 * no visible buttons, -1 is returned.
 */
int ExpandingExtraWidget::buttonsBottom() const
{
	if (buttons_.isEmpty())
		return -1;

	YaSelfMoodButton* button = 0;
	for (int i = buttons_.count() - 1; i > 0; --i) {
		button = buttons_.at(i);
		if (button->actionVisible() && !button->isSeparator())
			break;
	}

	if (!button)
		return -1;

	return button->geometry().bottom() + margin();
}

int ExpandingExtraWidget::margin()
{
	return ::margin;
}

bool ExpandingExtraWidget::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* e = static_cast<QKeyEvent*>(event);
		if (e->key() == Qt::Key_Escape)  {
			cancelAction();
			return true;
		}
	}

	{
		QWidget* objWidget = dynamic_cast<QWidget*>(obj);
		bool childEvent = false;
		if (objWidget)
			childEvent = findChildren<QWidget*>().contains(objWidget);

		if (obj != this && !childEvent) {
			if (event->type() == QEvent::MouseButtonPress) {
				setNormalState();
			}
			return false;
		}
	}

	if (obj == this && event->type() == QEvent::MouseMove) {
		update();
	}

	if (event->type() == QEvent::FocusOut) {
		bool focusIsHere = hasFocus();
		foreach(QWidget* w, findChildren<QWidget*>())
			focusIsHere = focusIsHere || w->hasFocus();

		if (!focusIsHere) {
			focusOut();
		}
	}

	return QWidget::eventFilter(obj, event);
}

void ExpandingExtraWidget::cancelAction()
{
	setNormalState();
}

void ExpandingExtraWidget::focusOut()
{
	setNormalState();
}

void ExpandingExtraWidget::animationFinished()
{
}
