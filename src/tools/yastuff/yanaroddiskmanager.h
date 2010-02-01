/*
 * yanaroddiskmanager.h
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

#ifndef YANARODDISKMANAGER_H
#define YANARODDISKMANAGER_H

#include <QObject>
#include <QStringList>

class QNetworkAccessManager;
class QNetworkReply;

class YaNarodDiskManager : public QObject
{
	Q_OBJECT
public:
	YaNarodDiskManager(QObject* parent);
	~YaNarodDiskManager();

	int uploadFile(const QString& fileName);

	enum State {
		State_GetTokenizedStorageUrl,
		State_GetStorage,
		State_Upload,
		State_UpdatingStatus,
		State_Finished,
		State_Error
	};

signals:
	void stateChanged(int id, State state);
	void uploadProgress(int id, qint64 bytesSent, qint64 bytesTotal);

private slots:
	void replyFinished(QNetworkReply* reply);
	void replyUploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void tokenizedUrlFinished(int id, const QString& tokenizedUrl);

private:
	QNetworkAccessManager* network_;

	struct Request {
		Request(int _id)
			: id(_id)
			, state(State_Error)
			, tokenizedStorageRequestId(0)
			, reply(0)
		{}

		int id;
		State state;

		int tokenizedStorageRequestId;
		QString uploadUrl;
		QString uploadHash;
		QString uploadProgressUrl;

		QString fileName;
		QNetworkReply* reply;

		QStringList uploadedUrls;
	};

	void processStorageReply(Request* r, QNetworkReply* reply);
	void startUpload(Request* r);
	void startStatusUpdate(Request* r, QNetworkReply* reply);
	void updatingStatus(Request* r, QNetworkReply* reply);

	int lastId_;
	QList<Request> requests_;
};

#endif
