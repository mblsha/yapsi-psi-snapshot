/*
 * yalastmailinformerview.cpp
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

#include "yalastmailinformerview.h"

#include <QHeaderView>

#include "smoothscrollbar.h"
#include "yavisualutil.h"
#include "yalastmailinformerdelegate.h"
#include "yaofficebackgroundhelper.h"

YaLastMailInformerView::YaLastMailInformerView(QWidget* parent)
	: QTreeView(parent)
{
	setUniformRowHeights(false);
	setAlternatingRowColors(false);
	setRootIsDecorated(false);
	setIndentation(0);
	header()->hide();

	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QAbstractItemDelegate* delegate = itemDelegate();
	delete delegate;

	YaLastMailInformerDelegate* newDelegate = new YaLastMailInformerDelegate();
	newDelegate->setParent(this);
	setItemDelegate(newDelegate);

	YaOfficeBackgroundHelper::instance()->registerWidget(this);

	SmoothScrollBar::install(this);

	setMouseTracking(true);
}
