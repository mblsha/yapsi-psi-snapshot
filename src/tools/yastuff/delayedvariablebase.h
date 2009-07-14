/*
 * delayedvariablebase.h
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

// this file is separate from delayedvariable.h because moc
// complaints on DelayedVariable class

#ifndef DELAYEDVARIABLEBASE_H
#define DELAYEDVARIABLEBASE_H

#include <QObject>

class QTimer;

class DelayedVariableBase : public QObject
{
	Q_OBJECT
public:
	DelayedVariableBase(QObject* parent, int delay);
	~DelayedVariableBase();

	int delay() const;
	void setDelay(int delay);

signals:
	void valueChanged();

protected slots:
	virtual void startDelay();
	virtual void delayTimeout();

private:
	QTimer* delayTimer_;
};

#endif
