/*
 * customtimeline.cpp - a generalization of QTimeLine that could animate different types
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

#include "customtimeline.h"

#include <math.h>

//----------------------------------------------------------------------------
// GTimeLine
//----------------------------------------------------------------------------

GTimeLine::GTimeLine(int duration, QObject *parent)
	: QObject(parent)
{
	timeLine_ = new QTimeLine(duration, this);
	connect(timeLine_, SIGNAL(frameChanged(int)), SIGNAL(frameChanged()));
	connect(timeLine_, SIGNAL(finished()), SLOT(timeLineFinished()));
}

GTimeLine::~GTimeLine()
{
}

void GTimeLine::timeLineFinished()
{
	stop();
	emit finished();
	setCurrentTime(0);
}

QTimeLine::CurveShape GTimeLine::curveShape() const
{
	return timeLine_->curveShape();
}

void GTimeLine::setCurveShape(QTimeLine::CurveShape curveShape)
{
	timeLine_->setCurveShape(curveShape);
}

QTimeLine::Direction GTimeLine::direction() const
{
	return timeLine_->direction();
}

void GTimeLine::setDirection(QTimeLine::Direction direction)
{
	timeLine_->setDirection(direction);
}

int GTimeLine::loopCount() const
{
	return timeLine_->loopCount();
}

void GTimeLine::setLoopCount(int loopCount)
{
	timeLine_->setLoopCount(loopCount);
}

int GTimeLine::updateInterval() const
{
	return timeLine_->updateInterval();
}

void GTimeLine::setUpdateInterval(int updateInterval)
{
	timeLine_->setUpdateInterval(updateInterval);
}

int GTimeLine::duration() const
{
	return timeLine_->duration();
}

void GTimeLine::setDuration(int duration)
{
	timeLine_->setDuration(duration);
}

int GTimeLine::currentTime() const
{
	return timeLine_->currentTime();
}

void GTimeLine::setCurrentTime(int msec)
{
	return timeLine_->setCurrentTime(msec);
}

QTimeLine::State GTimeLine::state() const
{
	return timeLine_->state();
}

void GTimeLine::resume()
{
	timeLine_->resume();
}

void GTimeLine::setPaused(bool paused)
{
	timeLine_->setPaused(paused);
}

void GTimeLine::start()
{
	setCurrentTime(0);
	timeLine_->start();
}

void GTimeLine::stop()
{
	timeLine_->stop();
}

void GTimeLine::toggleDirection()
{
	timeLine_->toggleDirection();
}

//----------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------

// template <>
// QSize __frameForTime(QTimeLine* timeLine, QTimeLine::Direction direction, QSize start, QSize end, int msec)
// {
// 	return QSize(__frameForTime(timeLine, direction, start.width(), end.width(), msec),
// 	             __frameForTime(timeLine, direction, start.height(), end.height(), msec));
// }
//
// template <>
// int __frameForTime(QTimeLine* timeLine, QTimeLine::Direction direction, int start, int end, int msec)
// {
// 	if (direction == QTimeLine::Forward)
// 		return start + int((end - start) * timeLine->valueForTime(msec));
// 	return start + int(::ceil(double((end - start) * timeLine->valueForTime(msec))));
// }
