/*
 * yanaroddiskmanager.cpp
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

#include "yanaroddiskmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutableListIterator>
#include <QRegExp>
#include <QStringList>
#include <QFile>
#include <QFileInfo>

#include "yatokenauth.h"
#include "httphelper.h"
#include "JsonToVariant.h"

YaNarodDiskManager::YaNarodDiskManager(QObject* parent)
	: QObject(parent)
	, lastId_(0)
{
	network_ = new QNetworkAccessManager(this);
	connect(network_, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));

	connect(YaTokenAuth::instance(), SIGNAL(finished(int, const QString&)), SLOT(tokenizedUrlFinished(int, const QString&)));
}

YaNarodDiskManager::~YaNarodDiskManager()
{
}

int YaNarodDiskManager::uploadFile(const QString& fileName)
{
	qWarning("YaNarodDiskManager::uploadFile %s", qPrintable(fileName));
	Request request(++lastId_);
	request.fileName = fileName;
	request.tokenizedStorageRequestId = YaTokenAuth::instance()->getTokenizedUrl("http://narod.yandex.ru/disk/getstorage");
	request.state = State_GetTokenizedStorageUrl;

	if (request.tokenizedStorageRequestId == -1) {
		qWarning("YaNarodDiskManager::uploadFile(): error");
		return -1;
	}

	requests_ << request;
	return request.id;
}

void YaNarodDiskManager::replyFinished(QNetworkReply* reply)
{
	reply->deleteLater();

	QMutableListIterator<Request> it(requests_);
	while (it.hasNext()) {
		Request r = it.next();
		if (r.reply == reply) {
			// qWarning("YaNarodDiskManager::replyFinished");
			r.reply = 0;

			if (r.state == State_GetStorage) {
				processStorageReply(&r, reply);
			}
			else if (r.state == State_Upload) {
				startStatusUpdate(&r, reply);
			}
			else if (r.state == State_UpdatingStatus) {
				updatingStatus(&r, reply);
			}

			it.setValue(r);
			emit stateChanged(r.id, r.state);
			break;
		}
	}
}

void YaNarodDiskManager::replyUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
	QMutableListIterator<Request> it(requests_);
	while (it.hasNext()) {
		Request r = it.next();
		if (r.reply == reply) {
			emit uploadProgress(r.id, bytesSent, bytesTotal);
			break;
		}
	}
}

void YaNarodDiskManager::processStorageReply(YaNarodDiskManager::Request* r, QNetworkReply* reply)
{
	QByteArray data = reply->readAll();
	QString body = QString::fromUtf8(data);

	QNetworkReply* redirect = HttpHelper::needRedirect(network_, reply, data);
	if (redirect) {
		r->reply = redirect;
	}
	else {
		r->state = State_Error;
		static QRegExp storageRx("getStorage\\((.+)\\);");
		if (storageRx.indexIn(body) != -1) {
			QString jsonStr = storageRx.capturedTexts().last();

			QVariant variant;
			try {
				variant = JsonQt::JsonToVariant::parse(jsonStr);
			}
			catch(...) {
			}
			QVariantMap map = variant.toMap();

			r->uploadUrl = map["url"].toString();
			r->uploadHash = map["hash"].toString();
			r->uploadProgressUrl = map["purl"].toString();
			if (!r->uploadUrl.isEmpty() && !r->uploadHash.isEmpty() && !r->uploadProgressUrl.isEmpty()) {
				startUpload(r);
			}
		}
	}
}

void YaNarodDiskManager::startUpload(Request* r)
{
	QFile file(r->fileName);
	QFileInfo fileInfo(file);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning("YaNarodDiskManager::startUpload(): unable to open file %s", qPrintable(r->fileName));
		return;
	}

	QByteArray fileData = file.readAll();
	file.close();

	QString postUrl = r->uploadUrl + "?tid=" + r->uploadHash;
	QByteArray postData;
	QNetworkRequest request = HttpHelper::postFileRequest(postUrl, "file", fileInfo.fileName(), fileData, &postData);

	r->state = State_Upload;
	r->reply = network_->post(request, postData);
	connect(r->reply, SIGNAL(uploadProgress(qint64, qint64)), SLOT(replyUploadProgress(qint64, qint64)));
}

void YaNarodDiskManager::startStatusUpdate(Request* r, QNetworkReply* reply)
{
	if (reply->error() != QNetworkReply::NoError) {
		r->state = State_Error;
		return;
	}

	r->state = State_UpdatingStatus;
	QString statusUrl = r->uploadProgressUrl + "?tid=" + r->uploadHash;
	r->reply = network_->get(HttpHelper::getRequest(statusUrl));
}

void YaNarodDiskManager::updatingStatus(Request* r, QNetworkReply* reply)
{
	if (reply->error() != QNetworkReply::NoError) {
		r->state = State_Error;
		return;
	}

	QByteArray data = reply->readAll();
	QString body = QString::fromUtf8(data);

	bool done = false;

	static QRegExp progressRx("getProgress\\((.+)\\);");
	if (progressRx.indexIn(body) != -1) {
		QString jsonStr = progressRx.capturedTexts().last();

		QVariant variant;
		try {
			variant = JsonQt::JsonToVariant::parse(jsonStr);
		}
		catch(...) {
		}
		QVariantMap map = variant.toMap();

		QString status = map["status"].toString();
		if (status == "done") {
			done = true;
			QVariantList files = map["files"].toList();
			foreach(const QVariant& file, files) {
				QVariantMap fileMap = file.toMap();
				QString fileUrl = QString("http://narod.ru/disk/%1/%2")
				                  .arg(fileMap["hash"].toString())
				                  .arg(fileMap["name"].toString());

				r->uploadedUrls << fileUrl;
				qWarning("fileUrl = '%s'", qPrintable(fileUrl));
				r->state = State_Finished;
			}
		}
	}

	if (!done) {
		startStatusUpdate(r, reply);
	}
}

void YaNarodDiskManager::tokenizedUrlFinished(int id, const QString& tokenizedUrl)
{
	QMutableListIterator<Request> it(requests_);
	while (it.hasNext()) {
		Request r = it.next();
		if (r.tokenizedStorageRequestId == id) {
			r.tokenizedStorageRequestId = -1;
			if (tokenizedUrl.isEmpty()) {
				r.state = State_Error;
			}
			else {
				r.state = State_GetStorage;
				r.reply = network_->get(HttpHelper::getRequest(tokenizedUrl));
			}

			it.setValue(r);
			emit stateChanged(r.id, r.state);
			break;
		}
	}
}
