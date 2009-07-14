/*
 * pongserver.cpp - XMPP Ping server
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

#include "pongserver.h"
#include "xmpp_xmlcommon.h"

using namespace XMPP;

/**
 * \class PongTask
 * \brief Sends ping request to jid and listens for pongs
 */

PingTask::PingTask(Task* parent)
	: XMPP::Task(parent)
{
}

XMPP::Jid PingTask::jid() const
{
	return jid_;
}

void PingTask::ping(const XMPP::Jid& jid)
{
	jid_ = jid;
}

void PingTask::onGo()
{
	QDomElement iq = createIQ(doc(), "get", jid().full(), id());
	QDomElement ping = doc()->createElement("ping");
	ping.setAttribute("xmlns", "urn:xmpp:ping");
	iq.appendChild(ping);
	send(iq);
}

bool PingTask::take(const QDomElement& x)
{
	if (!iqVerify(x, jid(), id()))
		return false;

	setSuccess(true);
	return true;
}

/**
 * \class PongServer
 * \brief Answers XMPP Pings
 */

PongServer::PongServer(Task* parent)
	: Task(parent)
{}

bool PongServer::take(const QDomElement& e)
{
	if (e.tagName() != "iq" || e.attribute("type") != "get")
		return false;

	bool found = false;
	QDomElement ping = findSubTag(e, "ping", &found);
	if (found && ping.attribute("xmlns") == "urn:xmpp:ping") {
		QDomElement iq = createIQ(doc(), "result", e.attribute("from"), e.attribute("id"));
		send(iq);
		emit serverPing();
		return true;
	}
	return false;
}
