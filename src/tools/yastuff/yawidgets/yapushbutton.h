/*
 * yapushbutton.h - custom push button
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

#ifndef YAPUSHBUTTON_H
#define YAPUSHBUTTON_H

#include <QPushButton>

class QStyleOptionButton;

class YaPushButton : public QPushButton
{
	Q_OBJECT
public:
	YaPushButton(QWidget* parent);

	enum ButtonStyle {
		ButtonStyle_Normal = 0,
		ButtonStyle_Destructive
	};

	virtual void setButtonStyle(ButtonStyle buttonStyle);

	// reimplemented
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

	virtual void init();
	static void initAllButtons(QWidget* window);

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

	virtual QStyleOptionButton getStyleOption() const;
	virtual float disabledOpacity() const;
	virtual int iconPopOut() const;
	virtual void adjustRect(const QStyleOptionButton* btn, QRect* rect) const;

protected slots:
	virtual void updateButtonStyle();

private:
	ButtonStyle buttonStyle_;
	bool initialized_;
};

#endif
