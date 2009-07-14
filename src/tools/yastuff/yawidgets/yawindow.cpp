/*
 * yawindow.cpp - custom borderless window class
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

#include "yawindow.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QPainter>
#include <QSizeGrip>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>

#define USE_PSIOPTIONS
#define USE_YAVISUALUTIL

#ifdef USE_YAVISUALUTIL
#include "yavisualutil.h"
#endif

static const int borderSize = 3;
static const int cornerSize = 10;

#ifdef USE_PSIOPTIONS
#include "psioptions.h"
static const QString customFrameOptionPath = "options.ya.custom-frame";
static const QString chatBackgroundOptionPath = "options.ya.chat-background";
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#ifdef USE_PSIOPTIONS
#include "psitooltip.h"
#endif
#include "yawindowextrabutton.h"

#ifdef YAPSI_ACTIVEX_SERVER
#if QT_VERSION >= 0x040500
#define CUSTOM_SHADOW
#endif
#endif

//----------------------------------------------------------------------------
// YaWindowExtra
//----------------------------------------------------------------------------

YaWindowExtra::YaWindowExtra(QWidget* parent)
	: QWidget(parent)
	, minimizeEnabled_(true)
	, maximizeEnabled_(true)
	, paintTimer_(0)
{
	hbox_ = new QHBoxLayout(this);
	hbox_->setMargin(0);
	hbox_->setSpacing(3);

	int leftMargin, topMargin, rightMargin, bottomMargin;
	getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
	setContentsMargins(leftMargin + 4, topMargin + 4, rightMargin + 4, bottomMargin + 4);

	minimizeButton_ = new YaWindowExtraButton(this);
	minimizeButton_->setType(YaWindowExtraButton::Minimize);
	connect(minimizeButton_, SIGNAL(clicked()), SIGNAL(minimizeClicked()));
	addButton(minimizeButton_);
	minimizeButton_->setToolTip(tr("Minimize"));

	maximizeButton_ = new YaWindowExtraButton(this);
	maximizeButton_->setType(YaWindowExtraButton::Maximize);
	connect(maximizeButton_, SIGNAL(clicked()), SIGNAL(maximizeClicked()));
	addButton(maximizeButton_);
	maximizeButton_->setToolTip(tr("Maximize"));

	closeButton_ = new YaWindowExtraButton(this);
	closeButton_->setType(YaWindowExtraButton::Close);
	connect(closeButton_, SIGNAL(clicked()), SIGNAL(closeClicked()));
	addButton(closeButton_);
	closeButton_->setToolTip(tr("Close"));

	paintTimer_ = new QTimer(this);
	paintTimer_->setSingleShot(true);
	paintTimer_->setInterval(0);
	connect(paintTimer_, SIGNAL(timeout()), SLOT(update()));
}

void YaWindowExtra::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	foreach(YaWindowExtraButtonBase* w, findChildren<YaWindowExtraButtonBase*>()) {
		if (!w->isVisible())
			continue;

		QPixmap pix = w->currentPixmap();
		int offset = (w->geometry().width() - pix.width()) / 2;
		p.drawPixmap(w->geometry().topLeft() + QPoint(offset, offset), pix);
	}
}

void YaWindowExtra::childEvent(QChildEvent* event)
{
	QWidget::childEvent(event);

	if (event->type() == QEvent::ChildPolished) {
		foreach(QWidget* w, findChildren<QWidget*>()) {
			disconnect(w, SIGNAL(doPaint()), this, SLOT(doPaint()));
			connect(w,    SIGNAL(doPaint()), this, SLOT(doPaint()), Qt::QueuedConnection);
			w->setUpdatesEnabled(false);
		}
	}
}

void YaWindowExtra::doPaint()
{
	paintTimer_->start();
}

void YaWindowExtra::addButton(QToolButton* button)
{
	QVBoxLayout* vbox = new QVBoxLayout(0);
	vbox->addSpacing(5);
	vbox->addWidget(button);
	vbox->addStretch();
	hbox_->addLayout(vbox);
}

bool YaWindowExtra::minimizeEnabled() const
{
	return minimizeEnabled_;
}

void YaWindowExtra::setMinimizeEnabled(bool enabled)
{
	minimizeEnabled_ = enabled;
	minimizeButton_->setVisible(enabled);
}

bool YaWindowExtra::maximizeEnabled() const
{
	return maximizeEnabled_;
}

void YaWindowExtra::setMaximizeEnabled(bool enabled)
{
	maximizeEnabled_ = enabled;
	maximizeButton_->setVisible(enabled);
}

void YaWindowExtra::setButtonsVisible(bool visible)
{
	minimizeButton_->setVisible(visible && minimizeEnabled_);
	maximizeButton_->setVisible(visible && maximizeEnabled_);
	closeButton_->setVisible(visible);
}

//----------------------------------------------------------------------------
// YaWindowBase
//----------------------------------------------------------------------------

YaWindowBase::YaWindowBase(QWidget* parent)
	: OverlayWidget<QFrame, YaWindowExtra>(0, new YaWindowExtra(0))
	, mode_(SystemWindowBorder)
	, currentOperation_(None)
	, isInInteractiveMode_(false)
	, staysOnTop_(false)
	, isFixedSize_(false)
{
	Q_ASSERT(parent == 0);
	Q_UNUSED(parent);
	setFrameShape(QFrame::NoFrame);

#ifdef CUSTOM_SHADOW
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TranslucentBackground);
#endif

	operationMap_.insert(Move,         OperationInfo(HMove | VMove, Qt::ArrowCursor, false));
	operationMap_.insert(LeftResize,   OperationInfo(HMove | HResize | HResizeReverse, Qt::SizeHorCursor));
	operationMap_.insert(RightResize,  OperationInfo(HResize, Qt::SizeHorCursor));
	operationMap_.insert(TopLeftResize, OperationInfo(HMove | VMove | HResize | VResize | VResizeReverse
	                     | HResizeReverse, Qt::SizeFDiagCursor));
	operationMap_.insert(TopRightResize, OperationInfo(VMove | HResize | VResize
	                     | VResizeReverse, Qt::SizeBDiagCursor));
	operationMap_.insert(TopResize,    OperationInfo(VMove | VResize | VResizeReverse, Qt::SizeVerCursor));
	operationMap_.insert(BottomResize, OperationInfo(VResize, Qt::SizeVerCursor));
	operationMap_.insert(BottomLeftResize,  OperationInfo(HMove | HResize | VResize | HResizeReverse, Qt::SizeBDiagCursor));
	operationMap_.insert(BottomRightResize, OperationInfo(HResize | VResize,
#ifdef Q_WS_MAC
	                     Qt::ArrowCursor
#else
	                     Qt::SizeFDiagCursor
#endif
	                                                     ));

	sizeGrip_ = new QSizeGrip(this);
	sizeGrip_->installEventFilter(this);

	extra()->setParent(this);

#ifdef USE_PSIOPTIONS
	optionChanged(customFrameOptionPath);
	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
#else
	invalidateMode();
#endif

	// In Qt 4.3.2 mouse move events are sent to widgets who track them
	// only if these widgets do not contain child widgets in front of them
	// that don't have mouse tracking enabled. In order to circumvent this
	// case we also install a hook on QApplication to get all MouseMove
	// events.
	setMouseTracking(true);
	qApp->installEventFilter(this);

	connect(extra(), SIGNAL(closeClicked()), SLOT(closeClicked()));
	connect(extra(), SIGNAL(minimizeClicked()), SLOT(minimizeClicked()));
	connect(extra(), SIGNAL(maximizeClicked()), SLOT(maximizeClicked()));
}

YaWindowBase::~YaWindowBase()
{
}

void YaWindowBase::optionChanged(const QString& option)
{
#ifndef USE_PSIOPTIONS
	Q_UNUSED(option);
#else
	if (option == customFrameOptionPath) {
		setMode(PsiOptions::instance()->getOption(customFrameOptionPath).toBool() ?
		        CustomWindowBorder :
		        SystemWindowBorder);
	}
	else if (option == opacityOptionPath_ ||
	         option == chatBackgroundOptionPath)
	{
		QEvent e(QEvent::ActivationChange);
		changeEvent(&e);
	}
#endif
}


YaWindowBase::Mode YaWindowBase::mode() const
{
	return mode_;
}

void YaWindowBase::setMode(Mode mode)
{
	mode_ = mode;
	invalidateMode();
}

void YaWindowBase::getPreviousGeometry()
{
	previousGeometry_ = geometry();
#ifdef Q_WS_WIN
	previousDesktopWidth_  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	previousDesktopHeight_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);
#endif
}

void YaWindowBase::moveEvent(QMoveEvent* e)
{
	OverlayWidget<QFrame, YaWindowExtra>::moveEvent(e);
	getPreviousGeometry();
}

void YaWindowBase::resizeEvent(QResizeEvent* e)
{
	OverlayWidget<QFrame, YaWindowExtra>::resizeEvent(e);
	invalidateMask();
	getPreviousGeometry();
}

int YaWindowBase::cornerRadius() const
{
	return Ya::VisualUtil::windowCornerRadius();
}

QRegion YaWindowBase::getMask() const
{
	QRect rect = qApp->desktop()->availableGeometry(this);
	bool square = (rect.top() == geometry().top() &&
	               rect.left() == geometry().left() &&
	               rect.right() == geometry().right());

#ifdef USE_YAVISUALUTIL
	return Ya::VisualUtil::roundedMask(size(), cornerRadius(), !square ? Ya::VisualUtil::TopBorders : Ya::VisualUtil::NoBorders);
#else
	return QRegion(this->rect());
#endif
}

void YaWindowBase::invalidateMask()
{
#ifdef USE_YAVISUALUTIL
#ifndef CUSTOM_SHADOW
	if (mode_ == CustomWindowBorder) {
		setMask(getMask());
	}
	else {
		clearMask();
	}
#endif
#endif
}

void YaWindowBase::closeClicked()
{
	if (!extraButtonsShouldBeVisible())
		return;

	close();
}

void YaWindowBase::minimizeClicked()
{
	if (!extraButtonsShouldBeVisible())
		return;

	showMinimized();
}

void YaWindowBase::maximizeClicked()
{
	if (!extraButtonsShouldBeVisible())
		return;

	setYaMaximized(!isYaMaximized());
}

QRect YaWindowBase::yaMaximizedRect() const
{
	Q_ASSERT(extraButtonsShouldBeVisible());
	QRect result = qApp->desktop()->availableGeometry(this);
	result.adjust(-additionalLeftMargin(), -additionalTopMargin(), additionalRightMargin(), additionalBottomMargin());
	if (!expandWidthWhenMaximized()) {
		result.setWidth(geometry().width());
		result.moveLeft(geometry().left());
	}
	return result;
}

bool YaWindowBase::isYaMaximized() const
{
	if (!extraButtonsShouldBeVisible())
		return isMaximized();

	return !normalRect_.isNull();
}

void YaWindowBase::setYaMaximized(bool maximized)
{
	// if (maximized) {
	// 	Q_ASSERT(maximizeEnabled());
	// }

	if (!extraButtonsShouldBeVisible()) {
		if (maximized) {
			showMaximized();
		}
		else {
			showNormal();
		}
	}
	else {
		if (maximized) {
			Q_ASSERT(!isYaMaximized());
			normalRect_ = geometry();
			setGeometry(yaMaximizedRect());
		}
		else {
			Q_ASSERT(isYaMaximized());
			if (!expandWidthWhenMaximized()) {
				normalRect_.moveLeft(geometry().left());
			}

			if (normalRect_.height() == yaMaximizedRect().height()) {
				normalRect_.setHeight(normalRect_.height() / 2);
			}

			if (normalRect_.width() == yaMaximizedRect().width() && expandWidthWhenMaximized()) {
				normalRect_.setWidth(normalRect_.width() / 2);
			}

			setGeometry(normalRect_);
			normalRect_ = QRect();
		}
	}

	repaintBackground();
	setYaFixedSize(isYaFixedSize());
}

void YaWindowBase::setNormalMode()
{
	currentOperation_ = None;
	updateCursor();
}

void YaWindowBase::initCurrentOperation(const QPoint& mousePos)
{
	isInInteractiveMode_ = currentOperation_ != None;
	updateCursor();

	mousePressPosition_ = mapToParent(mousePos);
	oldGeometry_ = geometry();
}

void YaWindowBase::deinitCurrentOperation()
{
	isInInteractiveMode_ = false;
}

void YaWindowBase::moveOperation(const QPoint& delta)
{
}

void YaWindowBase::mousePressEvent(QMouseEvent* e)
{
	// if (!extraButtonsShouldBeVisible() || isYaMaximized()) {
	// 	setNormalMode();
	// 	return;
	// }

	if (e->button() == Qt::LeftButton) {
		e->accept();
		currentOperation_ = getOperation(e->pos());
		initCurrentOperation(e->pos());
	}
}

void YaWindowBase::mouseReleaseEvent(QMouseEvent* e)
{
	// if (!extraButtonsShouldBeVisible() || isYaMaximized()) {
	// 	setNormalMode();
	// 	return;
	// }

	if (isInInteractiveMode_) {
		interactiveOperationFinished();
	}

	if (e->button() == Qt::LeftButton) {
		e->accept();
		deinitCurrentOperation();
		currentOperation_ = getOperation(e->pos());
		updateCursor();
	}
}

bool YaWindowBase::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == sizeGrip_) {
		if (event->type() == QEvent::MouseButtonPress) {
			interactiveOperationStarted();
		}
		else if (event->type() == QEvent::MouseButtonRelease) {
			// FIXME: somehow (obj==sizeGrip_ && event->type() == QEvent::MouseButtonRelease)
			// never happen in reality on Windows
			interactiveOperationFinished();
		}
	}

	if (event->type() == QEvent::MouseMove) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->buttons() == Qt::NoButton) {
			QPoint pos = mapFromGlobal(mouseEvent->globalPos());
			if (rect().contains(pos)) {
				event->accept();
				mouseMoveEvent(mouseEvent, pos);
			}
		}
	}

	return OverlayWidget<QFrame, YaWindowExtra>::eventFilter(obj, event);
}

void YaWindowBase::mouseMoveEvent(QMouseEvent* e, QPoint pos)
{
	if (isInInteractiveMode_ && !(e->buttons() & Qt::LeftButton)) {
		e->ignore();
		deinitCurrentOperation();
		return;
	}

	if (isInInteractiveMode_) {
		interactiveOperationStarted();
	}

	// if (!extraButtonsShouldBeVisible() || isYaMaximized()) {
	// 	setNormalMode();
	// 	return;
	// }

	if (e->buttons() & Qt::LeftButton && isInInteractiveMode_) {
		e->accept();
		setNewGeometry(mapToParent(pos));
		return;
	}

	currentOperation_ = getOperation(pos);
	updateCursor();
}

void YaWindowBase::mouseMoveEvent(QMouseEvent* e)
{
	mouseMoveEvent(e, e->pos());
}

void YaWindowBase::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton && extraButtonsShouldBeVisible()) {
		if (getRegion(Move).contains(e->pos())) {
#ifdef Q_WS_MAC
			showMinimized();
#else
			setYaMaximized(!isYaMaximized());
#endif
			e->accept();
			return;
		}
	}

	OverlayWidget<QFrame, YaWindowExtra>::mouseDoubleClickEvent(e);
}

int getMoveDeltaComponent(uint cflags, uint moveFlag, uint resizeFlag, int delta, int maxDelta, int minDelta)
{
	if (cflags & moveFlag) {
		if (delta > 0)
			return (cflags & resizeFlag) ? qMin(delta, maxDelta) : delta;
		return (cflags & resizeFlag) ? qMax(delta, minDelta) : delta;
	}
	return 0;
}

int getResizeDeltaComponent(uint cflags, uint resizeFlag, uint resizeReverseFlag, int delta)
{
	if (cflags & resizeFlag) {
		if (cflags & resizeReverseFlag)
			return -delta;
		return delta;
	}
	return 0;
}

QPoint constraintToAvailableGeometry(const QRect& availableGeometry, const QPoint& _pos)
{
	QPoint pos = _pos;
	if (pos.x() < availableGeometry.left())
		pos.setX(availableGeometry.left());
	if (pos.y() < availableGeometry.top())
		pos.setY(availableGeometry.top());
	if (pos.x() > availableGeometry.right())
		pos.setX(availableGeometry.right());
	if (pos.y() > availableGeometry.bottom())
		pos.setY(availableGeometry.bottom());
	return pos;
}

void YaWindowBase::setNewGeometry(const QPoint& _pos)
{
	QRegion allScreens;
	for (int i = 0; i < qApp->desktop()->numScreens(); ++i)
		allScreens += qApp->desktop()->availableGeometry(i);

	QRect availableGeometry = allScreens.boundingRect();
	// QRect availableGeometry = qApp->desktop()->availableGeometry(this);

	QPoint pos = constraintToAvailableGeometry(availableGeometry, _pos);
	uint cflags = operationMap_.find(currentOperation_).value().changeFlags;

	QRect g;
	if (cflags & (HMove | VMove)) {
		int dx = getMoveDeltaComponent(cflags, HMove, HResize, pos.x() - mousePressPosition_.x(),
		                               oldGeometry_.width() - minimumSize().width(),
		                               oldGeometry_.width() - maximumSize().width());
		int dy = getMoveDeltaComponent(cflags, VMove, VResize, pos.y() - mousePressPosition_.y(),
		                               oldGeometry_.height() - minimumSize().height(),
		                               oldGeometry_.height() - maximumSize().height());
		g.setTopLeft(oldGeometry_.topLeft() + QPoint(dx, dy));
	}
	else {
		g.setTopLeft(geometry().topLeft());
	}

	if (cflags & (HResize | VResize)) {
		int dx = getResizeDeltaComponent(cflags, HResize, HResizeReverse,
		                                 pos.x() - mousePressPosition_.x());
		int dy = getResizeDeltaComponent(cflags, VResize, VResizeReverse,
		                                 pos.y() - mousePressPosition_.y());
		g.setSize(oldGeometry_.size() + QSize(dx, dy));
	}
	else {
		g.setSize(geometry().size());
	}

	// limit the maximum window size to be the size of the current desktop
	if ((g.width() - additionalLeftMargin() - additionalRightMargin()) >
	    availableGeometry.width()) {
		g.setWidth(availableGeometry.width() +
		           additionalLeftMargin() +
		           additionalRightMargin());
	}
	if ((g.height() - additionalTopMargin() - additionalBottomMargin()) >
	    availableGeometry.height()) {
		g.setHeight(availableGeometry.height() +
		            additionalTopMargin() +
		            additionalBottomMargin());
	}

	// check that window is not moved outside of screen, expecially on
	// Mac OS X. On OSX title should never be higher than the menu bar.
	if ((g.top() + additionalTopMargin()) < availableGeometry.top())
		g.moveTop(availableGeometry.top() - additionalTopMargin());

	// if moved completely off-screen, make sure it's visible again
	if ((g.right() - additionalRightMargin()) < availableGeometry.left())
		g.moveLeft(availableGeometry.left() - additionalLeftMargin());
	if ((g.left() + additionalLeftMargin()) > availableGeometry.right())
		g.moveRight(availableGeometry.right() + additionalRightMargin());
	if ((g.top() + additionalTopMargin()) > availableGeometry.bottom())
		g.moveBottom(availableGeometry.bottom() + additionalBottomMargin());

	QTimer::singleShot(100, this, SLOT(invalidateGeometry()));

	moveOperation(g.topLeft() - oldGeometry_.topLeft());
	setGeometry(g);
}

void YaWindowBase::invalidateGeometry()
{
#ifdef Q_WS_WIN
	// FIXME: Work-around for ONLINE-2155
	QRect g = geometry();
	setGeometry(g.adjusted(0, 0, 10, 0));
	setGeometry(g);
#endif
}

void YaWindowBase::invalidateMode()
{
	bool doShow = isVisible();

	updateWindowFlags();
	invalidateMask();

	extra()->setButtonsVisible(extraButtonsShouldBeVisible());

	QPoint p = pos();
	if (p.x() < 0)
		p.setX(0);
	if (p.y() < 0)
		p.setY(0);
	move(p);

	if (doShow)
		show();
}

QRect YaWindowBase::extraGeometry() const
{
	QRect yaRect = yaContentsRect();
	QRect g(yaRect);
	g.setTopLeft(QPoint(yaRect.right()  - sizeGrip_->sizeHint().width()  + 1,
	                    yaRect.bottom() - sizeGrip_->sizeHint().height() + 1));
	sizeGrip_->setGeometry(g);
	sizeGrip_->raise();

	const QSize sh = extra()->sizeHint();
	return QRect(globalRect().x() + width() - sh.width() - 5 - additionalRightMargin(),
	             globalRect().y() + 0 + additionalTopMargin(),
	             sh.width(), sh.height());
}

bool YaWindowBase::extraButtonsShouldBeVisible() const
{
	return mode_ == CustomWindowBorder;
}

bool YaWindowBase::isMoveOperation() const
{
	return currentOperation_ == Move;
}

bool YaWindowBase::isResizeOperation() const
{
	return currentOperation_ != None && currentOperation_ != Move;
}


YaWindowBase::Operation YaWindowBase::getOperation(const QPoint& pos) const
{
	OperationInfoMap::const_iterator it;
	for (it = operationMap_.constBegin(); it != operationMap_.constEnd(); ++it) {
		// FIXME: an opportunity to cache getRegion() results
		if (getRegion(it.key()).contains(pos)) {
			if (isYaMaximized()) {
				if (it.key() == Move && !expandWidthWhenMaximized())
					return Move;

				return None;
			}

			return it.key();
		}
	}

	return None;
}

QRegion YaWindowBase::getRegion(Operation operation) const
{
	if (mode_ == SystemWindowBorder) {
		return QRegion();
	}

	int ml = additionalLeftMargin();
	int mr = additionalRightMargin();
	int mt = additionalTopMargin();
	int mb = additionalBottomMargin();

#ifdef CUSTOM_SHADOW
	ml -= borderSize;
	mr -= borderSize;
	mt -= borderSize;
	mb -= borderSize;
#endif

	int width = this->width() - ml - mr;
	int height = this->height() - mt - mb;
	QRegion leftBorder(ml, mt, borderSize, height);
	QRegion topBorder(ml, mt, width, borderSize);
	QRegion bottomBorder(ml, mt + height - borderSize, width, borderSize);
	QRegion rightBorder(ml + width - borderSize, mt, borderSize, height);
	QRegion topLeftBorder(ml, mt, cornerSize, cornerSize);
	QRegion topRightBorder(ml + width - cornerSize, mt, cornerSize, cornerSize);
	QRegion bottomLeftCorner(ml, mt + height - cornerSize, cornerSize, cornerSize);
	QRegion bottomRightCorner(ml + width - cornerSize, mt + height - cornerSize, cornerSize, cornerSize);

	bottomLeftCorner = leftBorder.united(bottomBorder).intersected(bottomLeftCorner);
	bottomRightCorner = rightBorder.united(bottomBorder).intersected(bottomRightCorner);

	int topRegionHeight = height;
#ifdef YAPSI
	topRegionHeight = 70;
#endif
	QRegion fullRegion(ml, mt, width, topRegionHeight);
	QRegion borders = leftBorder.united(bottomBorder).united(rightBorder);
#ifndef Q_WS_MAC
	borders = borders.united(topBorder).united(topLeftBorder).united(topRightBorder);
#endif
	fullRegion = fullRegion.subtracted(borders);

	leftBorder = leftBorder.subtracted(bottomLeftCorner);
	bottomBorder = bottomBorder.subtracted(bottomLeftCorner.united(bottomRightCorner));
	rightBorder = rightBorder.subtracted(bottomRightCorner);

#ifndef Q_WS_MAC
#if 0
	if (operation == LeftResize)
		return leftBorder;
	if (operation == TopResize)
		return topBorder;
	if (operation == TopLeftResize)
		return topLeftBorder;
	if (operation == TopRightResize)
		return topRightBorder;
	if (operation == BottomLeftResize)
		return bottomLeftCorner;
#endif
	if (operation == BottomResize)
		return bottomBorder;
	if (operation == RightResize)
		return rightBorder;
#endif
	if (operation == BottomRightResize)
		return bottomRightCorner;
	if (operation == Move)
		return fullRegion;

	return QRegion();
}

void YaWindowBase::updateCursor()
{
	if (currentOperation_ == None) {
		unsetCursor();
		return;
	}

	if (currentOperation_ == Move || operationMap_.find(currentOperation_).value().hover) {
		setCursor(operationMap_.find(currentOperation_).value().cursorShape);
		return;
	}
}

void YaWindowBase::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	Ya::VisualUtil::drawWindowTheme(&p, theme(), rect(), yaContentsRect(), showAsActiveWindow());
	paint(&p);
	Ya::VisualUtil::drawAACorners(&p, theme(), rect(), yaContentsRect());
}

void YaWindowBase::paint(QPainter* p)
{
}

void YaWindowBase::updateContentsMargins(int left, int top, int right, int bottom)
{
	int currentLeft, currentTop, currentRight, currentBottom;
	layout()->getContentsMargins(&currentLeft, &currentTop, &currentRight, &currentBottom);
	layout()->setContentsMargins(additionalLeftMargin() + left + currentLeft,
	                             additionalTopMargin() + top + currentTop,
	                             additionalRightMargin() + right + currentRight,
	                             additionalBottomMargin() + bottom + currentBottom);
}

void YaWindowBase::changeEvent(QEvent* e)
{
	OverlayWidget<QFrame, YaWindowExtra>::changeEvent(e);
	if (e->type() == QEvent::ActivationChange) {
		updateOpacity();
		repaintBackground();
	}
}

void YaWindowBase::repaintBackground()
{
	// update() doesn't repaint all the background on Windows in all cases
	repaint();

	// make YaRosterTabButton repaint correctly
	foreach(QWidget* w, findChildren<QWidget*>()) {
		w->repaint();
	}
}

bool YaWindowBase::minimizeEnabled() const
{
	return extra()->minimizeEnabled();
}

void YaWindowBase::setMinimizeEnabled(bool enabled)
{
	extra()->setMinimizeEnabled(enabled);
	updateWindowFlags();
}

bool YaWindowBase::maximizeEnabled() const
{
	return extra()->maximizeEnabled();
}

bool YaWindowBase::isYaFixedSize() const
{
	return isFixedSize_;
}

void YaWindowBase::setYaFixedSize(bool fixedSize)
{
	isFixedSize_ = fixedSize;
	sizeGrip_->setVisible(!isYaMaximized() && !isFixedSize_);
}

void YaWindowBase::updateWindowFlags()
{
	setWindowFlags(0);
}

Qt::WindowFlags YaWindowBase::desiredWindowFlags()
{
	Qt::WindowFlags wflags = Qt::Window;
	if (mode_ == CustomWindowBorder)
		wflags |= Qt::FramelessWindowHint;

	// on windows adds context menu to the window taskbar button
	wflags |= Qt::WindowSystemMenuHint;

#ifndef Q_WS_WIN
	if (mode_ != CustomWindowBorder) {
#endif
		// without Qt::WindowMinimizeButtonHint window
		// won't get minimized when its taskbar button
		// is clicked
		// if (minimizeEnabled())
			wflags |= Qt::WindowMinimizeButtonHint;
		// if (maximizeEnabled())
			wflags |= Qt::WindowMaximizeButtonHint;
#ifndef Q_WS_WIN
	}
#endif

#ifndef Q_WS_WIN
	if (staysOnTop_)
		wflags |= Qt::WindowStaysOnTopHint;
#endif
	return wflags;
}

void YaWindowBase::setMaximizeEnabled(bool enabled)
{
	extra()->setMaximizeEnabled(enabled);
	updateWindowFlags();
}

bool YaWindowBase::staysOnTop() const
{
	return staysOnTop_;
}

void YaWindowBase::setStaysOnTop(bool staysOnTop)
{
	staysOnTop_ = staysOnTop;
#ifdef Q_WS_WIN
	// we're using this in order to avoid from deleting and
	// recreating a handle each time setStaysOnTop() is called
	SetWindowPos(winId(), staysOnTop_ ? HWND_TOPMOST : HWND_NOTOPMOST,
	             0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
#else
	updateWindowFlags();
#endif
}

bool YaWindowBase::showAsActiveWindow() const
{
	return isActiveWindow();
}

void YaWindowBase::setOpacityOptionPath(const QString& optionPath)
{
	opacityOptionPath_ = optionPath;
	updateOpacity();
}

void YaWindowBase::updateOpacity()
{
	if (opacityOptionPath_.isEmpty())
		return;

#ifdef USE_PSIOPTIONS
	int maximum_opacity = 100;
	int opacity = PsiOptions::instance()->getOption(opacityOptionPath_).toInt();
#ifdef Q_WS_WIN
	// work-around for severe flickering that happens on windows when borderless
	// windows become completely opaque / non-opaque
	if (opacity < 100) {
		maximum_opacity = 99;
	}
#endif

	if (!showAsActiveWindow())
		setWindowOpacity(double(qMax(MINIMUM_OPACITY, qMin(opacity, maximum_opacity))) / 100);
	else
		setWindowOpacity(double(maximum_opacity) / 100);
#endif
}

/**
 * Sadly setWindowFlags isn't virtual, so we cannot fully overload it.
 * But nevertheless we try to override it sorta in order to not
 * accidentally reset borderlessness.
 */
void YaWindowBase::setWindowFlags(Qt::WindowFlags type)
{
	QWidget::setWindowFlags(desiredWindowFlags() | type);

#ifdef Q_WS_WIN
	HMENU sysMenu = GetSystemMenu(winId(), false);
	if (sysMenu) {
		// DeleteMenu(sysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
		DeleteMenu(sysMenu, SC_SIZE, MF_BYCOMMAND);
		DeleteMenu(sysMenu, SC_MOVE, MF_BYCOMMAND);
	}

	setStaysOnTop(staysOnTop_);
#endif
}

YaWindowExtra* YaWindowBase::windowExtra() const
{
	return extra();
}

/**
 * Overriding this in a roster window in order to return false could be useful.
 */
bool YaWindowBase::expandWidthWhenMaximized() const
{
	return true;
}

bool YaWindowBase::isMoveArea(const QPoint& pos)
{
	return getOperation(pos) == Move;
}

#ifdef Q_OS_WIN
bool YaWindowBase::winEvent(MSG* msg, long* result)
{
#if 0
	if (msg->message == WM_WINDOWPOSCHANGING) {
		QRect desktopGeometry = qApp->desktop()->availableGeometry(this);
		int desktopWidth  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int desktopHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

		WINDOWPOS *wpos = (WINDOWPOS *)msg->lParam;
		if ((previousDesktopWidth_ == desktopWidth) &&
		    (previousDesktopHeight_ == desktopHeight) &&
		    (previousGeometry_.width() == wpos->cx) &&
			(previousGeometry_.height() == wpos->cy) &&
			(previousGeometry_.x() != wpos->x || previousGeometry_.y() != wpos->y) &&
			!isInInteractiveMode_ &&
		    extraButtonsShouldBeVisible() &&
		    !isMinimized())
		{
			RECT r;
			GetWindowRect(winId(), &r);
			wpos->x = r.left;
			wpos->y = r.top;
			wpos->cx = r.right - r.left;
			wpos->cy = r.bottom - r.top;

			*result = 0;
			return true;
		}
	}
#endif

	if (msg->message == WM_SYSCOMMAND) {
		if (msg->wParam == SC_MAXIMIZE && mode_ == CustomWindowBorder) {
			maximizeClicked();

			*result = 0;
			return true;
		}
	}

	return OverlayWidget<QFrame, YaWindowExtra>::winEvent(msg, result);
}
#endif

int YaWindowBase::additionalTopMargin() const
{
#ifdef CUSTOM_SHADOW
	return Ya::VisualUtil::windowShadowSize();
#else
	return 0;
#endif
}

int YaWindowBase::additionalBottomMargin() const
{
#ifdef CUSTOM_SHADOW
	return Ya::VisualUtil::windowShadowSize();
#else
	return 0;
#endif
}

int YaWindowBase::additionalLeftMargin() const
{
#ifdef CUSTOM_SHADOW
	return Ya::VisualUtil::windowShadowSize();
#else
	return 0;
#endif
}

int YaWindowBase::additionalRightMargin() const
{
#ifdef CUSTOM_SHADOW
	return Ya::VisualUtil::windowShadowSize();
#else
	return 0;
#endif
}

QRect YaWindowBase::yaContentsRect() const
{
	return QRect(additionalLeftMargin(),
	             additionalTopMargin(),
	             width() - additionalLeftMargin() - additionalRightMargin(),
	             height() - additionalTopMargin() - additionalBottomMargin());
}

QRect YaWindowBase::contentsGeometryToFrameGeometry(const QRect& g) const
{
	return QRect(g.left() - additionalLeftMargin(),
	             g.top() - additionalTopMargin(),
	             g.width() + additionalLeftMargin() + additionalRightMargin(),
	             g.height() + additionalTopMargin() + additionalBottomMargin());
}

QRect YaWindowBase::frameGeometryToContentsGeometry(const QRect& g) const
{
	return QRect(g.left() + additionalLeftMargin(),
	             g.top() + additionalTopMargin(),
	             g.width() - additionalLeftMargin() - additionalRightMargin(),
	             g.height() - additionalTopMargin() - additionalBottomMargin());
}

bool YaWindowBase::isMoveOperationActive() const
{
	return currentOperation_ == Move;
}

void YaWindowBase::interactiveOperationStarted()
{
}

void YaWindowBase::interactiveOperationFinished()
{
}

QPoint YaWindowBase::mousePressPosition() const
{
	return mousePressPosition_;
}

//----------------------------------------------------------------------------
// YaWindow
//----------------------------------------------------------------------------

YaWindow::YaWindow(QWidget* parent, Qt::WindowFlags f)
	: AdvancedWidget<YaWindowBase>(parent, f)
	, activationChangeUpdateTimer_(0)
	, isActiveWindow_(false)
{
	activationChangeUpdateTimer_ = new QTimer(this);
	connect(activationChangeUpdateTimer_, SIGNAL(timeout()), SLOT(activationChangeUpdate()));
	activationChangeUpdateTimer_->setInterval(50);
	activationChangeUpdateTimer_->setSingleShot(true);
}

YaWindow::~YaWindow()
{
}

void YaWindow::setVisible(bool visible)
{
	if (visible && !isVisible() && property("show-offscreen").toBool()) {
		showWidgetOffscreen();
		return;
	}

	AdvancedWidget<YaWindowBase>::setVisible(visible);
}

bool YaWindow::showAsActiveWindow() const
{
	return isActiveWindow_;
}

void YaWindow::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::ActivationChange) {
#ifdef YAPSI_ACTIVEX_SERVER
		activationChangeUpdateTimer_->start();
#else
		activationChangeUpdate();
#endif
		return;
	}

	AdvancedWidget<YaWindowBase>::changeEvent(e);
}

void YaWindow::activationChangeUpdate()
{
	isActiveWindow_ = isActiveWindow();
	QEvent e(QEvent::ActivationChange);
	AdvancedWidget<YaWindowBase>::changeEvent(&e);
}

QTimer* YaWindow::activationChangeUpdateTimer() const
{
	return activationChangeUpdateTimer_;
}
