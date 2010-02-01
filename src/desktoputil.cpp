/*
 * desktoputil.cpp - url-opening routines
 * Copyright (C) 2007  Maciej Niedzielski, Michail Pishchagin
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

#include "desktoputil.h"

#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QSysInfo>
#include <QSet>

#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif

#ifdef YAPSI
#include "yatokenauth.h"
#endif

#ifdef Q_WS_WIN
#include <windows.h>

QString defaultBrowser()
{
	QSettings settings("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command", QSettings::NativeFormat);
	QString command = settings.value(".").toString();
	QRegExp rx("\"(.+)\"");
	if (rx.indexIn(command) != -1)
		return rx.capturedTexts()[1];
	return command;
}
#endif

static QSet<QString> handlers;

static bool doOpenUrl(const QUrl& url)
{
#ifdef YAPSI_ACTIVEX_SERVER
	YaOnlineHelper::instance()->openUrl(QString(url.toEncoded()), false);
	return true;
#endif

#ifdef Q_WS_WIN
	if (!handlers.contains(url.scheme())) {
		// on Vista it always returns iexplore.exe as default browser
		bool oldStyleDefaultBrowserInfo = QSysInfo::WindowsVersion < QSysInfo::WV_VISTA;

		QFileInfo browserFileInfo(defaultBrowser());
		if (oldStyleDefaultBrowserInfo && browserFileInfo.fileName() == "iexplore.exe") {
			return QProcess::startDetached(browserFileInfo.absoluteFilePath(),
				QStringList() << "-new" << url.toEncoded());
		}
		else {
			// FIXME: This is necessary for Qt 4.3.3 to handle all URLs correctly
			QT_WA(
				ShellExecuteW(0, 0, (WCHAR *)QString(url.toEncoded()).utf16(), 0, 0, SW_SHOWNORMAL);
			,
				QByteArray a = QString(url.toEncoded()).toLocal8Bit();	// must not call constData() of a temp object
				ShellExecuteA(0, 0, (CHAR *)a.constData(), 0, 0, SW_SHOWNORMAL);
			)
			return true;
		}
	}
#endif
	return QDesktopServices::openUrl(url);
}

/**
 *	\brief Opens URL using OS default handler
 *	\param url the url to open
 *
 *	\a url may be either percent encoded or not.
 *	If it contains only ASCII characters, it is decoded,
 *	else it is converted to QUrl in QUrl::TolerantMode mode.
 *	Resulting QUrl object is passed to QDesktopServices::openUrl().
 *
 *	\sa QDesktopServices::openUrl()
 */
bool DesktopUtil::openUrl(const QString& url)
{
#ifdef YAPSI
	if (yaFilterEmail(url))
		return true;
#endif

#ifdef YAPSI_ACTIVEX_SERVER
	YaOnlineHelper::instance()->openUrl(url, false);
	return true;
#endif

	QByteArray ascii = url.toAscii();
	if (ascii == url)
		return doOpenUrl(QUrl::fromEncoded(ascii));
	else
		return doOpenUrl(QUrl(url, QUrl::TolerantMode));
}

#ifdef YAPSI
bool DesktopUtil::yaFilterEmail(const QString& url)
{
#ifdef YAPSI_ACTIVEX_SERVER
	if (url.startsWith("mailto:")) {
		return openEMail(url.mid(7));
	}
#endif
	return false;
}

bool DesktopUtil::openYaUrl(const QString& url)
{
	if (yaFilterEmail(url))
		return true;
#ifdef YAPSI_ACTIVEX_SERVER
	YaOnlineHelper::instance()->openUrl(url, true);
	return true;
#else
	YaTokenAuth::instance()->openYaUrl(url);
	return true;
#endif
}

bool DesktopUtil::openEMail(const QString& email)
{
#ifdef YAPSI_ACTIVEX_SERVER
	YaOnlineHelper::instance()->openMail(email);
	return true;
#else
	return DesktopUtil::openUrl(
	           QString("mailto:%1").arg(email));
#endif
}
#endif

bool DesktopUtil::openUrl(const QUrl& url)
{
	return doOpenUrl(url);
}

/**
 * \brief Sets the handler for the given \a scheme to be the \a handler method provided by the \a receiver object.
 *
 * Handler is set in QDesktopServices and DesktopUtil will always use QDesktopServices to open URLs of this type.
 *
 * Note: Always manage handlers via DesktopUtil. Using QDestopServices directly may lead to unexpected behavior.
 */
void DesktopUtil::setUrlHandler(const QString& scheme, QObject* receiver, const char* method)
{
	QDesktopServices::setUrlHandler(scheme, receiver, method);
	handlers.insert(scheme);
}

/**
 * Removes a previously set URL handler for the specified \a scheme.
 *
 * Handler is unset in QDesktopServices.
 *
 * Note: Always manage handlers via DesktopUtil. Using QDestopServices directly may lead to unexpected behavior.
 */
void DesktopUtil::unsetUrlHandler(const QString& scheme)
{
	handlers.remove(scheme);
	QDesktopServices::unsetUrlHandler(scheme);
}
