/*
 * uitest.cpp - simple .ui + .qss file tester
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

#include <QApplication>
#include <QFile>
#include <QUiLoader>
#include <QWidget>

#include "stylesheeteditor.h"
#include "yastyle.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyle(new YaStyle(app.style()));

	if (argc < 2) {
		qWarning("USAGE: uitest file.ui");
		return 1;
	}

	QUiLoader loader;
	loader.addPluginPath("../yawidgets");

	QFile file(argv[1]);
	file.open(QFile::ReadOnly);
	QWidget* widget = loader.load(&file);
	file.close();

	StyleSheetEditor editor;
	editor.show();

	widget->show();

	return app.exec();
}
