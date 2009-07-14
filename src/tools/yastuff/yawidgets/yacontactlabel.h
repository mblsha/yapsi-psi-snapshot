/*
 * yacontactlabel.h - contact's name widget
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

#ifndef YACONTACTLABEL_H
#define YACONTACTLABEL_H

#include <QList>

#include "yalabel.h"
#include "yaprofile.h"

class PsiAccount;

class YaContactLabel : virtual public YaLabel
{
	Q_OBJECT
public:
	YaContactLabel(QWidget* parent);

public:
	// reimplemented
	virtual QString text() const;

	QColor backgroundColor() const;
	void setBackgroundColor(QColor backgroundColor);

public:
	virtual void setProfile(const YaProfile* profile);
	virtual const YaProfile* profile() const;

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

private:
	YaProfile* profile_;
	QColor backgroundColor_;
};

#endif
