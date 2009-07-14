/*
 * yaprogressdialog.h
 * Copyright (C) 2009  Yandex LLC (Michail Pishchagin)
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

#ifndef YAPROGRESSDIALOG_H
#define YAPROGRESSDIALOG_H

#include "yawindow.h"
#include "yawindowtheme.h"

#include "ui_yaprogressdialog.h"

class YaProgressDialog : public YaWindow
{
	Q_OBJECT
public:
	YaProgressDialog(const QString& labelText, const QString& cancelButtonText, int minimum, int maximum, QWidget* parent = 0, Qt::WindowFlags f = 0);
	~YaProgressDialog();

	bool wasCanceled() const;
	void setValue(int value);

	// reimplemented
	virtual const YaWindowTheme& theme() const;

protected:
	// reimplemented
	void closeEvent(QCloseEvent* event);

private:
	Ui::YaProgressDialog ui_;
	YaWindowTheme theme_;
	bool wasCanceled_;
};

#endif
