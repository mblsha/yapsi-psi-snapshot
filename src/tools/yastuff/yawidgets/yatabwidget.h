/*
 * yatabwidget.h
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

#ifndef YATABWIDGET_H
#define YATABWIDGET_H

#include <QTabWidget>

class QMenu;
class YaWindowTheme;

class YaTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	YaTabWidget(QWidget* parent);
	~YaTabWidget();

	QSize tabSizeHint() const;
	QRect tabRect() const;

	bool tabHighlighted(int index) const;
	void setTabHighlighted(int index, bool highlighted);

	void updateHiddenTabs();
	void setLayoutUpdatesEnabled(bool enabled);

	const YaWindowTheme& theme() const;

	void doBlockSignals(bool blockSignals);

signals:
	void closeTab(int index);
	void reorderTabs(int oldIndex, int newIndex);

	// not implemented:
	void mouseDoubleClickTab(QWidget* tab);
	void aboutToShowMenu(QMenu *);
	void tabContextMenu(int tab, QPoint pos, QContextMenuEvent * event);

public slots:
	void aboutToShow(int index);

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);
	void tabInserted(int);
	void tabRemoved(int);

private:
	void updateLayout();
};

#endif
