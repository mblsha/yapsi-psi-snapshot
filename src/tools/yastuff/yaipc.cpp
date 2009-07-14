/*
 * yaipc.cpp - simple Windows-only inter-process communication
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

#include "yaipc.h"

#include <QCoreApplication>
#include <QWidget>

#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif

#include "applicationinfo.h"

class PsiIPCSingleton : public QWidget
{
	Q_OBJECT
public:
	static PsiIPCSingleton* instance()
	{
		if (!instance_) {
			instance_ = new PsiIPCSingleton();
		}
		return instance_;
	}

	static bool isRunning()
	{
#ifdef Q_WS_WIN
		return findWindow() != 0;
#else
		return false;
#endif
	}

	static void sendMessage(const QString& message)
	{
#ifdef Q_WS_WIN
		// http://www.codeproject.com/thread/ipc_wmcopy.asp
		if (!findWindow())
			return;

		COPYDATASTRUCT cpd;
		cpd.dwData = 0;
		cpd.cbData = sizeof(QChar) * (message.length() + 1);
		cpd.lpData = (void*)message.utf16();
		SendMessage(findWindow(), WM_COPYDATA, 0, (LPARAM)&cpd);
#else
		Q_UNUSED(message);
#endif
	}

	void doConnect(QObject* obj, const char* slot)
	{
		connect(this, SIGNAL(ipcMessage(const QString&)), obj, slot, Qt::QueuedConnection);
	}

signals:
	void ipcMessage(const QString& message);

private:
	PsiIPCSingleton()
		: QWidget()
	{
		connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(deleteLater()));
		setWindowTitle(ipcName());
	}

#ifdef Q_WS_WIN
	bool winEvent(MSG* msg, long* result)
	{
		if (msg->message != WM_COPYDATA)
			return false;

		COPYDATASTRUCT* cpd = (COPYDATASTRUCT*)msg->lParam;
		QString message = QString::fromUtf16((ushort*)cpd->lpData);
		emit ipcMessage(message);

		if (result)
			*result = 0;
		return true;
	}

	static HWND findWindow()
	{
		return FindWindowEx(0, 0, L"QWidget", (TCHAR*)ipcName().utf16());
	}
#endif

	static QString ipcName()
	{
#ifdef YAPSI
		return "YaChat_instance";
#else
		return ApplicationInfo::name() + "_instance";
#endif
	}

	static PsiIPCSingleton* instance_;
};

PsiIPCSingleton* PsiIPCSingleton::instance_ = 0;

bool YaIPC::isRunning()
{
	return PsiIPCSingleton::isRunning();
}

void YaIPC::initIPC()
{
#ifdef Q_WS_WIN
	PsiIPCSingleton* ipc = PsiIPCSingleton::instance();
	Q_ASSERT(ipc);
#endif
}

void YaIPC::sendMessage(const QString& message)
{
	PsiIPCSingleton::sendMessage(message);
}

void YaIPC::connect(QObject* obj, const char* slot)
{
	PsiIPCSingleton::instance()->doConnect(obj, slot);
}

#include "yaipc.moc"
