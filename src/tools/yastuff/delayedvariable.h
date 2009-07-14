/*
 * delayedvariable.h
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

#ifndef DELAYEDVARIABLE_H
#define DELAYEDVARIABLE_H

#include "delayedvariablebase.h"

template <class T>
class DelayedVariable : public DelayedVariableBase
{
public:
	DelayedVariable(QObject* parent = 0, int delay = 100)
		: DelayedVariableBase(parent, delay)
	{}

	T value() const
	{
		return value_;
	}

	void setValue(T value)
	{
		tempValue_ = value;
		startDelay();
	}

	void setValueImmediately(T value)
	{
		setValue(value);
		delayTimeout();
	}

protected slots:
	// reimplemented
	virtual void delayTimeout()
	{
		value_ = tempValue_;
		DelayedVariableBase::delayTimeout();
	}

private:
	T value_;
	T tempValue_;
};

#endif
