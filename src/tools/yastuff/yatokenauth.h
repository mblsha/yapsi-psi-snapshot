/*
 * yatokenauth.h
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

#ifndef YATOKENAUTH_H
#define YATOKENAUTH_H

#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;
class PsiCon;

class YaTokenAuth : public QObject
{
	Q_OBJECT
public:
	static YaTokenAuth* instance();
	void setController(PsiCon* controller);

	void openYaUrl(const QString& url);
	void openYaUrl(const QString& login, const QString& password, const QString& url);

	int getTokenizedUrl(const QString& url);
	int getTokenizedUrl(const QString& login, const QString& password, const QString& url);

signals:
	void finished(int id, const QString& tokenizedUrl);

private slots:
	void replyFinished(QNetworkReply* reply);

private:
	YaTokenAuth();
	~YaTokenAuth();

	enum Type {
		Type_OpenUrl,
		Type_GetTokenizedUrl
	};

	struct Request {
		Request(int _id, Type _type)
			: id(_id)
			, type(_type)
			, reply(0)
		{}

		int id;
		Type type;
		QString login;
		QString password;
		QString url;
		QNetworkReply* reply;
	};

	int addRequest(Type type, const QString& login, const QString& password, const QString& url);
	QString processRequest(const Request& r, const QByteArray& ba);
	bool getDefaultLogin(QString* login, QString* passw);

	int lastId_;
	QList<Request> requests_;
	QNetworkAccessManager* network_;
	QPointer<PsiCon> controller_;
};

#endif
