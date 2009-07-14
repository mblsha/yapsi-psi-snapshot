/*
 * yalabel.h 
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

#ifndef YALABEL_H
#define YALABEL_H

#include "m11editablelabel.h"

#include <QList>
#include <QPointer>

#include "yaprofile.h"

class QPaintEvent;
class YaWindowExtra;
class QEvent;

using namespace Ya;

class YaLabel : virtual public M11EditableLabel
{
	Q_OBJECT
public:
	YaLabel(QWidget* parent);

	bool isEventNotifierLabel() const;
	void setIsEventNotifierLabel(bool isEventNotifierLabel);

	void setEffectiveWidth(int effectiveWidth);
	int effectiveWidth() const;

	void setWindowExtra(YaWindowExtra* extra);
	void updateEffectiveWidth(YaWindowExtra* extra);

public:
	// reimplemented
	bool eventFilter(QObject* obj, QEvent* e);
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	virtual void paintEvent(QPaintEvent* e);
	virtual QString text() const;

protected:
	QColor textColor() const;

private:
	int effectiveWidth_;
	bool isEventNotifierLabel_;
	QPointer<YaWindowExtra> windowExtra_;
};

#endif
