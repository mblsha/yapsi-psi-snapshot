/*
 * yaclosebutton.h - close button
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

#ifndef YACLOSEBUTTON_H
#define YACLOSEBUTTON_H

#include <QToolButton>

class YaCloseButton : public QToolButton
{
	Q_OBJECT
public:
	YaCloseButton(QWidget* parent);

	// reimplemented
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

	bool chatButton() const;
	void setChatButton(bool chatButton);

private slots:
	void closeButtonAnimation();

protected:
	// reimplemented
	void paintEvent(QPaintEvent* e);
	void enterEvent(QEvent* event);
	void leaveEvent(QEvent* event);

private:
	bool chatButton_;
	bool isSmall_;
	QPixmap normal_;
	int closeFrame_;
	QList<QPixmap> closePixmaps_;
	QTimer* closeButtonAnimationTimer_;
};

#endif
