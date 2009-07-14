/*
 * yalinkbutton.h - hyperlink-styled button
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

#ifndef YALINKBUTTON_H
#define YALINKBUTTON_H

#include <QPushButton>

class YaLinkButton : public QPushButton
{
	Q_OBJECT
public:
	YaLinkButton(QWidget* parent = 0);

	// reimplemented
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

	Qt::Alignment alignment() const;
	void setAlignment(Qt::Alignment alignment);

	bool encloseInBrackets() const;
	void setEncloseInBrackets(bool enclose);

protected:
	void setPaintOptions();
	
	// reimplemented
	void changeEvent(QEvent*);
	void paintEvent(QPaintEvent*);

private:
	Qt::Alignment alignment_;
	bool encloseInBrackets_;
};

#endif
