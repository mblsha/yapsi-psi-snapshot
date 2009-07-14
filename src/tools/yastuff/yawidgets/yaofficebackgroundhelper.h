/*
 * yaofficebackgroundhelper.h
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

#ifndef YAOFFICEBACKGROUNDHELPER_H
#define YAOFFICEBACKGROUNDHELPER_H

#include <QObject>
#include <QWidget>
#include <QPointer>

class QWidget;

class YaOfficeBackgroundHelper : public QObject
{
	Q_OBJECT
public:
	static YaOfficeBackgroundHelper* instance();

	void registerWidget(QWidget* widget);

private slots:
	void destroyed(QObject* obj);
	void optionChanged(const QString&);

private:
	YaOfficeBackgroundHelper();
	~YaOfficeBackgroundHelper();

	static YaOfficeBackgroundHelper* instance_;
	QList<QPointer<QWidget> > widgets_;

	void updateWidget(QWidget* widget);
};

#endif
