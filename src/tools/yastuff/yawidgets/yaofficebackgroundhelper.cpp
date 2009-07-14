/*
 * yaofficebackgroundhelper.cpp
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

#include "yaofficebackgroundhelper.h"

#include <QWidget>
#include <QCoreApplication>

#include "psioptions.h"
#include "yavisualutil.h"

YaOfficeBackgroundHelper* YaOfficeBackgroundHelper::instance_ = 0;

YaOfficeBackgroundHelper* YaOfficeBackgroundHelper::instance()
{
	if (!instance_) {
		instance_ = new YaOfficeBackgroundHelper();
	}
	return instance_;
}

YaOfficeBackgroundHelper::YaOfficeBackgroundHelper()
	: QObject(QCoreApplication::instance())
{
	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
}

YaOfficeBackgroundHelper::~YaOfficeBackgroundHelper()
{
}

void YaOfficeBackgroundHelper::registerWidget(QWidget* widget)
{
	Q_ASSERT(widget);
	connect(widget, SIGNAL(destroyed(QObject*)), SLOT(destroyed(QObject*)));
	updateWidget(widget);
	widgets_ << widget;
}

void YaOfficeBackgroundHelper::destroyed(QObject* obj)
{
	QWidget* widget = dynamic_cast<QWidget*>(obj);
	if (!widget)
		return;
	Q_ASSERT(widget);
	Q_ASSERT(widgets_.contains(widget));
	widgets_.remove(widget);
}

void YaOfficeBackgroundHelper::optionChanged(const QString& optionPath)
{
	Q_UNUSED(optionPath);
#if 0
	if (optionPath == "options.ya.office-background") {
		foreach(QPointer<QWidget> widget, widgets_) {
			if (!widget.isNull())
				updateWidget(widget);
		}
	}
#endif
}

void YaOfficeBackgroundHelper::updateWidget(QWidget* widget)
{
	QPalette pal = widget->palette();
	pal.setColor(QPalette::Highlight, Ya::VisualUtil::highlightColor());
#if 0
	pal.setColor(QPalette::Inactive, QPalette::Highlight, QColor(0xE5, 0xE5, 0xE5));
#endif
	widget->setPalette(pal);
}
