/*
 * yajidlabel.h - clickable dash-underlined JID label widget
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

#ifndef YAJIDLABEL_H
#define YAJIDLABEL_H

#include <QLabel>

class QPainter;

class YaJidLabel : public QLabel
{
	Q_OBJECT
public:
	YaJidLabel(QWidget* parent);

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

	virtual QRect textRect() const;
	virtual Qt::Alignment alignment() const;
	virtual void drawBackground(QPainter* painter);
};

#endif
