/*
 * breakpad.h - google-breakpad wrapper for Qt applications
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

#ifndef BREAKPAD_H
#define BREAKPAD_H

#include <QMap>
#include <QString>

#if defined(Q_WS_WIN) && !defined(Q_CC_GNU)
#define BREAKPAD_STDCALL __stdcall
#else
#define BREAKPAD_STDCALL
#endif

namespace Breakpad {
	void install(const QString& minidumpPath, const QMap<QString, QString>& params);
	void writeMinidump(const QString& suffix);

	int BREAKPAD_STDCALL handleExceptionCallback(void* exceptionInfo);
}

#if defined(Q_WS_WIN) && !defined(Q_CC_GNU)
#define BREAKPAD_TRY \
	__try {
#define BREAKPAD_EXCEPT \
	} \
	__except(Breakpad::handleExceptionCallback(GetExceptionInformation())) { \
		throw; \
	}
#else
#define BREAKPAD_TRY /* noop */
#define BREAKPAD_EXCEPT /* noop */
#endif

#endif
