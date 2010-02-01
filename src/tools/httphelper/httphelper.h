/*
 * httphelper.h
 * Copyright (C) 2010  Yandex LLC (Michail Pishchagin)
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

#ifndef HTTPHELPER_H
#define HTTPHELPER_H

class QHttp;
class QString;
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QByteArray;

namespace HttpHelper {
	int httpGet(QHttp* http, const QString& urlString);
	QNetworkRequest getRequest(const QString& url, QNetworkReply* referer = 0);
	QNetworkRequest postFileRequest(const QString& urlString, const QString& fieldName, const QString& fileName, const QByteArray& fileData, QByteArray* postData);
	QNetworkReply* needRedirect(QNetworkAccessManager* network, QNetworkReply* reply, const QByteArray& data);
	QString urlEncode(const QString& str);
	QString urlDecode(const QString& str);
	void debugReply(QNetworkReply* reply);
};

#endif
