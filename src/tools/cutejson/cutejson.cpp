/*
 * cutejson.cpp - lightweight JSON implementation in Qt
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

#include "cutejson.h"

#include <QVariant>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QMapIterator>
#include <QDateTime>

QString CuteJson::variantToJson(const QVariant& variant)
{
	if (variant.type() == QVariant::Map) {
		QStringList result;
		QVariantMap map = variant.toMap();
		QMapIterator<QString, QVariant> it(map);
		while (it.hasNext()) {
			it.next();
			result += QString("%1: %2")
			          .arg(escapedString(it.key()))
			          .arg(variantToJson(it.value()));
		}
		return "{" + result.join(", ") + "}";
	}
	else if (variant.type() == QVariant::List || variant.type() == QVariant::StringList) {
		QStringList result;
		foreach(QVariant v, variant.toList()) {
			result += variantToJson(v);
		}
		return "[" + result.join(", ") + "]";
	}
	else if (variant.type() == QVariant::String) {
		return escapedString(variant.toString());
	}
	else if (variant.type() == QVariant::Bool) {
		return variant.toBool() ? "true" : "false";
	}
	else if (variant.type() == QVariant::DateTime) {
		return QString("\"%1\"").arg(variant.toDateTime().toString(Qt::ISODate));
	}
	else if (variant.type() == QVariant::Int || variant.type() == QVariant::UInt) {
		return QString::number(variant.toInt());
	}
	else if (variant.canConvert(QVariant::Double)) {
		return QString::number(variant.toDouble());
	}

	return "null";
}

QString CuteJson::escapedString(const QString& string)
{
	QString result;
	for (int i = 0; i < string.length(); ++i) {
		const QChar c = string[i];
		if (c == QChar('\"'))
			result += "\\\"";
		else if (c == QChar('\\'))
			result += "\\\\";
		// else if (c == QChar('/'))
		// 	result += "\\/";
		else if (c == QChar('\b'))
			result += "\\b";
		else if (c == QChar('\f'))
			result += "\\f";
		else if (c == QChar('\n'))
			result += "\\n";
		else if (c == QChar('\r'))
			result += "\\r";
		else if (c == QChar('\t'))
			result += "\\t";
		else
			result += c;
	}
	return "\"" + result + "\"";
}
