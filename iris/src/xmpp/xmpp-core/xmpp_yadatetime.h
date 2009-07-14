/*
 * xmpp_yadatetime.h
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

#ifndef XMPP_YADATETIME_H
#define XMPP_YADATETIME_H

#include <QDateTime>

namespace XMPP {

	class YaDateTime : public QDateTime
	{
	public:
		YaDateTime();
		YaDateTime(const QDateTime& dateTime);
		YaDateTime(const YaDateTime& dateTime);

		int microsec() const;
		void setMiscosec(int microsec);

		YaDateTime& operator=(const YaDateTime &other);
		bool operator==(const YaDateTime& other) const;
		bool operator!=(const YaDateTime& other) const;
		bool operator<(const YaDateTime& other) const;
		bool operator<=(const YaDateTime& other) const;
		bool operator>(const YaDateTime& other) const;
		bool operator>=(const YaDateTime& other) const;

		QString toYaTime_t() const;
		static YaDateTime fromYaTime_t(const QString& str);

		QString toYaIsoTime() const;
		static YaDateTime fromYaIsoTime(const QString& str);

	private:
		int microsec_;
	};

}; // namespace XMPP

#include <QMetaType>

Q_DECLARE_METATYPE(XMPP::YaDateTime)

#endif
