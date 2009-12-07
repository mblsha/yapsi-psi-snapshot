/*
 * yawindow.h - custom borderless window class
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

#ifndef YAWINDOW_H
#define YAWINDOW_H

#include <QFrame>

#include "overlaywidget.h"
#include "advwidget.h"

class QSizeGrip;
class QToolButton;
class QHBoxLayout;
class YaWindowExtraButton;
class YaWindowTheme;
class QTimer;

class YaWindowExtra : public QWidget
{
	Q_OBJECT
public:
	YaWindowExtra(QWidget* parent);

	bool minimizeEnabled() const;
	void setMinimizeEnabled(bool enabled);
	bool maximizeEnabled() const;
	void setMaximizeEnabled(bool enabled);

	void setButtonsVisible(bool visible);

signals:
	void closeClicked();
	void minimizeClicked();
	void maximizeClicked();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void childEvent(QChildEvent* event);

private slots:
	void doPaint();

private:
	QHBoxLayout* hbox_;
	YaWindowExtraButton* minimizeButton_;
	YaWindowExtraButton* maximizeButton_;
	YaWindowExtraButton* closeButton_;
	bool minimizeEnabled_;
	bool maximizeEnabled_;
	QTimer* paintTimer_;

	void addButton(QToolButton* button);
};

class YaWindowBase : public OverlayWidget<QFrame, YaWindowExtra>
{
	Q_OBJECT
public:
	YaWindowBase(QWidget* parent = 0);
	~YaWindowBase();

	enum Mode {
		SystemWindowBorder = 0,
		CustomWindowBorder
	};

	Mode mode() const;
	void setMode(Mode mode);

	virtual bool showAsActiveWindow() const;
	void setOpacityOptionPath(const QString& optionPath);

	bool minimizeEnabled() const;
	void setMinimizeEnabled(bool enabled);
	bool maximizeEnabled() const;
	void setMaximizeEnabled(bool enabled);
	bool isYaFixedSize() const;
	void setYaFixedSize(bool fixedSize);

	bool staysOnTop() const;
	void setStaysOnTop(bool staysOnTop);

	bool isMoveArea(const QPoint& pos);

	// reimplemented (sorta)
	void setWindowFlags(Qt::WindowFlags type);

	// reimplemented
	bool eventFilter(QObject* obj, QEvent* event);

	YaWindowExtra* windowExtra() const;

	QRect yaContentsRect() const;
	QRect contentsGeometryToFrameGeometry(const QRect& contentsGeometry) const;
	QRect frameGeometryToContentsGeometry(const QRect& frameGeometry) const;

	virtual int additionalTopMargin() const;
	virtual int additionalBottomMargin() const;
	virtual int additionalLeftMargin() const;
	virtual int additionalRightMargin() const;

	virtual const YaWindowTheme& theme() const = 0;

protected:
	// reimplemented
	void moveEvent(QMoveEvent*);
	void resizeEvent(QResizeEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseDoubleClickEvent(QMouseEvent*);
	void paintEvent(QPaintEvent*);
	void changeEvent(QEvent* event);

	virtual void paint(QPainter* p);
	virtual void updateContentsMargins(int left = 0, int top = 0, int right = 0, int bottom = 0);

	void mouseMoveEvent(QMouseEvent*, QPoint pos);

	bool isMoveOperationActive() const;
	virtual void interactiveOperationStarted();
	virtual void interactiveOperationFinished();
	virtual bool interactiveOperationEnabled() const;

	virtual void initCurrentOperation(const QPoint& mousePos);
	virtual void deinitCurrentOperation();
	virtual void moveOperation(const QPoint& delta);

#ifdef Q_OS_WIN
	// reimplemented
	bool winEvent(MSG* msg, long* result);
#endif

protected slots:
	virtual void optionChanged(const QString& option);

private slots:
	void closeClicked();
	void minimizeClicked();
	void maximizeClicked();
	void invalidateGeometry();

private:
	// adapted from QMdiSubWindow class code
	enum Operation {
		None,
		Move,
		LeftResize,
		RightResize,
		TopLeftResize,
		TopRightResize,
		TopResize,
		BottomResize,
		BottomLeftResize,
		BottomRightResize
	};

	enum ChangeFlag {
		HMove          = 1 << 0,
		VMove          = 1 << 1,
		HResize        = 1 << 2,
		VResize        = 1 << 3,
		HResizeReverse = 1 << 4,
		VResizeReverse = 1 << 5
	};

	struct OperationInfo {
		uint changeFlags;
		Qt::CursorShape cursorShape;
		QRegion region;
		bool hover;
		OperationInfo(uint changeFlags, Qt::CursorShape cursorShape, bool hover = true)
			: changeFlags(changeFlags)
			, cursorShape(cursorShape)
			, hover(hover)
		{}
	};

	typedef QMap<Operation, OperationInfo> OperationInfoMap;

	Mode mode_;
	Operation currentOperation_;
	bool isInInteractiveMode_;
	QPoint mousePressGlobalPosition_;
	QPoint mousePressPosition_;
	QRect oldGeometry_;
	OperationInfoMap operationMap_;
	QSizeGrip* sizeGrip_;
	QString opacityOptionPath_;
	QRect normalRect_;
	bool staysOnTop_;
	bool isFixedSize_;
	QRect previousGeometry_;
#ifdef Q_WS_WIN
	int previousDesktopWidth_;
	int previousDesktopHeight_;
#endif

	void invalidateMode();
	void updateWindowFlags();
	Qt::WindowFlags desiredWindowFlags();

	Operation getOperation(const QPoint& pos) const;
	QRegion getRegion(Operation operation) const;
	void updateCursor();
	void setNormalMode();

	void setNewGeometry(const QPoint& pos);
	void updateOpacity();

	bool isMoveOperation() const;
	bool isResizeOperation() const;

	// reimplemented
	virtual QRect extraGeometry() const;
	virtual bool extraButtonsShouldBeVisible() const;

protected:
	virtual void repaintBackground();
	virtual bool expandWidthWhenMaximized() const;
	virtual int cornerRadius() const;
	virtual QRegion getMask() const;
	virtual bool enableTopLeftBorderResize() const;
	QPoint mousePressGlobalPosition() const;
	QPoint mousePressPosition() const;
	void getPreviousGeometry();
	bool isInInteractiveMode() const;

	virtual void invalidateMask();
	QRect yaMaximizedRect() const;
	bool isYaMaximized() const;
	virtual void setYaMaximized(bool maximized);
};

class YaWindow : public AdvancedWidget<YaWindowBase>
{
	Q_OBJECT
public:
	YaWindow(QWidget* parent = 0, Qt::WindowFlags f = 0);
	~YaWindow();

	// reimplemented
	void setVisible(bool visible);
	virtual bool showAsActiveWindow() const;

protected:
	// reimplemented
	void changeEvent(QEvent* e);

	QTimer* activationChangeUpdateTimer() const;

protected slots:
	virtual void activationChangeUpdate();

private:
	QTimer* activationChangeUpdateTimer_;
	bool isActiveWindow_;
};

#endif
