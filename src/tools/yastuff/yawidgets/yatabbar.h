/*
 * yatabbar.h
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

#ifndef YATABBAR_H
#define YATABBAR_H

#include "yatabbarbase.h"

#include <QColor>
#include <QTimeLine>

#include "overlaywidget.h"
#include "yachevronbutton.h"

class QSignalMapper;
class YaCloseButton;
class TabbableWidget;

class YaTabBar : public OverlayWidget<YaTabBarBase, YaChevronButton>
{
	Q_OBJECT
public:
	YaTabBar(QWidget* parent);
	~YaTabBar();

	// reimplemented
	QSize minimumSizeHint() const;

	// reimplemented
	virtual void updateHiddenTabs();

signals:
	void reorderTabs(int oldIndex, int newIndex);

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	QSize tabSizeHint(int index) const;
	void tabLayoutChange();
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

private slots:
	void closeButtonClicked();

private:
	YaCloseButton* closeButton_;
	QList<QAction*> hiddenTabsActions_;

	QVector<int> hiddenTabs_;
	QVector<QSize> tabSizeHint_;
	QStringList previousTabNames_;
	int previousCurrentIndex_;
	QSize previousSize_;
	QString previousFont_;

	QList<int> tabDrawOrder_;
	QPoint dragPosition_;
	QPoint dragOffset_;
	int draggedTabIndex_;

	void updateTabDrawOrder();
	QRect draggedTabRect() const;
	void startDrag();

	// reimplemented
	virtual QSize preferredTabSize(const QString& text, bool current) const;
	virtual int minimumTabWidth() const;
	virtual int maximumTabWidth() const;
	virtual int maximumWidth() const;
	virtual int draggedTabIndex() const;

	bool needToRelayoutTabs();
	virtual void relayoutTabs();
	void relayoutTabsHelper();
	QSize relayoutTab(int index, int totalTabWidth, const QList<QSize>& relayoutTabData, bool rightmostTab);
	QList<int> visibleTabs();
	bool drawHighlightChevronBackground() const;

	// reimplemented
	virtual void updateHiddenTabActions();
	virtual QRect closeButtonRect(int index, const QRect& currentTabRect) const;

	bool showTabs() const;
	QRect closeButtonRect() const;
	void updateCloseButtonPosition();
	void updateCloseButtonPosition(const QRect& closeButtonRect);
	QRect chevronButtonRect() const;

	// reimplemented
	QRect extraGeometry() const;
	bool extraShouldBeVisible() const;

	QSignalMapper* activateTabMapper_;
};

#endif
