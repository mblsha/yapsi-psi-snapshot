/*
 * yachatcontactinfo.h
 * Copyright (C) 2009  Yandex LLC (Michail Pishchagin)
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

#ifndef YACHATCONTACTINFO_H
#define YACHATCONTACTINFO_H

#include <QAbstractButton>
#include <QFrame>

#include "overlaywidget.h"

class YaChatContactInfoExtra : public QAbstractButton
{
	Q_OBJECT
public:
	YaChatContactInfoExtra(QWidget *parent);

	// reimplemented
	QSize sizeHint() const;

	enum Mode {
		Compact = 0,
		Button
	};

	Mode mode() const;
	void setMode(Mode mode);

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void enterEvent(QEvent* event);
	void leaveEvent(QEvent* event);

private slots:
	void animate();

private:
	static void ensureFrames();

	static QList<QPixmap> frames_;
	QTimer* animationTimer_;
	int currentFrame_;
	Mode mode_;
};

class YaChatContactInfo : public OverlayWidget<QFrame, YaChatContactInfoExtra>
{
	Q_OBJECT
public:
	YaChatContactInfo(QWidget *parent);

	YaChatContactInfoExtra* realWidget() const { return extra(); }

	// reimplemented
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

	// reimplemented
	QRect extraGeometry() const;

signals:
	void clicked();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	bool eventFilter(QObject* obj, QEvent* e);
};

#endif
