/*
 * yatokenauth.cpp
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

#include "yatokenauth.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMutableListIterator>

#include "httphelper.h"
#include "xmpp_xmlcommon.h"
#include "desktoputil.h"
#include "psicon.h"
#include "psicontactlist.h"
#include "psiaccount.h"

YaTokenAuth* YaTokenAuth::instance()
{
	static YaTokenAuth* instance_ = 0;
	if (!instance_) {
		instance_ = new YaTokenAuth();
	}
	return instance_;
}

YaTokenAuth::YaTokenAuth()
	: QObject(QCoreApplication::instance())
	, lastId_(0)
	, network_(0)
{
	network_ = new QNetworkAccessManager(this);
	connect(network_, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));
}

YaTokenAuth::~YaTokenAuth()
{
}

void YaTokenAuth::setController(PsiCon* controller)
{
	controller_ = controller;
}

void YaTokenAuth::openYaUrl(const QString& url)
{
	QString login;
	QString passw;
	if (getDefaultLogin(&login, &passw)) {
		openYaUrl(login, passw, url);
	}
	else {
		DesktopUtil::openUrl(url);
	}
}

bool YaTokenAuth::getDefaultLogin(QString* login, QString* passw)
{
	if (!controller_)
		return false;

	PsiContactList* contactList = controller_->contactList();
	if (!contactList || !contactList->accountsLoaded())
		return false;

	PsiAccount* account = contactList->yaServerHistoryAccount();
	if (!account)
		return false;

	*login = account->jid().node();
	*passw = account->userAccount().pass;
	return true;
}

int YaTokenAuth::getTokenizedUrl(const QString& url)
{
	QString login;
	QString passw;
	if (getDefaultLogin(&login, &passw)) {
		return getTokenizedUrl(login, passw, url);
	}

	return -1;
}

int YaTokenAuth::getTokenizedUrl(const QString& login, const QString& password, const QString& url)
{
	return addRequest(Type_GetTokenizedUrl, login, password, url);
}

void YaTokenAuth::openYaUrl(const QString& login, const QString& password, const QString& url)
{
	addRequest(Type_OpenUrl, login, password, url);
}

int YaTokenAuth::addRequest(YaTokenAuth::Type type, const QString& login, const QString& password, const QString& url)
{
	Request request(++lastId_, type);
	request.login = login;
	request.password = password;
	request.url = url;

	QString passportUrl = QString("https://passport.yandex.ru/passport?mode=genauthtoken&for=yaonline&login=%1@yandex.ru&passwd=%2")
	                      .arg(HttpHelper::urlEncode(login))
	                      .arg(HttpHelper::urlEncode(password));
	request.reply = network_->get(HttpHelper::getRequest(passportUrl));

	requests_ << request;
	return request.id;
}

void YaTokenAuth::replyFinished(QNetworkReply* reply)
{
	reply->deleteLater();

	QMutableListIterator<Request> it(requests_);
	while (it.hasNext()) {
		Request r = it.next();
		if (r.reply == reply) {
			QByteArray ba = reply->readAll();
			QString fullUrl = processRequest(r, ba);

			if (r.type == Type_GetTokenizedUrl) {
				emit finished(r.id, HttpHelper::urlDecode(fullUrl));
			}
			else if (r.type == Type_OpenUrl) {
				if (!fullUrl.isEmpty()) {
					DesktopUtil::openUrl(fullUrl);
				}
			}

			it.remove();
			break;
		}
	}
}

QString YaTokenAuth::processRequest(const YaTokenAuth::Request& r, const QByteArray& ba)
{
	QDomDocument doc;
	QString errorMsg;
	if (!doc.setContent(ba, false, &errorMsg)) {
		qWarning("YaTokenAuth::processRequest(): error %s", qPrintable(errorMsg));
		return QString();
	}
	QDomElement auth = doc.documentElement();
	QString result = XMLHelper::subTagText(auth, "result");
	if (result != "ok") {
		qWarning("YaTokenAuth::processRequest(): result = %s", qPrintable(result));
		return QString();
	}
	QString url = XMLHelper::subTagText(auth, "url");
	if (url.isEmpty()) {
		qWarning("YaTokenAuth::processRequest(): url is empty");
		return QString();
	}
	QString fullUrl = url + HttpHelper::urlEncode(r.url);
	return fullUrl;
}
