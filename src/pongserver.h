/*
 * pongserver.h - XMPP Ping server
 * Copyright (C) 2007  Maciej Niedzielski
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

#ifndef PONGSERVER_H
#define PONGSERVER_H

#include "xmpp_task.h"
#include "xmpp_jid.h"

class PingTask : public XMPP::Task
{
	Q_OBJECT
public:
	PingTask(Task*);

	void ping(const XMPP::Jid& jid);

	XMPP::Jid jid() const;

	void onGo();
	bool take(const QDomElement&);

private:
	XMPP::Jid jid_;
};

class PongServer : public XMPP::Task
{
	Q_OBJECT
public:
	PongServer(Task*);
	bool take(const QDomElement&);

signals:
	void serverPing();
};

#endif
