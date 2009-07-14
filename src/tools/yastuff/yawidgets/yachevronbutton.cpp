/*
 * yachevronbutton.cpp - chevron button
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

#include "yachevronbutton.h"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

YaChevronButton::YaChevronButton(QWidget* parent)
	: QToolButton(parent)
	, enableBlinking_(false)
	, blinkTimer_(0)
	, isBlinking_(false)
{
	setFocusPolicy(Qt::NoFocus);

	normal_ = QPixmap(":/images/chevron.png");
	pressed_ = QPixmap(":/images/chevron_pressed.png");
	blinking_ = QPixmap(":/images/chevron_blinking.png");

	blinkTimer_ = new QTimer(this);
	blinkTimer_->setInterval(500);
	blinkTimer_->setSingleShot(false);
	connect(blinkTimer_, SIGNAL(timeout()), SLOT(toggleBlinkState()));

// #ifndef YAPSI_NO_STYLESHEETS
// 	setStyleSheet(
// 	"QToolButton {"
// 	"	border-image: none;"
// 	"	border: 0px;"
// 	"	background: url(:/images/chevron.png) center center no-repeat;"
// 	"}"
// 	"QToolButton:pressed, QToolButton:hover {"
// 	"	background: url(:/images/chevron_pressed.png) center center no-repeat;"
// 	"}"
// 	"*::down-arrow, *::menu-indicator {"
// 	"	image: none;"
// 	"	width: 0px;"
// 	"	height: 0px;"
// 	"}"
// 	);
// #endif
}

QSize YaChevronButton::sizeHint() const
{
	return normal_.size();
}

void YaChevronButton::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		showMenu();
	}
}

QColor YaChevronButton::highlightColor() const
{
	return highlightColor_;
}

void YaChevronButton::setHighlightColor(const QColor& color)
{
	highlightColor_ = color;
	// if (isVisible()) {
	// 	QTimer::singleShot(0, this, SLOT(update()));
	// }
}

bool YaChevronButton::enableBlinking() const
{
	return enableBlinking_;
}

void YaChevronButton::setEnableBlinking(bool enableBlinking)
{
	if (enableBlinking_ != enableBlinking) {
		enableBlinking_ = enableBlinking;
		if (enableBlinking_) {
			blinkTimer_->start();
		}
		else {
			blinkTimer_->stop();
			isBlinking_ = false;
		}
	}
}

void YaChevronButton::toggleBlinkState()
{
	isBlinking_ = !isBlinking_;
	if (isVisible()) {
		QTimer::singleShot(0, this, SLOT(update()));
	}
}

void YaChevronButton::paintEvent(QPaintEvent* e)
{
	QPainter p(this);

	// if (highlightColor_.isValid()) {
	// 	p.fillRect(rect(), highlightColor_);
	// }

	QPixmap px;
	if (testAttribute(Qt::WA_UnderMouse) || isDown()) {
		px = pressed_;
	}
	else if (isBlinking_) {
		px = blinking_;
	}
	else {
		px = normal_;
	}

	// QToolButton::paintEvent(e);
	p.drawPixmap((width() - px.width()) / 2,
	             (height() - px.height()) / 2,
	             px);
}

QPixmap YaChevronButton::buttonPixmap(bool pressed) const
{
	if (pressed)
		return pressed_;
	else
		return normal_;
}
