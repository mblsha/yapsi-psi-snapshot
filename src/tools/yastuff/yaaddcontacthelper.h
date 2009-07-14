/*
 * yaaddcontacthelper.h
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

#ifndef YAADDCONTACTHELPER_H
#define YAADDCONTACTHELPER_H

#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QTime>

namespace XMPP {
	class Jid;
}

class YaOnline;
class PsiAccount;
class YaContactListModel;
class QRect;
class QTimer;

class YaAddContactHelper : public QObject
{
	Q_OBJECT
public:
	YaAddContactHelper(QObject* parent);
	~YaAddContactHelper();

	void setYaOnline(YaOnline* yaOnline);
	void setAccount(PsiAccount* account);
	void setContactListModel(YaContactListModel* model);

	void addContactClicked(QRect addButtonRect, QRect windowRect);
	void findContact(const QStringList& jids, const QString& type);
	void addContact(const QString& jid, const QString& group);
	void cancelSearch();

protected:
	void foundContact(PsiAccount* account, const XMPP::Jid& jid);
	void searchComplete();

	QStringList groups() const;
	void taskFinished(const XMPP::Jid& jid);

private slots:
	void taskFinished();
	void searchTimerTimeout();
	void ljTimerTimeout();

private:
	QTimer* searchTimer_;
	QTimer* ljTimer_;
	QPointer<YaOnline> yaOnline_;
	QPointer<PsiAccount> account_;
	QPointer<YaContactListModel> contactListModel_;
	QStringList jids_;
	QStringList jidsOriginal_;
	QTime findStartedTime_;

	QString ljHackJid() const;
};

#endif
