/*
 * yapopupnotification.h - custom popup notifications
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

#ifndef YAPOPUPNOTIFICATION_H
#define YAPOPUPNOTIFICATION_H

#include <QWidget>
#include <QPointer>

class PsiAccount;
class PsiEvent;
class YaEventNotifierFrame;

#include "xmpp_jid.h"
#include "psiactions.h"

class YaPopupNotification : public QWidget
{
	Q_OBJECT
private:
	YaPopupNotification(int id, PsiEvent* event);

public:
	static void notify(int id, PsiEvent* event);
	static void openNotify(int id, PsiAccount* account, const XMPP::Jid& jid, ActivationType activationType);

protected:
	// reimplemented
	void mousePressEvent(QMouseEvent*);

private slots:
	void eventDestroyed();
	void closeToster();
	void clicked();

private:
	YaEventNotifierFrame* eventNotifierFrame_;
	QPointer<PsiAccount> account_;
};

#endif
