/*
 * yaboldmenu.cpp - QMenu subclass where some items could be drawn in bold
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

#include "yaboldmenu.h"

#include <QStyleOptionMenuItem>

YaBoldMenu::YaBoldMenu(QWidget* parent)
	: QMenu(parent)
{
}

YaBoldMenu::~YaBoldMenu()
{
}

void YaBoldMenu::ensureActionBoldText(QAction* action)
{
	action->setProperty("boldFont", true);

	QFont font = action->font();
	font.setBold(true);
	action->setFont(font);

	// TODO: make QMenuPrivate::calcActionRects() work as we want
	action->setText(action->text() + "     ");
}

/**
 * qmenu.h must be patched in order to make initStyleOption virtual,
 * otherwise the bold font magic won't work
 */
void YaBoldMenu::initStyleOption(QStyleOptionMenuItem* option, const QAction* action) const
{
	QMenu::initStyleOption(option, action);
	if (option->text.isEmpty() && !action->property("emptyText").toString().isEmpty()) {
		option->text = action->property("emptyText").toString();
	}

	if (action->property("boldFont").toBool()) {
		option->font.setBold(true);
		option->fontMetrics = QFontMetrics(option->font);
	}
}
