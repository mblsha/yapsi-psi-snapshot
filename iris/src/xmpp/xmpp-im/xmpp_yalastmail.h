/*
 * xmpp_yalastmail.h - storage class for new mail notifications
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

#ifndef XMPP_YALASTMAIL_H
#define XMPP_YALASTMAIL_H

#include <QString>
#include <QDateTime>

class YaLastMail
{
public:
	YaLastMail()
		: from(QString())
		, subject(QString())
		, timeStamp(QDateTime())
		, mid(QString())
	{}

	bool isNull() const
	{
		return from.isNull() && subject.isNull() && timeStamp.isNull() && mid.isNull();
	}

	QString from;
	QString subject;
	QDateTime timeStamp;
	QString mid;
};

#endif
