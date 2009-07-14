/*
 * yalicense.cpp
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

#include "yalicense.h"

#include <QPainter>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QPointer>
#include <QCoreApplication>
#include <QEventLoop>

#include "yapushbutton.h"
#include "applicationinfo.h"
#include "yapsi_revision.h"
#include "aboutdlg.h"

bool YaLicense::ensureLicenseAccepted()
{
#ifdef YAPSI_ACTIVEX_SERVER
	return true;
#endif

	if (!YaLicense::licenseAccepted()) {
		QPointer<YaLicense> lic = new YaLicense();
		lic->moveToCenterOfScreen();
		lic->show();

		QEventLoop loop;
		connect(lic, SIGNAL(destroyed(QObject*)), &loop, SLOT(quit()));
		loop.exec();
	}

	return YaLicense::licenseAccepted();
}

YaLicense::YaLicense()
	: YaWindow()
	, theme_(YaWindowTheme::Roster)
{
	setAttribute(Qt::WA_DeleteOnClose, true);
	ui_.setupUi(this);
	updateContentsMargins();

	setMinimizeEnabled(false);
	setMaximizeEnabled(false);

	ui_.textBrowser->setText(AboutDlg::loadText(":/COPYING"));
	ui_.textBrowser->setFont(QFont("Arial", 11));

	YaPushButton* accept = new YaPushButton(this);
	accept->setText(tr("Accept"));
	ui_.buttonBox->addButton(accept, QDialogButtonBox::AcceptRole);
	accept->setDefault(true);

	YaPushButton* decline = new YaPushButton(this);
	decline->setText(tr("Decline"));
	ui_.buttonBox->addButton(decline, QDialogButtonBox::RejectRole);

	connect(ui_.buttonBox, SIGNAL(rejected()), SLOT(rejected()));
	connect(ui_.buttonBox, SIGNAL(accepted()), SLOT(accepted()));

	resize(590, height());

	YaPushButton::initAllButtons(this);
}

YaLicense::~YaLicense()
{
}

QString YaLicense::licenseAcceptedFilePath()
{
	return ApplicationInfo::homeDir() + "/license_accepted";
}

bool YaLicense::licenseAccepted()
{
	QFile file(licenseAcceptedFilePath());
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream text(&file);
		text.setCodec("UTF-8");
		QString str = text.readLine();

		QRegExp rx("^\\d+\\.\\d+\\.\\d+\\.\\d+$");
		if (rx.exactMatch(str)) {
			return true;
		}
	}

	return false;
}

void YaLicense::setLicenseAccepted(bool accepted)
{
	if (!accepted) {
		QFile::remove(licenseAcceptedFilePath());
	}
	else {
		QFile file(licenseAcceptedFilePath());
		if (file.open(QIODevice::WriteOnly)) {
			QTextStream text(&file);
			text.setCodec("UTF-8");
			text << QString("%1.%2").arg(YAPSI_VERSION).arg(YAPSI_REVISION);
		}
	}
}

void YaLicense::rejected()
{
	setLicenseAccepted(false);
	close();
}

void YaLicense::accepted()
{
	setLicenseAccepted(true);
	close();
}

const YaWindowTheme& YaLicense::theme() const
{
	return theme_;
}
