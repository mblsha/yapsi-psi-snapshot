/*
 * yamultilinetabbar.h
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

#ifndef YAMULTILINETABBAR_H
#define YAMULTILINETABBAR_H

#include "yatabbarbase.h"

class TabbableWidget;
class QToolButton;
class YaFlashingScrollBar;

class YaMultiLineTabBar : public YaTabBarBase
{
	Q_OBJECT
public:
	YaMultiLineTabBar(QWidget* parent);
	~YaMultiLineTabBar();

	// reimplemented
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

signals:
	void reorderTabs(int oldIndex, int newIndex);

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	QSize tabSizeHint(int index) const;
	QRect tabRect(int index) const;
	void tabLayoutChange();
	void wheelEvent(QWheelEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void enterEvent(QEvent* event);
	void leaveEvent(QEvent* event);

private slots:
	void scrollUp();
	void scrollDown();
	void scrollBarValueChanged(int value);
	void autoScroll();
	void closeButtonAnimation();
	void clearCloseButtonFrame();

private:
	bool layoutUpdatesEnabled_;
	QList<QRect> tabRect_;
	QRect emptySpace_;
	int numRows_;
	int rowOffset_;
	QToolButton* scrollUp_;
	QToolButton* scrollDown_;
	YaFlashingScrollBar* scrollBar_;
	QList<QPixmap> closePixmaps_;
	QVector<int> closeFrame_;
	QTimer* closeButtonAnimationTimer_;

	QList<int> tabDrawOrder_;
	QPoint dragPosition_;
	QPoint dragOffset_;
	int draggedTabIndex_;
	int dragDestinationIndex_;
	QRect dragEmptySpaceRect_;
	QTimer* autoScrollTimer_;
	int autoScrollCount_;
	bool closeButtonHit_;

	void updateTabDrawOrder();
	void startDrag();

	bool autoScrollToBottom() const;
	bool autoScrollToTop() const;

	// reimplemented
	virtual int minimumTabWidth() const;
	virtual int maximumTabWidth() const;
	virtual QSize preferredTabSize(const QString& text, bool current) const;
	virtual void currentIndexChanged();
	virtual QRect closeButtonRect(int index, const QRect& currentTabRect) const;
	virtual void drawTab(QPainter* painter, int index, const QRect& tabRect);
	virtual bool isMultiLine() const;
	virtual bool tabHovered(int index) const;

	void relayoutTabs(bool ensureCurrentVisible = false);

	int rowHeight() const;
};

#endif
