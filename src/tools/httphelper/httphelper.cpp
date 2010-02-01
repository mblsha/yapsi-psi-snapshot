/*
 * httphelper.cpp
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

#include "httphelper.h"

#include <QHttp>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUuid>

#define USE_APPLICATIONINFO

#ifdef USE_APPLICATIONINFO
#include "applicationinfo.h"
#endif

#include "textutil.h"

namespace HttpHelper {

struct ProcessedUrl {
	ProcessedUrl(const QUrl& _url, const QString& _fullUri)
		: url(_url)
		, fullUri(_fullUri)
	{}

	QUrl url;
	QString fullUri;
};

static ProcessedUrl processUrl(const QString& urlString)
{
	QUrl url(urlString, QUrl::TolerantMode);
	// Q_ASSERT(url.hasQuery());

	QString query = url.encodedQuery();
	query.replace("?", "&"); // FIXME: Bug in Qt 4.4.2?

	QString fullUri = url.path();
	if (!query.isEmpty()) {
		fullUri += "?" + query;
	}

	return ProcessedUrl(url, fullUri);
}

static QString userAgent()
{
#ifdef YAPSI
	return QString("Ya.Online %1").arg(ApplicationInfo::version());
#elif defined(USE_APPLICATIONINFO)
	return QString("%1 %2")
	       .arg(ApplicationInfo::name())
	       .arg(ApplicationInfo::version());
#else
	return "QHttp";
#endif
}

// adapted from afisha-cinema/httphelpers.cpp
int httpGet(QHttp* http, const QString& urlString)
{
	ProcessedUrl url = processUrl(urlString);

	QHttpRequestHeader header("GET", url.fullUri);
	header.setValue("User-Agent", userAgent());
	header.setValue("Host", QString("%1:%2").arg(url.url.host(), url.url.port()));
	header.setValue("Accept-Language", "en-us");
	header.setValue("Accept", "*/*");

	QByteArray content;
	header.setContentLength(content.length());

	Q_ASSERT(url.url.scheme() == "https");
	http->setHost(url.url.host(), QHttp::ConnectionModeHttps,
	              443);
	return http->request(header, content);
}

QNetworkRequest getRequest(const QString& urlString, QNetworkReply* referer)
{
	ProcessedUrl url = processUrl(urlString);

	QNetworkRequest result(url.url);
	result.setRawHeader("User-Agent", userAgent().toUtf8());

	if (referer) {
		result.setRawHeader("Referer", referer->url().toEncoded());
	}

	return result;
}

static QString getBoundaryString(const QString& data)
{
	QString uuid;
	while (true) {
		uuid = QUuid::createUuid().toString().replace(QRegExp("\\{|\\-|\\}"), "");

		if (!data.contains(uuid.toUtf8())) {
			break;
		}
	}

	return uuid;
}

QNetworkRequest postFileRequest(const QString& urlString, const QString& fieldName, const QString& fileName, const QByteArray& fileData, QByteArray* postData)
{
	QString boundary = getBoundaryString(fileData);

	postData->append("--" + boundary + "\r\n");
	postData->append("Content-Disposition: form-data; name=\"" + TextUtil::escape(fieldName) +
	                "\"; filename=\"" + TextUtil::escape(fileName.toUtf8()) + "\"\r\n");
	postData->append("Content-Type: application/octet-stream\r\n");
	postData->append("\r\n");
	postData->append(fileData);
	postData->append("\r\n--" + boundary + "--\r\n");

	ProcessedUrl url = processUrl(urlString);

	QNetworkRequest result(url.url);
	result.setRawHeader("User-Agent", userAgent().toUtf8());
	result.setRawHeader("Content-Type", "multipart/form-data, boundary=" + boundary.toLatin1());
	result.setRawHeader("Content-Length", QString::number(postData->length()).toUtf8());
	return result;
}

QNetworkReply* needRedirect(QNetworkAccessManager* network, QNetworkReply* reply, const QByteArray& data)
{
	static QRegExp windowLocationReplaceRx("window\\.location\\.replace\\(\\\"(.+)\\\"\\);");
	windowLocationReplaceRx.setMinimal(true);

	// HttpHelper::debugReply(reply);

	QString body = QString::fromUtf8(data);
	if (reply->hasRawHeader("Location")) {
		QString url = HttpHelper::urlDecode(QString::fromUtf8(reply->rawHeader("Location")));
		return network->get(HttpHelper::getRequest(url));
	}
	else if (windowLocationReplaceRx.indexIn(body) != -1) {
		QString url = HttpHelper::urlDecode(windowLocationReplaceRx.capturedTexts().last());
		return network->get(HttpHelper::getRequest(url));
	}

	return 0;
}

QString urlEncode(const QString& str)
{
	return QString::fromUtf8(QUrl::toPercentEncoding(str.toUtf8()));
}

QString urlDecode(const QString& str)
{
	return QString::fromUtf8(QUrl::fromPercentEncoding(str.toUtf8()));
}

void debugReply(QNetworkReply* reply)
{
	qWarning("*** %s / %s", reply->url().toEncoded().constData(), qPrintable(reply->errorString()));

	foreach(const QByteArray& header, reply->request().rawHeaderList()) {
		qWarning("\t%s: %s", header.constData(), reply->request().rawHeader(header).constData());
	}

	qWarning(" ");

	foreach(const QByteArray& header, reply->rawHeaderList()) {
		qWarning("\t%s: %s", header.constData(), reply->rawHeader(header).constData());
	}
	qWarning("***\n");
}

}; // namespace HttpHelper
