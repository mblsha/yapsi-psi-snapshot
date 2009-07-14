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

YaCloseButton::YaCloseButton(QWidget* parent)
	: QToolButton(parent)
	, chatButton_(false)
{
	setFocusPolicy(Qt::NoFocus);
	setCursor(Qt::PointingHandCursor);

	setChatButton(chatButton_);
}

QSize YaCloseButton::sizeHint() const
{
	return normal_.size();
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
