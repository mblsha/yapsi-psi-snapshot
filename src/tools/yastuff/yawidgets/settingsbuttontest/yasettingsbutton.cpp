/*
 * yasettingsbutton.cpp - settings gear button
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

#include "yasettingsbutton.h"

#include <QPainter>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QPlastiqueStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include "yavisualutil.h"
#include "pixmaputil.h"
#include "yawindow.h"
#include "yawindowtheme.h"

YaSettingsButtonExtraButton::YaSettingsButtonExtraButton(QWidget* parent)
	: YaWindowExtraButtonBase(parent)
{
	Q_ASSERT(dynamic_cast<QBoxLayout*>(parent->layout()));

	setPopupMode(QToolButton::InstantPopup);

	QVBoxLayout* vbox = new QVBoxLayout(0);
	vbox->addSpacing(4);
	vbox->addWidget(this);

	QBoxLayout* parentLayout = dynamic_cast<QBoxLayout*>(parent->layout());
	Q_ASSERT(parentLayout);
	parentLayout->setSpacing(4);
	parentLayout->insertLayout(0, vbox);

	QSize size(14, 17);
	setFixedSize(size);

	setToolTip(tr("Settings"));
}

QPixmap YaSettingsButtonExtraButton::currentPixmap() const
{
	YaWindow* yaWindow = dynamic_cast<YaWindow*>(window());
	if (yaWindow) {
		return yaWindow->theme().theme().gearButton(underMouse() || isDown());
	}
	return QPixmap();
}
