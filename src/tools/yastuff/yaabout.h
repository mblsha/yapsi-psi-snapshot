/*
 * yaabout.h - about dialog
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

#ifndef YAABOUT_H
#define YAABOUT_H

#include "yawindow.h"
#include "yawindowtheme.h"

#include "ui_yaabout.h"

class YaAbout : private YaWindow
{
	Q_OBJECT
public:
	static void show();

	// reimplemented
	virtual const YaWindowTheme& theme() const;

private slots:
	void forceUpdateCheck();

protected:
	// reimplemented
	bool eventFilter(QObject* obj, QEvent* e);

private:
	Ui::YaAbout ui_;
	static YaAbout* instance_;
	YaWindowTheme theme_;

	YaAbout();
	~YaAbout();
};

#endif
