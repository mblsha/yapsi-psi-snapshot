/*
 * yawindowextrabutton.h
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

#ifndef YAWINDOWEXTRABUTTON_H
#define YAWINDOWEXTRABUTTON_H

#include <QToolButton>

class YaWindowExtraButtonBase : public QToolButton
{
	Q_OBJECT
public:
	YaWindowExtraButtonBase(QWidget* parent);

	virtual QPixmap currentPixmap() const = 0;

signals:
	void doPaint();

protected:
	// reimplemented
	void paintEvent(QPaintEvent* event);
	bool event(QEvent* event);

private slots:
	void buttonToggled();
};

class YaWindowExtraButton : public YaWindowExtraButtonBase
{
	Q_OBJECT
public:
	YaWindowExtraButton(QWidget* parent);

	enum Type {
		Close = 0,
		Maximize,
		Minimize
	};

	void setType(Type type);

	// reimplemented
	QPixmap currentPixmap() const;

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

private:
	Type type_;
};

#endif
