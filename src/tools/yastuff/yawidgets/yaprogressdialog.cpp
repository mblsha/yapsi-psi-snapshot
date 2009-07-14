/*
 * yaprogressdialog.cpp
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

#include "yaprogressdialog.h"

YaProgressDialog::YaProgressDialog(const QString& labelText, const QString& cancelButtonText, int minimum, int maximum, QWidget* parent, Qt::WindowFlags f)
	: YaWindow()
	, theme_(YaWindowTheme::ProgressDialog)
	, wasCanceled_(false)
{
	ui_.setupUi(this);
	ui_.label->setText(labelText);
	ui_.busyWidget->start();

	setMinimizeEnabled(false);
	setMaximizeEnabled(false);
	setYaFixedSize(true);

	moveToCenterOfScreen();
	setFixedSize(sizeHint());
}

YaProgressDialog::~YaProgressDialog()
{
}

const YaWindowTheme& YaProgressDialog::theme() const
{
	return theme_;
}

void YaProgressDialog::closeEvent(QCloseEvent* event)
{
	YaWindow::closeEvent(event);
	wasCanceled_ = true;
}

bool YaProgressDialog::wasCanceled() const
{
	return wasCanceled_;
}

void YaProgressDialog::setValue(int value)
{
	Q_UNUSED(value);
}
