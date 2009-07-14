/*
 * yaabout.cpp - about dialog
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

#include "yaabout.h"

#include "yapsi_revision.h" // auto-generated file, see src.pro for details

#include <QPainter>

#ifdef YAPSI_ACTIVEX_SERVER
#include <QAxObject>
#endif

YaAbout* YaAbout::instance_ = 0;

void YaAbout::show()
{
#ifdef YAPSI_ACTIVEX_SERVER
	return;
#endif

	if (!instance_)
		instance_ = new YaAbout();
	instance_->moveToCenterOfScreen();
	instance_->setVisible(true);
}

YaAbout::YaAbout()
	: YaWindow()
	, theme_(YaWindowTheme::Roster)
{
	ui_.setupUi(this);
	updateContentsMargins();
	instance_ = this;

	ui_.label->installEventFilter(this);

	QString osName;
#if defined(Q_WS_WIN)
	osName = "Windows";
#elif defined(Q_WS_MAC)
	osName = "Mac OS X";
#else
	osName = "Linux";
#endif
	ui_.appName->setText(ui_.appName->text().arg(osName));

	QString str = tr("%1 (Build %2)").arg(YAPSI_VERSION).arg(YAPSI_REVISION);
	ui_.buildLabel->setText(str);

	setMinimizeEnabled(false);
	setMaximizeEnabled(false);

	YaPushButton::initAllButtons(this);
}

YaAbout::~YaAbout()
{
	instance_ = 0;
}

void YaAbout::forceUpdateCheck()
{
#ifdef YAPSI_ACTIVEX_SERVER
	QAxObject updater("Yandex.YandexUpdater");
	QStringList ids;
	ids << "chat";
	ids << "online";

	foreach(QString id, ids) {
		updater.dynamicCall("ForceUpdateCheck(const QString&)", id);
	}
#endif
}

bool YaAbout::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == ui_.label && e->type() == QEvent::MouseButtonPress) {
		forceUpdateCheck();
	}

	return QWidget::eventFilter(obj, e);
}

const YaWindowTheme& YaAbout::theme() const
{
	return theme_;
}
