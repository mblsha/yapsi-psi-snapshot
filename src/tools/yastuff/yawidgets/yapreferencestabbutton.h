/*
 * yapreferencestabbutton.h - a button for the Preferences tabbar
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

#ifndef YAPREFERENCESTABBUTTON_H
#define YAPREFERENCESTABBUTTON_H

#include "yapushbutton.h"

class QStyleOptionButton;

class YaPreferencesTabButton : public YaPushButton
{
	Q_OBJECT
public:
	YaPreferencesTabButton(QWidget* parent);

	// reimplemented
	QSize minimumSizeHint() const;

protected slots:
	// reimplemented
	virtual void updateButtonStyle();

protected:
	// reimplemented
	virtual QStyleOptionButton getStyleOption() const;
	virtual float disabledOpacity() const;
	virtual int iconPopOut() const;
	virtual void adjustRect(const QStyleOptionButton* btn, QRect* rect) const;
};

#endif
