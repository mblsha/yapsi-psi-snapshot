/*
 * yaunreadmessagesmanager.h
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

#ifndef YAUNREADMESSAGESMANAGER_H
#define YAUNREADMESSAGESMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QPointer>

class QTimer;

class PsiCon;
class PsiEvent;
namespace XMPP {
	class JT_YaRetrieveHistory;
};

class YaUnreadMessagesManager : public QObject
{
	Q_OBJECT
public:
	YaUnreadMessagesManager(PsiCon* parent);
	~YaUnreadMessagesManager();

	void eventRead(PsiEvent* event);

public slots:
	void checkUnread();
	void secondsIdle(int seconds);

private slots:
	void checkUnreadFinished();

private:
	PsiCon* controller_;
	QTimer* checkUnreadTimer_;
	QDateTime lastCheckTime_;
	int lastSecondsIdle_;
	QPointer<XMPP::JT_YaRetrieveHistory> task_;
};

#endif
