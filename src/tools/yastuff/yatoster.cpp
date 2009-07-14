/*
 * yatoster.cpp - custom toaster widget
 * Copyright (C) 2008  Yandex LLC (Alexei Matiouchkine)
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

#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>

#include "yatoster.h"

/* @TODO:
 *		* react on click;
 *		* recalculate size accordingly to contained widget.
 */

const static int IN_TIME = 2000;
const static int SHOW_TIME = 5000;
const static int OUT_TIME = 1000;

const static int TOSTER_WIDTH = 280;
const static int TOSTER_HEIGHT = 100;

/**
 *  We place the toster according to the following rules:
 *  <ul>
 *    <li>if the task panel is horizontal and it's position lets
 *        to show the toster above, it's shown above;</li>
 *    <li>if the task panel is horizontal and it's position does
 *        not let to show the toster above, it's shown below;</li>
 *    <li>if the task panel is not horizontal and it's position lets
 *        to show the toster on the left, it's shown on the left;</li>
 *    <li>the toster is shown on the right otherwise.</li>
 *  </ul>
 *  @param w — the width of the controlled toster
 *  @param h — the height of the controlled toster
 *
 *  Hint:
 *  <code>
 *  #ifdef Q_WS_WIN<br/>
 *  APPBARDATA abd;<br/>
 *  abd.cbSize = sizeof(abd);<br/>
 *  SHAppBarMessage(ABM_GETTASKBARPOS, &abd);<br/>
 *  #endif // Q_WS_WIN
 *  </code>
 */
const YaToster::Data YaToster::getTosterData(int w, int h)
{
	QRect dFull = QApplication::desktop()->screenGeometry();
	QRect d = QApplication::desktop()->availableGeometry();
	YaToster::Data tosterData;
	if (dFull.height() > d.height()) {   // horiz
		if (d.y()) {   // top
			tosterData.from = QRect(QPoint(d.width() - w, d.y()), QSize(w, 0));
			tosterData.to = QRect(QPoint(d.width() - w, d.y()), QSize(w, h));
		}
		else { // bottom only
			tosterData.from = QRect(QPoint(d.width() - w, d.height()), QSize(w, 0));
			tosterData.to = QRect(QPoint(d.width() - w, d.height() - h), QSize(w, h));
		}
	}
	else { // vertic
		if (d.x()) {   // left
			tosterData.from = QRect(QPoint(d.x(), d.height() - h), QSize(0, h));
			tosterData.to = QRect(QPoint(d.x(), d.height() - h), QSize(w, h));
		}
		else { // right
			tosterData.from = QRect(QPoint(d.width(), d.height() - h), QSize(0, h));
			tosterData.to = QRect(QPoint(d.width() - w, d.height() - h), QSize(w, h));
		}
	}

	return tosterData;
}

/**
 * @param stayForever designates, whether the toster should be hidden after showTime
 */
YaToster::YaToster(QWidget* notification, QWidget* parent)
	: QFrame(parent, Qt::ToolTip | Qt::WindowStaysOnTopHint)
	, stayForever_(false)
	, notification_(notification)
	, size_(QSize(TOSTER_WIDTH, TOSTER_HEIGHT))
	, tosterData_(getTosterData(size_.width(), size_.height()))
	, inTime_(IN_TIME)
	, showTime_(SHOW_TIME)
	, outTime_(OUT_TIME)
{
	setWindowModality(Qt::NonModal);
	setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
	setWindowOpacity(0.9);

	notification_->setParent(this);
}

YaToster::~YaToster()
{
	stop();
}

void YaToster::setStayForever(bool stayForever)
{
	stayForever_ = stayForever;
}

// ========================================================================= //
// ====================        Event handlers         ====================== //
// ========================================================================= //

void YaToster::mousePressEvent(QMouseEvent* e)
{
	stop();
	emit clicked();
	QFrame::mousePressEvent(e);
}

// ========================================================================= //
// ==================        Timer handlers         ======================== //
// ========================================================================= //

void YaToster::init()
{
	move(tosterData_.from.topLeft());
	setMinimumSize(tosterData_.from.size());
	setMaximumSize(tosterData_.to.size());
	resize(tosterData_.from.size());
}

void YaToster::start()
{
	tosterData_ = getTosterData(size_.width(), size_.height());
	notification_->move(QPoint(0, 0));
	notification_->setMaximumSize(size_);
	notification_->resize(size_);

	init();
	show();
	started_ = QTime::currentTime();
	connect(&timer_, SIGNAL(timeout()), this, SLOT(onClick()));
	timer_.setInterval((inTime_ + showTime_ + outTime_) / 1000);
	timer_.start();
}

void YaToster::stop()
{
	timer_.stop();
	disconnect(&timer_, SIGNAL(timeout()), this, SLOT(onClick()));
	hide();
	init();
	deleteLater();
}

void YaToster::onClick()
{
	int elapsed = started_.msecsTo(QTime::currentTime());
	if (elapsed < inTime_) { // riding in
		move(
		    tosterData_.from.topLeft().x() + (tosterData_.to.topLeft().x() - tosterData_.from.topLeft().x())* elapsed / inTime_,
		    tosterData_.from.topLeft().y() + (tosterData_.to.topLeft().y() - tosterData_.from.topLeft().y())* elapsed / inTime_
		);
		resize(
		    tosterData_.from.size().width() + (tosterData_.to.size().width() - tosterData_.from.size().width())* elapsed / inTime_,
		    tosterData_.from.size().height() + (tosterData_.to.size().height() - tosterData_.from.size().height())* elapsed / inTime_
		);
	}
	else if (stayForever_ || (elapsed < inTime_ + showTime_)) { // showing static
// 	  These «movings» are superfluous
//		move(tosterData_.to.topLeft());
//		resize(tosterData_.to.size());
	}
	else if (elapsed < inTime_ + showTime_ + outTime_) { // riding out
		move(
		    tosterData_.to.topLeft().x() + (tosterData_.from.topLeft().x() - tosterData_.to.topLeft().x())*(elapsed - inTime_ - showTime_) / outTime_,
		    tosterData_.to.topLeft().y() + (tosterData_.from.topLeft().y() - tosterData_.to.topLeft().y())*(elapsed - inTime_ - showTime_) / outTime_
		);
		resize(
		    tosterData_.to.size().width() + (tosterData_.from.size().width() - tosterData_.to.size().width())*(elapsed - inTime_ - showTime_) / outTime_,
		    tosterData_.to.size().height() + (tosterData_.from.size().height() - tosterData_.to.size().height())*(elapsed - inTime_ - showTime_) / outTime_
		);
	}
	else {
		stop();
	}
}


// ========================================================================= //
// ===============        Getters and setters       ======================== //
// ========================================================================= //

QWidget* YaToster::getNotification() const
{
	return notification_;
}

void YaToster::setNotification(QWidget* notification)
{
	if (notification_ != notification) {
		notification_ = notification;
		emit notificationChanged(notification_);
	}
}

QSize YaToster::getSize() const
{
	return size_;
}

void YaToster::setSize(const QSize & size)
{
	if (size_ != size) {
		size_ = size;
		emit sizeChanged(size_);
	}
}

int YaToster::getInTime() const
{
	return inTime_;
}

int YaToster::getShowTime() const
{
	return showTime_;
}

int YaToster::getOutTime() const
{
	return outTime_;
}

void YaToster::setTimes(const int inTime, const int showTime, const int outTime)
{
	inTime_   = inTime;
	showTime_ = showTime;
	outTime_  = outTime;
	emit timesChanged(inTime_, showTime_, outTime_);
}
