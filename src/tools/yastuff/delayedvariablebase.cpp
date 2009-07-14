/*
 * delayedvariablebase.cpp
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

#include "delayedvariablebase.h"

#include <QTimer>

DelayedVariableBase::DelayedVariableBase(QObject* parent, int delay)
	: QObject(parent)
{
	delayTimer_ = new QTimer(this);
	connect(delayTimer_, SIGNAL(timeout()), SLOT(delayTimeout()));
	delayTimer_->setInterval(delay);
	delayTimer_->setSingleShot(true);
}

DelayedVariableBase::~DelayedVariableBase()
{
}

void DelayedVariableBase::startDelay()
{
	delayTimer_->start();
}

void DelayedVariableBase::delayTimeout()
{
	delayTimer_->stop();
	emit valueChanged();
}

int DelayedVariableBase::delay() const
{
	return delayTimer_->interval();
}

void DelayedVariableBase::setDelay(int delay)
{
	delayTimer_->setInterval(delay);
}
