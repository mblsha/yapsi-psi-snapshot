/*
 * yadebugconsole.cpp
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

#include "yadebugconsole.h"

#include "common.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psicontact.h"
#include "psicontactlist.h"
#include "lastactivitytask.h"

YaDebugConsole::YaDebugConsole(PsiCon* controller)
	: QWidget()
	, controller_(controller)
{
	ui_.setupUi(this);

	connect(ui_.clear, SIGNAL(clicked()), SLOT(clear()));
	connect(ui_.detectInvisible, SIGNAL(clicked()), SLOT(detectInvisible()));
}

YaDebugConsole::~YaDebugConsole()
{
}

void YaDebugConsole::clear()
{
	ui_.textEdit->clear();
}

void YaDebugConsole::detectInvisible()
{
	PsiAccount* account = controller_->contactList()->yaServerHistoryAccount();
	if (!account || !account->isAvailable())
		return;

	taskList_.clear();
	detectInvisibleStartTime_ = QDateTime::currentDateTime().addSecs(-(60 * 10));
	appendLog(QString("detectInvisible started %1").arg(detectInvisibleStartTime_.toString(Qt::ISODate)));

	foreach(PsiContact* c, account->contactList()) {
		if (!c->isOnline()) {
			// appendLog(QString("%1 is offline").arg(c->jid().full()));

			LastActivityTask* jtLast = new LastActivityTask(c->jid().bare(), account->client()->rootTask());
			connect(jtLast, SIGNAL(finished()), SLOT(detectInvisibleTaskFinished()));
			jtLast->go(true);
			taskList_.append(jtLast);
		}
	}
}

void YaDebugConsole::detectInvisibleTaskFinished()
{
	LastActivityTask *j = static_cast<LastActivityTask*>(sender());
	if (j->success()) {
		if (detectInvisibleStartTime_ <= j->time()) {
			appendLog(QString("finished %1 %2")
			.arg(j->time().toString(Qt::ISODate))
			.arg(j->jid().full())
			);
		}
	}
}

void YaDebugConsole::activate()
{
	::bringToFront(this);
}

void YaDebugConsole::appendLog(const QString& message)
{
	QDateTime now = QDateTime::currentDateTime();
	QString text = QString("[%1] %2").arg(now.toString("hh:mm:ss")).arg(message);
	ui_.textEdit->insertHtml(text);
	ui_.textEdit->insertHtml("<br>");
	ui_.textEdit->verticalScrollBar()->setValue(ui_.textEdit->verticalScrollBar()->maximum());
}
