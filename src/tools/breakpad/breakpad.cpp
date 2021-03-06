/*
 * breakpad.cpp - google-breakpad wrapper for Qt applications
 * Copyright (C) 2008  Michail Pishchagin
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

#include "breakpad.h"

#include <qglobal.h>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QProcess>
#include <QCoreApplication>

#if defined(Q_WS_MAC)
#include "client/mac/handler/exception_handler.h"
#else if defined(Q_WS_WIN)
#include "client/windows/handler/exception_handler.h"
#endif

using google_breakpad::ExceptionHandler;

typedef int (BREAKPAD_STDCALL *BREAKPAD_HandleException)(void*);
extern void qt_breakpad_setHandleExceptionCallback(BREAKPAD_HandleException callback);

namespace Breakpad {

static QMap<QString, QString> params;
static QString debugSuffix;

#if defined(Q_WS_MAC)
bool MDCallback(const char* _dump_dir,
                const char* _minidump_id,
                void *context, bool success)
#else if defined(Q_WS_WIN)
bool MDCallback(const wchar_t* _dump_dir,
                const wchar_t* _minidump_id,
                void* context,
                EXCEPTION_POINTERS* exinfo,
                MDRawAssertionInfo* assertion,
                bool success)
#endif
{
#if defined(Q_WS_MAC)
	QString dump_dir = QString::fromUtf8(_dump_dir);
	QString minidump_id = QString::fromUtf8(_minidump_id);
#else if defined(Q_WS_WIN)
	Q_UNUSED(assertion);
	Q_UNUSED(exinfo);
	Q_UNUSED(context);
	QString dump_dir = QString::fromWCharArray(_dump_dir);
	QString minidump_id = QString::fromWCharArray(_minidump_id);
#endif

	QString minidumpPath = QString("%1/%2.dmp").arg(dump_dir).arg(minidump_id);
	if (!success) {
		qWarning("Error writing minidump: %s", qPrintable(minidumpPath));
		return true;
	}

#if defined(Q_WS_MAC)
	QString os = "darwin";
#else if defined(Q_WS_WIN)
	QString os = "windows";
#endif

	QFileInfo executableFileInfo(QCoreApplication::applicationFilePath());

	QFile minidumpFile(minidumpPath);
	QString newName = QString("%1/%2_%3_%4.dmp")
	                  .arg(dump_dir)
	                  .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hhmmss"))
	                  .arg(QString("%1-%2").arg(executableFileInfo.baseName()).arg(params["version"]))
	                  .arg(os);

	if (!Breakpad::debugSuffix.isEmpty()) {
		qWarning("in Breakpad::writeMinidump(%s)", Breakpad::debugSuffix);
		newName.replace(QRegExp("\\.dmp$"), QString("_%1.dmp").arg(Breakpad::debugSuffix));
	}

	if (minidumpFile.rename(newName)) {
		minidumpPath = newName;
	}

	QFileInfo fi(minidumpPath);
	qWarning("Minidump: %s", qPrintable(fi.absoluteFilePath()));

	if (!Breakpad::debugSuffix.isEmpty()) {
		return true;
	}

	QStringList arg;
	arg << QString("-appPath=%1").arg(executableFileInfo.absoluteFilePath());
	arg << QString("-appName=%1").arg(params["name"]);
	arg << QString("-appVersion=%1").arg(params["version"]);
	arg << QString("-reportURL=%1").arg(params["report-url"]);
	arg << QString("-reportEmail=%1").arg(params["report-email"]);
	arg << QString("-minidump=%1").arg(fi.absoluteFilePath());

#if defined(Q_WS_MAC)
	QString crashReporter = QCoreApplication::applicationDirPath() + "/../Resources/crashreporter";
#else if defined(Q_WS_WIN)
	QString crashReporter = QCoreApplication::applicationDirPath() + "/crashreporter.exe";
#endif
	qWarning("Starting %s", qPrintable(crashReporter));
	QProcess::startDetached(crashReporter, arg);

#ifdef Q_WS_WIN
	TerminateProcess(GetCurrentProcess(), 0);
#endif

	return true;
}

static ExceptionHandler* handler = 0;

void install(const QString& minidumpPath, const QMap<QString, QString>& params)
{
	char* p = getenv("DISABLE_BREAKPAD");
	if (p || handler) {
		return;
	}

	Breakpad::params = params;

#if defined(Q_OS_WIN32) && !defined(Q_CC_GNU)
	qt_breakpad_setHandleExceptionCallback(Breakpad::handleExceptionCallback);
#endif

	handler = new ExceptionHandler(
#if defined(Q_WS_MAC)
	    minidumpPath.toUtf8().data(),
#else if defined(Q_WS_WIN)
	    minidumpPath.toStdWString(),
#endif
	    0, MDCallback, 0, true);
}

void writeMinidump(const QString& suffix)
{
	if (suffix.isEmpty()) {
		qWarning("Breakpad::writeMinidump(): empty suffix.");
		return;
	}

	if (!handler) {
		qWarning("Breakpad::writeMinidump(%s) requested, but breakpad is not initialized.", qPrintable(suffix));
		return;
	}

	Q_ASSERT(Breakpad::debugSuffix.isEmpty());
	Breakpad::debugSuffix = suffix;

	if (!handler->WriteMinidump()) {
		qWarning("Breakpad::writeMinidump(%s): failed.", qPrintable(suffix));
	}

	Breakpad::debugSuffix = QString();
}

int BREAKPAD_STDCALL handleExceptionCallback(void* exceptionInfo)
{
#ifdef Q_WS_WIN
	Q_ASSERT(handler);
	handler->WriteMinidumpForException(static_cast<EXCEPTION_POINTERS*>(exceptionInfo));
	TerminateProcess(GetCurrentProcess(), 0);
	return EXCEPTION_EXECUTE_HANDLER;
#else
	Q_ASSERT(false);
	// TODO
	abort();
#endif
}

} // namespace Breakpad
