/*
 * xmpp_yadatetime.cpp
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

#include "xmpp_yadatetime.h"

XMPP::YaDateTime::YaDateTime()
	: QDateTime()
	, microsec_(0)
{
}

XMPP::YaDateTime::YaDateTime(const QDateTime& dateTime)
	: QDateTime(dateTime)
	, microsec_(0)
{
}

XMPP::YaDateTime::YaDateTime(const YaDateTime& dateTime)
	: QDateTime(dateTime)
	, microsec_(dateTime.microsec_)
{
}

int XMPP::YaDateTime::microsec() const
{
	return microsec_;
}

void XMPP::YaDateTime::setMiscosec(int microsec)
{
	microsec_ = microsec;
}

XMPP::YaDateTime& XMPP::YaDateTime::operator=(const YaDateTime &other)
{
	setDate(other.date());
	setTime(other.time());
	microsec_ = other.microsec_;
	return *this;
}

bool XMPP::YaDateTime::operator==(const YaDateTime& other) const
{
	return (date() == other.date()) &&
	       (time() == other.time()) &&
	       (microsec() == other.microsec());
}

bool XMPP::YaDateTime::operator!=(const YaDateTime& other) const
{
	return !operator==(other);
}

QString XMPP::YaDateTime::toYaTime_t() const
{
	QString msec;
	msec.sprintf("%06d", microsec_);
	QString ts = QString::number(toUTC().toTime_t()) + msec;
	return ts;
}

XMPP::YaDateTime XMPP::YaDateTime::fromYaTime_t(const QString& str)
{
	QString timeStamp = str;
	YaDateTime result;
	if (str.isEmpty())
		return result;
	result.microsec_ = timeStamp.right(6).toInt();
	timeStamp.chop(6);
	QDateTime ts = QDateTime::fromTime_t(timeStamp.toInt());
	// ts.setTimeSpec(Qt::UTC);
	// ts = ts.toLocalTime();
	result.setDate(ts.date());
	result.setTime(ts.time());
	return result;
}

QString XMPP::YaDateTime::toYaIsoTime() const
{
	QString result = toUTC().toString("yyyy-MM-dd HH:mm:ss");

	QString msec;
	msec.sprintf("%06d", microsec_);

	result += "." + msec;
	return result;
}

XMPP::YaDateTime XMPP::YaDateTime::fromYaIsoTime(const QString& str)
{
	YaDateTime result;
	if (str.isEmpty())
		return result;
	QString timeStamp = str;
	if (timeStamp.length() == 26) {
		result.microsec_ = timeStamp.right(6).toInt();
		timeStamp.chop(7);
	}
	else {
		result.microsec_ = 0;
	}

	QDateTime ts = QDateTime::fromString(timeStamp, "yyyy-MM-dd HH:mm:ss");
	ts.setTimeSpec(Qt::UTC);
	ts = ts.toLocalTime();
	result.setDate(ts.date());
	result.setTime(ts.time());
	return result;
}

bool XMPP::YaDateTime::operator<(const YaDateTime& other) const
{
	if (date() != other.date())
		return date() < other.date();
	if (time() != other.time())
		return time() < other.time();
	return microsec_ < other.microsec_;
}

bool XMPP::YaDateTime::operator<=(const YaDateTime& other) const
{
	if (date() != other.date())
		return date() <= other.date();
	if (time() != other.time())
		return time() <= other.time();
	return microsec_ <= other.microsec_;
}

bool XMPP::YaDateTime::operator>(const YaDateTime& other) const
{
	if (date() != other.date())
		return date() > other.date();
	if (time() != other.time())
		return time() > other.time();
	return microsec_ > other.microsec_;
}

bool XMPP::YaDateTime::operator>=(const YaDateTime& other) const
{
	if (date() != other.date())
		return date() >= other.date();
	if (time() != other.time())
		return time() >= other.time();
	return microsec_ >= other.microsec_;
}
