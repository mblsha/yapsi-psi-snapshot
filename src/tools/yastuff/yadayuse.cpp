/*
 * yadayuse.cpp
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

#include "yadayuse.h"

#include <QTimer>
#include <QUuid>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegExp>
#include <QHttp>
#include <QUrl>

#ifdef Q_WS_MAC
extern "C" {
	#include "carboncocoa.h"
}
#endif

#if defined(Q_WS_X11) || defined(Q_WS_MAC)
#include <sys/stat.h> // chmod
#endif

#ifdef YAPSI_ACTIVEX_SERVER
#include <QAxObject>
#endif

#ifdef Q_WS_WIN
#if __GNUC__ >= 3
#	define WINVER    0x0500
#	define _WIN32_IE 0x0500
#endif
#include <windows.h>
#include <shlobj.h>
#endif

#include "psilogger.h"
#include "systeminfo.h"

#include "yapsi_revision.h"

QString YaDayUse::basePingUrl()
{
	return "http://soft.export.yandex.ru/status.xml";
}

QString YaDayUse::appId()
{
	return "chat";
}

QString YaDayUse::ver()
{
	return QString("%1.%2").arg(YAPSI_VERSION).arg(YAPSI_REVISION);
}

QString YaDayUse::osId()
{
#if defined(Q_WS_WIN)
	return "winnt";
#elif defined(Q_WS_MAC)
	return "darwin";
#elif defined(Q_WS_X11)
	return "linux";
#else
	Q_ASSERT(false);
	return "na";
#endif
}

QString YaDayUse::osVer()
{
#if defined(Q_WS_WIN)
	OSVERSIONINFOEX osvi = {0};

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) & osvi);
	if (!bOsVersionInfoEx) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *)&osvi))
			return "na";
	}

	int OsBuildNumber = (int)(osvi.dwBuildNumber & 0xFFFF);
	return QString("%1.%2.%3")
	       .arg(osvi.dwMajorVersion)
	       .arg(osvi.dwMinorVersion)
	       .arg(OsBuildNumber);
#elif defined(Q_WS_MAC)
	return QString::fromUtf8(ownSUCurrentSystemVersionString());
#endif
	// return SystemInfo::instance()->osVersion();
	return "na";
}

YaDayUse::YaDayUse(QObject* parent)
	: QObject(parent)
	, http_(0)
{
	http_ = new QHttp(this);
	connect(http_, SIGNAL(done(bool)), SLOT(httpDone(bool)));

	dayUse();
}

void YaDayUse::dayUse()
{
	PsiLogger::instance()->log("YaDayUse::dayUse()");
	int timeout = 86400;
#ifdef YAPSI_ACTIVEX_SERVER
	// QAxObject updater("Yandex.YandexUpdater");
	// updater.dynamicCall("PingStat(const QString&, const QString&, const QString&)",
	//                     YaDayUse::appId(),
	//                     YaDayUse::basePingUrl(),
	//                     QString::number(timeout));
#else
	QString url = QString("%1?yasoft=%2&ui=%3&ver=%4&os=%5&osver=%6&stat=dayuse")
	              .arg(basePingUrl())
	              .arg(appId())
	              .arg(ui())
	              .arg(ver())
	              .arg(osId())
	              .arg(osVer());

	httpGet(url);
#endif
	QTimer::singleShot(timeout * 1000, this, SLOT(dayUse()));
}

void YaDayUse::httpDone(bool error)
{
	Q_UNUSED(error);
}

void YaDayUse::httpGet(const QString& _url)
{
	QUrl url(_url, QUrl::TolerantMode);
	Q_ASSERT(url.hasQuery());

	QString query = url.encodedQuery();
	query.replace("?", "&"); // FIXME: Bug in Qt 4.4.2?

	QHttpRequestHeader header("GET", url.path() + "?" + query);
	header.setValue("User-Agent", appId());
	header.setValue("Host", url.port() == -1 ? url.host() : QString("%1:%2").arg(url.host(), url.port()));
	header.setValue("Accept-Language", "en-us");
	header.setValue("Accept", "*/*");

	QByteArray bytes;

	int contentLength = bytes.length();
	header.setContentLength(contentLength);

	http_->setHost(url.host(), url.port() == -1 ? 80 : url.port());
	http_->request(header, bytes);
}

QString YaDayUse::uiPath() const
{
#ifdef Q_WS_WIN
	QString base = QDir::homePath();
	WCHAR str[MAX_PATH+1] = { 0 };
	if (SHGetSpecialFolderPathW(0, str, CSIDL_APPDATA, true))
		base = QString::fromWCharArray(str);

	QDir uihome(base + "/Yandex");
	if(!uihome.exists()) {
		QDir home(base);
		home.mkdir("Yandex");
	}
#else
	QDir uihome(QDir::homeDirPath() + "/.yandex");
	if(!uihome.exists()) {
		QDir home = QDir::home();
		home.mkdir(".yandex");
		chmod(QFile::encodeName(uihome.path()), 0700);
	}
#endif
	return uihome.absoluteFilePath("ui");
}

QString YaDayUse::ui()
{
	QFile file(uiPath());
	if (!QFile::exists(uiPath())) {
		QUuid uuid = QUuid::createUuid();

		if (file.open(QIODevice::WriteOnly)) {
			QTextStream text(&file);
			text.setCodec("UTF-8");
			text << uuid.toString();

			return uuid.toString();
		}
	}

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream text(&file);
		text.setCodec("UTF-8");
		QString str = text.readLine();

		QRegExp rx("\\{[\\da-fA-F]{8}(-[\\da-fA-F]{4}){3}-[\\da-fA-F]{12}\\}");
		if (rx.exactMatch(str)) {
			return str;
		}
	}

	return "na";
}
