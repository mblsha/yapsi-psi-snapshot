/*
 * YaBusyWidget2.h - cool animating widget
 * Copyright (C) 2007  Michail Pishchagin
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

#ifndef YABUSYWIDGET2_H
#define YABUSYWIDGET2_H

#include <QWidget>

class QTimeLine;

class YaBusyWidget2 : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( bool active READ isActive WRITE setActive )

	Q_OVERRIDE( QSize minimumSize DESIGNABLE false SCRIPTABLE false )
	Q_OVERRIDE( QSize maximumSize DESIGNABLE false SCRIPTABLE false )

public:
	YaBusyWidget2(QWidget* parent);
	~YaBusyWidget2();

	bool isActive() const;

	// reimplemented
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

signals:
	void stateChanged();

public slots:
	void start();
	void stop();
	void setActive(bool);

protected:
	// reimplemented
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);

private:
	enum State {
		Normal = 0,
		Activating,
		Activated,
		Deactivating
	};
	State state_;
	QTimeLine* fadeTimeLine_;
	QTimeLine* scrollTimeLine_;

	const QPixmap& framePixmap() const;

private slots:
	void animate();
	void updateTimeLines();
};

#endif
