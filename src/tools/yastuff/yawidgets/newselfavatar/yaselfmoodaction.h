/*
 * yaselfmoodaction.h - mood widget buttons
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

#ifndef YASELFMOODACTION_H
#define YASELFMOODACTION_H

#include <QAbstractButton>

class YaRotation;

class YaSelfMoodButton : public QAbstractButton
{
	Q_OBJECT
public:
	YaSelfMoodButton(QWidget* parent);
	~YaSelfMoodButton();

	void enableRotation(bool enable);

	QString text() const;

	// reimplemented
	QSize sizeHint() const;

	void updateRects();
	const QRect& pixmapRect() const;
	const QRect& textRect() const;

	QAction* action() const;
	bool isSeparator() const;
	bool actionVisible() const;
	void setDrawSelection(bool draw);
	void setDrawEmpty(bool draw);

	QPixmap currentPixmap() const;

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent* event);
	void actionEvent(QActionEvent* event);

private slots:
	void clicked();
	void actionChanged();

private:
	QAction* action_;
	QWidget* helperWidget_;
	YaRotation* rotation_;
	bool drawSelection_;
	bool drawEmpty_;
	QRect pixmapRect_;
	QRect textRect_;

	bool currentAction() const;
	bool isEmpty() const;
};

class YaSelfMoodActionGroup : public QObject
{
	Q_OBJECT
public:
	YaSelfMoodActionGroup(QObject* parent);
	~YaSelfMoodActionGroup();

	void clear();

	QList<QAction*> actions() const;
	void addAction(QAction* action);
	void removeAction(QAction* action);

signals:
	void actionsChanged();

private:
	QList<QAction*> actions_;
};

#endif
