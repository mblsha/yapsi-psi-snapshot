/*
 * yaclosebutton.cpp - close button
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

#include "yaclosebutton.h"

#include <QTimer>
#include <QPainter>

#include "yavisualutil.h"

YaCloseButton::YaCloseButton(QWidget* parent)
	: QToolButton(parent)
	, chatButton_(false)
	, closeFrame_(0)
	, closeButtonAnimationTimer_(0)
{
	setAttribute(Qt::WA_Hover, true);
	setFocusPolicy(Qt::NoFocus);
	setCursor(Qt::PointingHandCursor);

	closeButtonAnimationTimer_ = new QTimer(this);
	closeButtonAnimationTimer_->setSingleShot(false);
	closeButtonAnimationTimer_->setInterval(20);
	connect(closeButtonAnimationTimer_, SIGNAL(timeout()), SLOT(closeButtonAnimation()));

	setChatButton(chatButton_);
}

QSize YaCloseButton::sizeHint() const
{
	if (!chatButton_) {
		return normal_.size();
	}

	return closePixmaps_[closeFrame_].size();
}

QSize YaCloseButton::minimumSizeHint() const
{
	return sizeHint();
}

bool YaCloseButton::chatButton() const
{
	return chatButton_;
}

void YaCloseButton::setChatButton(bool chatButton)
{
	// Q_ASSERT(!chatButton);
	chatButton_ = chatButton;
#ifndef YAPSI_NO_STYLESHEETS
	if (!chatButton_) {
		normal_  = QPixmap(":/images/pushbutton/cancel/cancel_idle.png");

		setStyleSheet(
		"QToolButton {"
		"	border-image: none;"
		"	border: 0px;"
		"	background: url(:/images/pushbutton/cancel/cancel_idle.png) center center no-repeat;"
		"}"
		"QToolButton:hover:pressed {"
		"	background: url(:/images/pushbutton/cancel/cancel_pressed.png) center center no-repeat;"
		"}"
		"QToolButton:hover {"
		"	background: url(:/images/pushbutton/cancel/cancel_hover.png) center center no-repeat;"
		"}"
		);
	}
	else {
		normal_  = QPixmap(":/images/closetab.png");
		closePixmaps_ = Ya::VisualUtil::closeTabButtonPixmaps();

		setStyleSheet(
		"QToolButton {"
		"	border-image: none;"
		"	border: 0px;"
		"	background: url(:/images/closetab.png) center center no-repeat;"
		"}"
		"QToolButton:hover:pressed, QToolButton:hover {"
		"	background: url(:/images/closetab_pressed.png) center center no-repeat;"
		"}"
		);
	}
#endif
}

void YaCloseButton::enterEvent(QEvent* event)
{
	QToolButton::enterEvent(event);
	if (!chatButton_)
		return;
	closeButtonAnimationTimer_->start();
}

void YaCloseButton::leaveEvent(QEvent* event)
{
	QToolButton::leaveEvent(event);
	if (!chatButton_)
		return;
	closeButtonAnimationTimer_->start();
}

void YaCloseButton::closeButtonAnimation()
{
	closeFrame_ += underMouse() ? +1 : -1;
	closeFrame_ = qMax(0, qMin(closeFrame_, closePixmaps_.count() - 1));

	if (closeFrame_ == 0 || closeFrame_ == closePixmaps_.count() - 1) {
		closeButtonAnimationTimer_->stop();
	}

	update();
}

void YaCloseButton::paintEvent(QPaintEvent* e)
{
	if (!chatButton_) {
		QToolButton::paintEvent(e);
		return;
	}

	QPainter p(this);
	p.drawPixmap(0, 0, closePixmaps_[closeFrame_]);
}
