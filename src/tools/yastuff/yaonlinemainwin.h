/*
 * yaonlinemainwin.h
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

#ifndef YAONLINEMAINWIN_H
#define YAONLINEMAINWIN_H

#include "yawindow.h"

class PsiCon;
class QPainter;
class YaOnlineExpansionButton;

#include "xmpp_status.h"
#include "yawindowtheme.h"
#include "delayedvariable.h"

class YaOnlineMainWin : public YaWindow
{
	Q_OBJECT
public:
	YaOnlineMainWin(PsiCon* controller, QWidget* parent, Qt::WindowFlags f);
	~YaOnlineMainWin();

	// reimplemented
	virtual const YaWindowTheme& theme() const;

	virtual XMPP::Status::Type statusType() const = 0;

protected:
	PsiCon* controller() const;
	virtual void paint(QPainter* p);
	bool isOnlineVisible() const;

	// reimplemented
	virtual QRegion getMask() const;
	virtual int additionalTopMargin() const;
	virtual int additionalBottomMargin() const;
	virtual int additionalLeftMargin() const;
	virtual int additionalRightMargin() const;
#ifdef YAPSI_ACTIVEX_SERVER
	virtual void interactiveOperationStarted();
	virtual void interactiveOperationFinished();
	virtual bool interactiveOperationEnabled() const;
	virtual void invalidateMask();
	virtual bool showAsActiveWindow() const;
	virtual void setYaMaximized(bool maximized);
	virtual bool enableTopLeftBorderResize() const;
#endif

#ifdef YAPSI_ACTIVEX_SERVER
	// reimplemented
	virtual void initCurrentOperation(const QPoint& mousePos);
	virtual void deinitCurrentOperation();
	virtual void moveOperation(const QPoint& delta);
#endif

	// reimplemented
	void paintEvent(QPaintEvent*);
#ifdef YAPSI_ACTIVEX_SERVER
	bool event(QEvent*);
	bool winEvent(MSG* message, long* result);
	void changeEvent(QEvent* event);
	void moveEvent(QMoveEvent*);
	void resizeEvent(QResizeEvent*);
#endif

public slots:
	void doBringToFront();
#ifdef YAPSI_ACTIVEX_SERVER
	void hideOnline();
#endif

	virtual void decorateButton(int);

protected slots:
	virtual void setWindowVisible(bool visible);
	virtual void clearMoods();
	virtual void togglePreferences();
	void raiseSidebar();

	virtual void statusSelected(XMPP::Status::Type) = 0;
	virtual void statusSelectedManually(XMPP::Status::Type);
	virtual void statusSelectedManuallyHelper(XMPP::Status::Type) = 0;

#ifdef YAPSI_ACTIVEX_SERVER
	void onlineHideRoster();
	void onlineShowRoster();
	void onlineHiding();
	void onlineVisible();
	void onlineCreated(int onlineWinId);
	void activateRoster();
	void onlineDeactivated();
	void showRelativeToOnline(const QRect& onlineRect);

	void showOnline(bool animate, bool raiseWindow = true);
	void showOnlineWithoutAnimation();
	void showOnlineAfterDesktopResize();
	void updateOnlineExpansion();
	QRect onlineExpansionRect() const;
	void onlineExpansionClicked();
	bool isOnlineActive() const;

	// reimplemented
	void activationChangeUpdate();

	void updateInteractiveOperation();
	void interactiveOperationFinishedHelper();

	void setMainToOnline();

	void desktopResized(int desktop);
	void afterShowWidgetOffscreen();
	void afterWindowMoved();
#endif

private:
	PsiCon* controller_;
	YaWindowTheme theme_;
#ifdef YAPSI_ACTIVEX_SERVER
	QTimer* updateInteractiveOperationTimer_;
	YaOnlineExpansionButton* onlineExpansion_;
	bool onlineExpansionVisible_;
	bool temporarilyHiddenOnline_;
	bool delayingVisibility_;
	bool initialShow_;
	bool showingSidebarWithoutRaise_;
	QTimer* afterWindowMovedTimer_;
	QTimer* showOnlineWithoutAnimationTimer_;
	QTimer* showOnlineAfterDesktopResizeTimer_;

	int onlineWinId_;
	QRect onlineOldGeometry_;
	DelayedVariable<bool> isOnlineActive_;
#endif
};

#endif
