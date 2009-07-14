/*
 * psilogger.h - a simple logger class
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

#ifndef PSILOGGER_H
#define PSILOGGER_H

#include <QObject>

class QFile;
class QTextStream;

class PsiLogger : public QObject
{
	Q_OBJECT
private:
	static PsiLogger* instance_;

public:
	PsiLogger(const QString& fileName);
	~PsiLogger();

	static PsiLogger* instance();

	void log(const QString& msg);
	void trace(const char* file, int line, const char* func_info);

private:
	QFile* file_;
	QTextStream* stream_;
};

#define LOG_THIS (long)this, 8, 16, QChar('0')
#define LOG_TRACE do {PsiLogger::instance()->trace(__FILE__,__LINE__,Q_FUNC_INFO);} while (0)

#endif
