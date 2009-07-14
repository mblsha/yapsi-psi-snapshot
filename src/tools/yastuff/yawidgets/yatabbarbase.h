/*
 * yatabbarbase.h - description of the file
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

#ifndef YATABBARBASE_H
#define YATABBARBASE_H

#include <QTabBar>
#include <QHash>

class QStyleOptionTabV2;
class QPainter;
class QTimeLine;
class TabbableWidget;
class YaWindowTheme;

class YaTabBarBase : public QTabBar
{
	Q_OBJECT
public:
	YaTabBarBase(QWidget* parent);
	~YaTabBarBase();

#ifndef WIDGET_PLUGIN
	TabbableWidget* currentTab() const;
	int indexOf(TabbableWidget* tab) const;
#endif
	const YaWindowTheme& theme() const;
	QString tabBackgroundName() const;
	QColor tabBackgroundColor() const;
	QColor tabBlinkColor() const;

	virtual void updateHiddenTabs();
	bool layoutUpdatesEnabled();
	void setLayoutUpdatesEnabled(bool enabled);

signals:
	void aboutToShow(int index);
	void closeTab(int index);

public slots:
	virtual void startFading();
	virtual void stopFading();
	virtual void updateFading();
	void updateLayout();

protected:
	// reimplemented
	void setCurrentIndex(int index);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	bool event(QEvent* event);

protected slots:
	virtual void currentIndexChanged();

protected:
	void updateSendButton();

	virtual QSize preferredTabSize(const QString& text, bool current) const = 0;
	QSize preferredTabSize(int index) const;
	virtual int minimumTabWidth() const = 0;
	virtual int maximumTabWidth() const = 0;
	virtual int maximumWidth() const;
	virtual bool isMultiLine() const;
	virtual bool tabHovered(int index) const;
	QRect tabIconRect(int index) const;
	QRect tabTextRect(int index) const;
	virtual bool tabTextFits(int index) const;

	// virtual void relayoutTabs() = 0;
	virtual void tabLayoutChange() = 0;
	virtual void updateHiddenTabActions();

	static int margin();
	virtual int draggedTabIndex() const;

	virtual QRect closeButtonRect(int index, const QRect& currentTabRect) const = 0;
	virtual void drawTab(QPainter* painter, int index, const QRect& tabRect);
	QStyleOptionTabV2 getStyleOption(int tab) const;
	QColor highlightColor() const;
	QTimeLine* timeLine() const;
	virtual QColor getCurrentColorGrade(const QColor&, const QColor&, const int, const int) const;
	static QPixmap tabShadow(bool isCurrent);

	void clearCaches();
	int textWidth(const QString& text) const;
	int cachedTextHeight() const;
	QSize cachedIconSize() const;

	void clearPressedPosition();
	QPoint pressedPosition() const;

private:
	bool layoutUpdatesEnabled_;
	QTimeLine* timeLine_;
	QPoint pressedPosition_;

	// caches
	QSize cachedIconSize_;
	mutable QHash<QString, int> cachedTextWidth_;
	int cachedTextHeight_;
};

#endif
