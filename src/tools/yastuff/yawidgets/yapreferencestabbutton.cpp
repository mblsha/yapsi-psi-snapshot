/*
 * yapreferencestabbutton.cpp - a button for the Preferences tabbar
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

#include "yapreferencestabbutton.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>

#include "yawindow.h"
#include "yawindowtheme.h"

YaPreferencesTabButton::YaPreferencesTabButton(QWidget* parent)
	: YaPushButton(parent)
{
	setFocusPolicy(Qt::NoFocus);
}

void YaPreferencesTabButton::updateButtonStyle()
{
#ifndef YAPSI_NO_STYLESHEETS
	// font is the same as on YaSelfLabel / YaContactLabel
	QString styleSheet = QString(
	"QPushButton, QPushButton:hover {"
	"	font-weight: bold;"
	"	font-size: 14px;"
	""
	"	border-image: url(:/images/pushbutton/preferences_tab/pushbutton.png) 8px 8px 2px 8px;"
	"	border-width: 8px 8px 2px 8px;"
	"}"
	""
	"QPushButton:hover {"
	"	border-image: url(:/images/pushbutton/preferences_tab/pushbutton_hover.png);"
	"}"
	""
	"QPushButton:pressed, QPushButton:checked {"
	"	border-image: url(:/images/pushbutton/preferences_tab/pushbutton_pressed.png);"
	"}"
	);
	setStyleSheet(styleSheet);
#endif
}

QStyleOptionButton YaPreferencesTabButton::getStyleOption() const
{
	QStyleOptionButton opt = YaPushButton::getStyleOption();

	YaWindow* yaWindow = dynamic_cast<YaWindow*>(window());
	Q_ASSERT(yaWindow);
	if (yaWindow) {
		opt.palette.setColor(QPalette::ButtonText, isDown() || isChecked() ?
		                     yaWindow->theme().theme().preferencesTabTextActive() :
		                     yaWindow->theme().theme().preferencesTabTextInactive());
	}
	return opt;
}

float YaPreferencesTabButton::disabledOpacity() const
{
	return 1.0;
}

QSize YaPreferencesTabButton::minimumSizeHint() const
{
	return QSize(92, 30);
}

int YaPreferencesTabButton::iconPopOut() const
{
	return 0;
}

void YaPreferencesTabButton::adjustRect(const QStyleOptionButton* btn, QRect* rect) const
{
	Q_UNUSED(btn);
	rect->translate(0, 2);
}
