/*
 * yachevronbutton.h - chevron button
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

#ifndef YACHEVRONBUTTON_H
#define YACHEVRONBUTTON_H

#include <QToolButton>

class QTimer;

class YaChevronButton : public QToolButton
{
	Q_OBJECT
public:
	YaChevronButton(QWidget* parent);

	QColor highlightColor() const;
	void setHighlightColor(const QColor& color);

	bool enableBlinking() const;
	void setEnableBlinking(bool enableBlinking);

	// reimplemented
	QSize sizeHint() const;

	QPixmap buttonPixmap(bool pressed) const;

protected:
	// reimplemented
	void mousePressEvent(QMouseEvent* e);
	void paintEvent(QPaintEvent* e);

private slots:
	void toggleBlinkState();

private:
	QPixmap normal_;
	QPixmap pressed_;
	QPixmap blinking_;
	QColor highlightColor_;
	bool enableBlinking_;
	QTimer* blinkTimer_;
	bool isBlinking_;
};


#endif
