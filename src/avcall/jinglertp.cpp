/*
 * Copyright (C) 2009  Barracuda Networks, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "jinglertp.h"

#include <stdio.h>
#include <stdlib.h>
#include <QtCrypto>
#include "iris/netnames.h"
#include "xmpp_client.h"

// TODO: reject offers that don't contain at least one of audio or video
// TODO: support candidate negotiations over the JingleRtpChannel thread
//   boundary, so we can change candidates after the stream is active

static QChar randomPrintableChar()
{
	// 0-25 = a-z
	// 26-51 = A-Z
	// 52-61 = 0-9

	uchar c = QCA::Random::randomChar() % 62;
	if(c <= 25)
		return 'a' + c;
	else if(c <= 51)
		return 'A' + (c - 26);
	else
		return '0' + (c - 52);
}

static QString randomCredential(int len)
{
	QString out;
	for(int n = 0; n < len; ++n)
		out += randomPrintableChar();
	return out;
}

// resolve external address and stun server
// FIXME: when/if our ICE engine supports adding these dynamically, we should
//   not have the lookups block on each other
class Resolver : public QObject
{
	Q_OBJECT

private:
	XMPP::NameResolver dnsA;
	XMPP::NameResolver dnsB;
	QString extHost;
	QString stunHost;
	bool extDone;
	bool stunDone;

public:
	QHostAddress extAddr;
	QHostAddress stunAddr;

	Resolver(QObject *parent = 0) :
		QObject(parent),
		dnsA(parent),
		dnsB(parent)
	{
		connect(&dnsA, SIGNAL(resultsReady(const QList<XMPP::NameRecord> &)), SLOT(dns_resultsReady(const QList<XMPP::NameRecord> &)));
		connect(&dnsA, SIGNAL(error(XMPP::NameResolver::Error)), SLOT(dns_error(XMPP::NameResolver::Error)));

		connect(&dnsB, SIGNAL(resultsReady(const QList<XMPP::NameRecord> &)), SLOT(dns_resultsReady(const QList<XMPP::NameRecord> &)));
		connect(&dnsB, SIGNAL(error(XMPP::NameResolver::Error)), SLOT(dns_error(XMPP::NameResolver::Error)));
	}

	void start(const QString &_extHost, const QString &_stunHost)
	{
		extHost = _extHost;
		stunHost = _stunHost;

		if(!extHost.isEmpty())
		{
			extDone = false;
			dnsA.start(extHost.toLatin1());
		}
		else
			extDone = true;

		if(!stunHost.isEmpty())
		{
			stunDone = false;
			dnsB.start(stunHost.toLatin1());
		}
		else
			stunDone = true;

		if(extDone && stunDone)
			QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
	}

signals:
	void finished();

private slots:
	void dns_resultsReady(const QList<XMPP::NameRecord> &results)
	{
		XMPP::NameResolver *dns = (XMPP::NameResolver *)sender();

		// FIXME: support more than one address?
		QHostAddress addr = results.first().address();

		if(dns == &dnsA)
		{
			extAddr = addr;
			extDone = true;
			tryFinish();
		}
		else // dnsB
		{
			stunAddr = addr;
			stunDone = true;
			tryFinish();
		}
	}

	void dns_error(XMPP::NameResolver::Error e)
	{
		Q_UNUSED(e);

		XMPP::NameResolver *dns = (XMPP::NameResolver *)sender();

		if(dns == &dnsA)
		{
			extDone = true;
			tryFinish();
		}
		else // dnsB
		{
			stunDone = true;
			tryFinish();
		}
	}

private:
	void tryFinish()
	{
		if(extDone && stunDone)
			emit finished();
	}
};

//----------------------------------------------------------------------------
// JingleRtp
//----------------------------------------------------------------------------
class JingleRtpChannelPrivate : public QObject
{
	Q_OBJECT

public:
	JingleRtpChannel *q;

	QMutex m;
	XMPP::Ice176 *iceA;
	XMPP::Ice176 *iceV;
	QTimer *rtpActivityTimer;
	QList<JingleRtp::RtpPacket> in;

	JingleRtpChannelPrivate(JingleRtpChannel *_q);
	~JingleRtpChannelPrivate();

	void setIceObjects(XMPP::Ice176 *_iceA, XMPP::Ice176 *_iceV);
	void restartRtpActivityTimer();

private slots:
	void start();
	void ice_readyRead(int componentIndex);
	void ice_datagramsWritten(int componentIndex, int count);
	void rtpActivity_timeout();
};

class JingleRtpManagerPrivate : public QObject
{
	Q_OBJECT

public:
	JingleRtpManager *q;

	XMPP::Client *client;
	QHostAddress selfAddr;
	QString extHost;
	QString stunHost;
	int stunPort;
	int basePort;
	QList<JingleRtp*> sessions;
	QList<JingleRtp*> pending;
	JT_PushJingleRtp *push_task;

	JingleRtpManagerPrivate(XMPP::Client *_client, JingleRtpManager *_q);
	~JingleRtpManagerPrivate();

	QString createSid(const XMPP::Jid &peer) const;

	void unlink(JingleRtp *sess);

private slots:
	void push_task_incomingRequest(const XMPP::Jid &from, const QString &iq_id, const JingleRtpEnvelope &envelope);
};

class JingleRtpPrivate : public QObject
{
	Q_OBJECT

public:
	JingleRtp *q;

	JingleRtpManagerPrivate *manager;
	bool incoming;
	XMPP::Jid peer;
	QString sid;

	int types;
	QList<JingleRtpPayloadType> localAudioPayloadTypes;
	QList<JingleRtpPayloadType> localVideoPayloadTypes;
	QList<JingleRtpPayloadType> remoteAudioPayloadTypes;
	QList<JingleRtpPayloadType> remoteVideoPayloadTypes;
	int localMaximumBitrate;
	int remoteMaximumBitrate;
	QString audioName;
	QString videoName;

	QString init_iq_id;
	JT_JingleRtp *jt;

	Resolver resolver;
	QHostAddress extAddr;
	QHostAddress stunAddr;
	int stunPort;
	XMPP::Ice176 *iceA;
	XMPP::Ice176 *iceV;
	JingleRtpChannel *rtpChannel;

	class IceStatus
	{
	public:
		bool started;

		QString remoteUfrag;
		QString remotePassword;

		// for queuing up candidates before using them
		QList<XMPP::Ice176::Candidate> localCandidates;
		QList<XMPP::Ice176::Candidate> remoteCandidates;

		QVector<bool> channelsReady;
	};
	IceStatus iceA_status;
	IceStatus iceV_status;

	bool local_media_ready;
	bool prov_accepted;
	bool ice_connected;
	bool session_accepted;
	bool session_activated;
	QTimer *handshakeTimer;
	JingleRtp::Error errorCode;

	JingleRtpPrivate(JingleRtp *_q) :
		QObject(_q),
		q(_q),
		manager(0),
		localMaximumBitrate(-1),
		remoteMaximumBitrate(-1),
		jt(0),
		resolver(this),
		iceA(0),
		iceV(0),
		local_media_ready(false),
		prov_accepted(false),
		ice_connected(false),
		session_accepted(false),
		session_activated(false)
	{
		connect(&resolver, SIGNAL(finished()), SLOT(resolver_finished()));

		handshakeTimer = new QTimer(this);
		connect(handshakeTimer, SIGNAL(timeout()), SLOT(handshake_timeout()));
		handshakeTimer->setSingleShot(true);

		rtpChannel = new JingleRtpChannel;
	}

	~JingleRtpPrivate()
	{
		cleanup();
		manager->unlink(q);

		handshakeTimer->setParent(0);
		handshakeTimer->disconnect(this);
		handshakeTimer->deleteLater();

		delete rtpChannel;
	}

	void startOutgoing()
	{
		local_media_ready = true;

		types = 0;
		if(!localAudioPayloadTypes.isEmpty())
		{
			printf("there are audio payload types\n");
			types |= JingleRtp::Audio;
		}
		if(!localVideoPayloadTypes.isEmpty())
		{
			printf("there are video payload types\n");
			types |= JingleRtp::Video;
		}

		printf("types=%d\n", types);
		resolver.start(manager->extHost, manager->stunHost);
	}

	void accept(int _types)
	{
		types = _types;

		// TODO: cancel away whatever media type is not used

		resolver.start(manager->extHost, manager->stunHost);
	}

	void reject()
	{
		if(!incoming)
		{
			bool ok = true;
			if((types & JingleRtp::Audio) && !iceA_status.started)
				ok = false;
			if((types & JingleRtp::Video) && !iceV_status.started)
				ok = false;

			// we haven't even sent session-initiate
			if(!ok)
			{
				cleanup();
				return;
			}
		}
		else
		{
			// send iq-result if we haven't done so yet
			if(!prov_accepted)
			{
				prov_accepted = true;
				manager->push_task->respondSuccess(peer, init_iq_id);
			}
		}

		JingleRtpEnvelope envelope;
		envelope.action = "session-terminate";
		envelope.sid = sid;
		envelope.reason.condition = JingleRtpReason::Gone;

		JT_JingleRtp *task = new JT_JingleRtp(manager->client->rootTask());
		task->request(peer, envelope);
		task->go(true);

		cleanup();
	}

	void localMediaUpdate()
	{
		local_media_ready = true;

		tryAccept();
		tryActivated();
	}

	// called by manager when request is received, including
	//   session-initiate.
	// note: manager will never send session-initiate twice.
	// return value only matters for session-initiate
	bool incomingRequest(const QString &iq_id, const JingleRtpEnvelope &envelope)
	{
		// TODO: jingle has a lot of fields, and we kind of skip over
		//   most of them just to grab what we need.  perhaps in the
		//   future we could do more integrity checking.

		if(envelope.action == "session-initiate")
		{
			// initially flag both types, so we don't drop any
			//   transport-info before we accept (at which point
			//   we specify what types we actually want)
			types = JingleRtp::Audio | JingleRtp::Video;

			init_iq_id = iq_id;

			const JingleRtpContent *audioContent = 0;
			const JingleRtpContent *videoContent = 0;

			// find content
			foreach(const JingleRtpContent &c, envelope.contentList)
			{
				if((types & JingleRtp::Audio) && c.desc.media == "audio" && !audioContent)
				{
					audioContent = &c;
					break;
				}
				else if((types & JingleRtp::Video) && c.desc.media == "video" && !videoContent)
				{
					videoContent = &c;
					break;
				}
			}

			if(audioContent)
			{
				audioName = audioContent->name;
				remoteAudioPayloadTypes = audioContent->desc.payloadTypes;
				if(!audioContent->trans.user.isEmpty())
				{
					iceA_status.remoteUfrag = audioContent->trans.user;
					iceA_status.remotePassword = audioContent->trans.pass;
				}
				iceA_status.remoteCandidates += audioContent->trans.candidates;
			}
			if(videoContent)
			{
				videoName = videoContent->name;
				remoteVideoPayloadTypes = videoContent->desc.payloadTypes;
				if(!videoContent->trans.user.isEmpty())
				{
					iceV_status.remoteUfrag = videoContent->trans.user;
					iceV_status.remotePassword = videoContent->trans.pass;
				}
				iceV_status.remoteCandidates += videoContent->trans.candidates;
			}

			// must offer at least one audio or video payload
			if(remoteAudioPayloadTypes.isEmpty() && remoteVideoPayloadTypes.isEmpty())
				return false;
		}
		else if(envelope.action == "session-accept" && !incoming)
		{
			const JingleRtpContent *audioContent = 0;
			const JingleRtpContent *videoContent = 0;

			// find content
			foreach(const JingleRtpContent &c, envelope.contentList)
			{
				if((types & JingleRtp::Audio) && c.desc.media == "audio" && c.name == audioName && !audioContent)
				{
					audioContent = &c;
					break;
				}
				else if((types & JingleRtp::Video) && c.desc.media == "video" && c.name == videoName && !videoContent)
				{
					videoContent = &c;
					break;
				}
			}

			// we support audio, peer doesn't
			if((types & JingleRtp::Audio) && !audioContent)
				types &= ~JingleRtp::Audio;

			// we support video, peer doesn't
			if((types & JingleRtp::Video) && !videoContent)
				types &= ~JingleRtp::Video;

			if(types == 0)
			{
				reject();
				emit q->rejected();
				return false;
			}

			if(audioContent)
			{
				remoteAudioPayloadTypes = audioContent->desc.payloadTypes;
				if(!audioContent->trans.user.isEmpty())
				{
					iceA_status.remoteUfrag = audioContent->trans.user;
					iceA_status.remotePassword = audioContent->trans.pass;
				}
				iceA_status.remoteCandidates += audioContent->trans.candidates;
			}
			if(videoContent)
			{
				remoteVideoPayloadTypes = videoContent->desc.payloadTypes;
				if(!videoContent->trans.user.isEmpty())
				{
					iceV_status.remoteUfrag = videoContent->trans.user;
					iceV_status.remotePassword = videoContent->trans.pass;
				}
				iceV_status.remoteCandidates += videoContent->trans.candidates;
			}

			restartHandshakeTimer();

			flushRemoteCandidates();

			session_accepted = true;
			QMetaObject::invokeMethod(this, "after_session_accept", Qt::QueuedConnection);
			emit q->remoteMediaUpdated();
		}
		else if(envelope.action == "session-terminate")
		{
			cleanup();
			emit q->rejected();
		}
		else if(envelope.action == "transport-info")
		{
			const JingleRtpContent *audioContent = 0;
			const JingleRtpContent *videoContent = 0;

			// find content
			foreach(const JingleRtpContent &c, envelope.contentList)
			{
				if((types & JingleRtp::Audio) && c.name == audioName && !audioContent)
				{
					audioContent = &c;
					break;
				}
				else if((types & JingleRtp::Video) && c.name == videoName && !videoContent)
				{
					videoContent = &c;
					break;
				}
			}

			if(audioContent)
			{
				if(!audioContent->trans.user.isEmpty())
				{
					iceA_status.remoteUfrag = audioContent->trans.user;
					iceA_status.remotePassword = audioContent->trans.pass;
				}

				printf("audio candidates=%d\n", audioContent->trans.candidates.count());
				iceA_status.remoteCandidates += audioContent->trans.candidates;
			}
			if(videoContent)
			{
				if(!videoContent->trans.user.isEmpty())
				{
					iceV_status.remoteUfrag = videoContent->trans.user;
					iceV_status.remotePassword = videoContent->trans.pass;
				}

				printf("video candidates=%d\n", videoContent->trans.candidates.count());
				iceV_status.remoteCandidates += videoContent->trans.candidates;
			}

			// don't process the candidates unless our ICE engine
			//   is started
			if(prov_accepted)
				flushRemoteCandidates();
		}
		else
			manager->push_task->respondError(peer, iq_id, 400, QString());

		return true;
	}

private:
	void cleanup()
	{
		resolver.disconnect(this);
		handshakeTimer->stop();

		delete jt;
		jt = 0;

		if(iceA)
		{
			iceA->disconnect(this);
			iceA->setParent(0);
			iceA->deleteLater();
			iceA = 0;
		}

		if(iceV)
		{
			iceV->disconnect(this);
			iceV->setParent(0);
			iceV->deleteLater();
			iceV = 0;
		}

		// prevent delivery of events by manager
		peer = XMPP::Jid();
		sid.clear();
	}

	void start_ice()
	{
		stunPort = manager->stunPort;
		if(!stunAddr.isNull() && stunPort > 0)
			printf("STUN service: %s:%d\n", qPrintable(stunAddr.toString()), stunPort);

		if(types & JingleRtp::Audio)
		{
			iceA = new XMPP::Ice176(this);
			setup_ice(iceA);
			if(manager->basePort != -1)
				iceA->setBasePort(manager->basePort);

			iceA_status.started = false;
			iceA_status.channelsReady.resize(2);
			iceA_status.channelsReady[0] = false;
			iceA_status.channelsReady[1] = false;
		}

		if(types & JingleRtp::Video)
		{
			iceV = new XMPP::Ice176(this);
			setup_ice(iceV);
			if(manager->basePort != -1)
				iceV->setBasePort(manager->basePort + 2);

			iceV_status.started = false;
			iceV_status.channelsReady.resize(2);
			iceV_status.channelsReady[0] = false;
			iceV_status.channelsReady[1] = false;
		}

		XMPP::Ice176::Mode m;
		if(!incoming)
			m = XMPP::Ice176::Initiator;
		else
			m = XMPP::Ice176::Responder;

		if(iceA)
		{
			printf("starting ice for audio\n");
			iceA->start(m);
		}
		if(iceV)
		{
			printf("starting ice for video\n");
			iceV->start(m);
		}
	}

	void setup_ice(XMPP::Ice176 *ice)
	{
		connect(ice, SIGNAL(started()), SLOT(ice_started()));
		connect(ice, SIGNAL(error()), SLOT(ice_error()));
		connect(ice, SIGNAL(localCandidatesReady(const QList<XMPP::Ice176::Candidate> &)), SLOT(ice_localCandidatesReady(const QList<XMPP::Ice176::Candidate> &)));
		connect(ice, SIGNAL(componentReady(int)), SLOT(ice_componentReady(int)), Qt::QueuedConnection); // signal is not DOR-SS

		// RTP+RTCP
		ice->setComponentCount(2);

		QList<XMPP::Ice176::LocalAddress> localAddrs;
		XMPP::Ice176::LocalAddress addr;

		// a local address is required to use ice.  however, if
		//   we don't have a local address, we won't handle it as
		//   an error here.  instead, we'll start Ice176 anyway,
		//   which should immediately error back at us.
		if(manager->selfAddr.isNull())
		{
			printf("no self address to use.  this will fail.\n");
			return;
		}

		addr.addr = manager->selfAddr;
		localAddrs += addr;
		ice->setLocalAddresses(localAddrs);

		if(!extAddr.isNull())
		{
			QList<XMPP::Ice176::ExternalAddress> extAddrs;
			XMPP::Ice176::ExternalAddress eaddr;
			eaddr.base = addr;
			eaddr.addr = extAddr;
			extAddrs += eaddr;
			ice->setExternalAddresses(extAddrs);
		}

		if(!stunAddr.isNull() && stunPort > 0)
		{
			// TODO: relay support
			XMPP::Ice176::StunServiceType stunType;
			//if(opt_is_relay)
			//	stunType = XMPP::Ice176::Relay;
			//else
				stunType = XMPP::Ice176::Basic;
			ice->setStunService(stunType, stunAddr, stunPort);
			/*if(!opt_user.isEmpty())
			{
				ice->setStunUsername(opt_user);
				ice->setStunPassword(opt_pass.toUtf8());
			}*/
		}
	}

	// called when all ICE objects are started
	void after_ice_started()
	{
		printf("after_ice_started\n");

		// for outbound, send the session-initiate
		if(!incoming)
		{
			sid = manager->createSid(peer);

			JingleRtpEnvelope envelope;
			envelope.action = "session-initiate";
			envelope.initiator = manager->client->jid().full();
			envelope.sid = sid;

			if(types & JingleRtp::Audio)
			{
				audioName = "A";

				JingleRtpContent content;
				content.creator = "initiator";
				content.name = audioName;
				content.senders = "both";

				content.desc.media = "audio";
				content.desc.payloadTypes = localAudioPayloadTypes;
				content.trans.user = iceA->localUfrag();
				content.trans.pass = iceA->localPassword();

				envelope.contentList += content;
			}

			if(types & JingleRtp::Video)
			{
				videoName = "V";

				JingleRtpContent content;
				content.creator = "initiator";
				content.name = videoName;
				content.senders = "both";

				content.desc.media = "video";
				content.desc.payloadTypes = localVideoPayloadTypes;
				content.trans.user = iceV->localUfrag();
				content.trans.pass = iceV->localPassword();

				envelope.contentList += content;
			}

			jt = new JT_JingleRtp(manager->client->rootTask());
			connect(jt, SIGNAL(finished()), SLOT(task_finished()));
			jt->request(peer, envelope);
			jt->go(true);
		}
		else
		{
			restartHandshakeTimer();

			prov_accepted = true;
			manager->push_task->respondSuccess(peer, init_iq_id);

			flushRemoteCandidates();
		}
	}

	// received iq-result to session-initiate
	void prov_accept()
	{
		prov_accepted = true;

		flushLocalCandidates();
	}

	// received iq-error to session-initiate
	void prov_reject()
	{
		cleanup();
		emit q->rejected();
	}

	void flushLocalCandidates()
	{
		printf("flushing local candidates\n");

		QList<JingleRtpContent> contentList;

		// according to xep-166, creator is always whoever added
		//   the content type, which in our case is always the
		//   initiator

		if((types & JingleRtp::Audio) && !iceA_status.localCandidates.isEmpty())
		{
			JingleRtpContent content;
			//if(!incoming)
				content.creator = "initiator";
			//else
			//	content.creator = "responder";
			content.name = audioName;

			content.trans.user = iceA->localUfrag();
			content.trans.pass = iceA->localPassword();
			content.trans.candidates = iceA_status.localCandidates;
			iceA_status.localCandidates.clear();

			contentList += content;
		}

		if((types & JingleRtp::Video) && !iceV_status.localCandidates.isEmpty())
		{
			JingleRtpContent content;
			//if(!incoming)
				content.creator = "initiator";
			//else
			//	content.creator = "responder";
			content.name = videoName;

			content.trans.user = iceV->localUfrag();
			content.trans.pass = iceV->localPassword();
			content.trans.candidates = iceV_status.localCandidates;
			iceV_status.localCandidates.clear();

			contentList += content;
		}

		if(!contentList.isEmpty())
		{
			JingleRtpEnvelope envelope;
			envelope.action = "transport-info";
			envelope.sid = sid;
			envelope.contentList = contentList;

			JT_JingleRtp *task = new JT_JingleRtp(manager->client->rootTask());
			task->request(peer, envelope);
			task->go(true);
		}
	}

	void flushRemoteCandidates()
	{
		// FIXME: currently, new candidates are ignored after the
		//   session is activated (iceA/iceV are passed to
		//   JingleRtpChannel and our local pointers are nulled).
		//   unfortunately this means we can't upgrade to better
		//   candidates on the fly.

		if(types & JingleRtp::Audio && iceA)
		{
			iceA->setPeerUfrag(iceA_status.remoteUfrag);
			iceA->setPeerPassword(iceA_status.remotePassword);
			if(!iceA_status.remoteCandidates.isEmpty())
			{
				iceA->addRemoteCandidates(iceA_status.remoteCandidates);
				iceA_status.remoteCandidates.clear();
			}
		}

		if(types & JingleRtp::Video && iceV)
		{
			iceV->setPeerUfrag(iceV_status.remoteUfrag);
			iceV->setPeerPassword(iceV_status.remotePassword);
			if(!iceV_status.remoteCandidates.isEmpty())
			{
				iceV->addRemoteCandidates(iceV_status.remoteCandidates);
				iceV_status.remoteCandidates.clear();
			}
		}
	}

	// called when all ICE components are established
	void after_ice_connected()
	{
		if(incoming)
			tryAccept();

		tryActivated();
	}

	void tryAccept()
	{
		if(local_media_ready && ice_connected && !session_accepted)
		{
			JingleRtpEnvelope envelope;
			envelope.action = "session-accept";
			envelope.responder = manager->client->jid().full();
			envelope.sid = sid;

			if(types & JingleRtp::Audio)
			{
				JingleRtpContent content;
				content.creator = "initiator";
				content.name = audioName;
				content.senders = "both";

				content.desc.media = "audio";
				content.desc.payloadTypes = localAudioPayloadTypes;
				content.trans.user = iceA->localUfrag();
				content.trans.pass = iceA->localPassword();

				envelope.contentList += content;
			}

			if(types & JingleRtp::Video)
			{
				JingleRtpContent content;
				content.creator = "initiator";
				content.name = videoName;
				content.senders = "both";

				content.desc.media = "video";
				content.desc.payloadTypes = localVideoPayloadTypes;
				content.trans.user = iceV->localUfrag();
				content.trans.pass = iceV->localPassword();

				envelope.contentList += content;
			}

			session_accepted = true;

			JT_JingleRtp *task = new JT_JingleRtp(manager->client->rootTask());
			task->request(peer, envelope);
			task->go(true);
		}
	}

	void tryActivated()
	{
		if(session_accepted && ice_connected)
		{
			if(session_activated)
			{
				printf("warning: attempting to activate an already active session\n");
				return;
			}

			printf("activating!\n");
			session_activated = true;
			handshakeTimer->stop();

			if(iceA)
			{
				iceA->disconnect(this);
				iceA->setParent(0);
			}
			if(iceV)
			{
				iceV->disconnect(this);
				iceV->setParent(0);
			}

			rtpChannel->d->setIceObjects(iceA, iceV);

			iceA = 0;
			iceV = 0;

			emit q->activated();
		}
	}

	void restartHandshakeTimer()
	{
		// there better be some activity in 10 seconds
		handshakeTimer->start(10000);
	}

private slots:
	void resolver_finished()
	{
		extAddr = resolver.extAddr;
		stunAddr = resolver.stunAddr;

		printf("resolver finished\n");

		start_ice();
	}

	void handshake_timeout()
	{
		reject();
		errorCode = JingleRtp::ErrorTimeout;
		emit q->error();
	}

	void ice_started()
	{
		XMPP::Ice176 *ice = (XMPP::Ice176 *)sender();

		printf("ice_started\n");

		if(ice == iceA)
			iceA_status.started = true;
		else // iceV
			iceV_status.started = true;

		bool ok = true;
		if((types & JingleRtp::Audio) && !iceA_status.started)
			ok = false;
		if((types & JingleRtp::Video) && !iceV_status.started)
			ok = false;

		if(ok)
			after_ice_started();
	}

	void ice_error()
	{
		errorCode = JingleRtp::ErrorICE;
		emit q->error();
	}

	void ice_localCandidatesReady(const QList<XMPP::Ice176::Candidate> &list)
	{
		XMPP::Ice176 *ice = (XMPP::Ice176 *)sender();

		if(ice == iceA)
			iceA_status.localCandidates += list;
		else // iceV
			iceV_status.localCandidates += list;

		printf("local candidate ready\n");

		if(prov_accepted)
			flushLocalCandidates();
	}

	void ice_componentReady(int index)
	{
		XMPP::Ice176 *ice = (XMPP::Ice176 *)sender();

		if(ice == iceA)
		{
			Q_ASSERT(!iceA_status.channelsReady[index]);
			iceA_status.channelsReady[index] = true;
		}
		else // iceV
		{
			Q_ASSERT(!iceV_status.channelsReady[index]);
			iceV_status.channelsReady[index] = true;
		}

		bool allReady = true;

		if(types & JingleRtp::Audio)
		{
			for(int n = 0; n < iceA_status.channelsReady.count(); ++n)
			{
				if(!iceA_status.channelsReady[n])
				{
					allReady = false;
					break;
				}
			}
		}

		if(types & JingleRtp::Video)
		{
			for(int n = 0; n < iceV_status.channelsReady.count(); ++n)
			{
				if(!iceV_status.channelsReady[n])
				{
					allReady = false;
					break;
				}
			}
		}

		if(allReady)
		{
			ice_connected = true;
			after_ice_connected();
		}
	}

	void task_finished()
	{
		if(!jt)
			return;

		JT_JingleRtp *task = jt;
		jt = 0;

		if(task->success())
			prov_accept();
		else
			prov_reject();
	}

	void after_session_accept()
	{
		tryActivated();
	}
};

JingleRtp::JingleRtp()
{
	d = new JingleRtpPrivate(this);
}

JingleRtp::~JingleRtp()
{
	delete d;
}

XMPP::Jid JingleRtp::jid() const
{
	return d->peer;
}

QList<JingleRtpPayloadType> JingleRtp::remoteAudioPayloadTypes() const
{
	return d->remoteAudioPayloadTypes;
}

QList<JingleRtpPayloadType> JingleRtp::remoteVideoPayloadTypes() const
{
	return d->remoteVideoPayloadTypes;
}

int JingleRtp::remoteMaximumBitrate() const
{
	return d->remoteMaximumBitrate;
}

void JingleRtp::setLocalAudioPayloadTypes(const QList<JingleRtpPayloadType> &types)
{
	d->localAudioPayloadTypes = types;
}

void JingleRtp::setLocalVideoPayloadTypes(const QList<JingleRtpPayloadType> &types)
{
	d->localVideoPayloadTypes = types;
}

void JingleRtp::setLocalMaximumBitrate(int kbps)
{
	d->localMaximumBitrate = kbps;
}

void JingleRtp::connectToJid(const XMPP::Jid &jid)
{
	d->peer = jid;
	d->startOutgoing();
}

void JingleRtp::accept(int types)
{
	d->accept(types);
}

void JingleRtp::reject()
{
	d->reject();
}

void JingleRtp::localMediaUpdate()
{
	d->localMediaUpdate();
}

JingleRtp::Error JingleRtp::errorCode() const
{
	return d->errorCode;
}

JingleRtpChannel *JingleRtp::rtpChannel()
{
	return d->rtpChannel;
}

//----------------------------------------------------------------------------
// JingleRtpChannel
//----------------------------------------------------------------------------
JingleRtpChannelPrivate::JingleRtpChannelPrivate(JingleRtpChannel *_q) :
	QObject(_q),
	q(_q),
	iceA(0),
	iceV(0)
{
	rtpActivityTimer = new QTimer(this);
	connect(rtpActivityTimer, SIGNAL(timeout()), SLOT(rtpActivity_timeout()));
}

JingleRtpChannelPrivate::~JingleRtpChannelPrivate()
{
	if(iceA)
	{
		iceA->disconnect(this);
		iceA->setParent(0);
		iceA->deleteLater();
	}

	if(iceV)
	{
		iceV->disconnect(this);
		iceV->setParent(0);
		iceV->deleteLater();
	}

	rtpActivityTimer->setParent(0);
	rtpActivityTimer->disconnect(this);
	rtpActivityTimer->deleteLater();
}

void JingleRtpChannelPrivate::setIceObjects(XMPP::Ice176 *_iceA, XMPP::Ice176 *_iceV)
{
	if(QThread::currentThread() != thread())
	{
		// if called from another thread, safely change ownership
		QMutexLocker locker(&m);

		iceA = _iceA;
		iceV = _iceV;

		if(iceA)
		{
			iceA->moveToThread(thread());
			connect(iceA, SIGNAL(readyRead(int)), SLOT(ice_readyRead(int)));
			connect(iceA, SIGNAL(datagramsWritten(int, int)), SLOT(ice_datagramsWritten(int, int)));
		}

		if(iceV)
		{
			iceV->moveToThread(thread());
			connect(iceV, SIGNAL(readyRead(int)), SLOT(ice_readyRead(int)));
			connect(iceV, SIGNAL(datagramsWritten(int, int)), SLOT(ice_datagramsWritten(int, int)));
		}

		QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
	}
	else
	{
		iceA = _iceA;
		iceV = _iceV;

		if(iceA)
		{
			connect(iceA, SIGNAL(readyRead(int)), SLOT(ice_readyRead(int)));
			connect(iceA, SIGNAL(datagramsWritten(int, int)), SLOT(ice_datagramsWritten(int, int)));
		}

		if(iceV)
		{
			connect(iceV, SIGNAL(readyRead(int)), SLOT(ice_readyRead(int)));
			connect(iceV, SIGNAL(datagramsWritten(int, int)), SLOT(ice_datagramsWritten(int, int)));
		}

		start();
	}
}

void JingleRtpChannelPrivate::restartRtpActivityTimer()
{
	// if we go 5 seconds without an RTP packet, then that's
	//   pretty bad
	rtpActivityTimer->start(5000);
}

void JingleRtpChannelPrivate::start()
{
	if(iceA)
		iceA->setParent(this);
	if(iceV)
		iceV->setParent(this);
	restartRtpActivityTimer();
}

void JingleRtpChannelPrivate::ice_readyRead(int componentIndex)
{
	XMPP::Ice176 *ice = (XMPP::Ice176 *)sender();

	if(ice == iceA && componentIndex == 0)
		restartRtpActivityTimer();

	if(ice == iceA)
	{
		while(iceA->hasPendingDatagrams(componentIndex))
		{
			JingleRtp::RtpPacket packet;
			packet.type = JingleRtp::Audio;
			packet.portOffset = componentIndex;
			packet.value = iceA->readDatagram(componentIndex);
			in += packet;
		}
	}
	else // iceV
	{
		while(iceV->hasPendingDatagrams(componentIndex))
		{
			JingleRtp::RtpPacket packet;
			packet.type = JingleRtp::Video;
			packet.portOffset = componentIndex;
			packet.value = iceV->readDatagram(componentIndex);
			in += packet;
		}
	}

	emit q->readyRead();
}

void JingleRtpChannelPrivate::ice_datagramsWritten(int componentIndex, int count)
{
	Q_UNUSED(componentIndex);

	emit q->packetsWritten(count);
}

void JingleRtpChannelPrivate::rtpActivity_timeout()
{
	printf("warning: 5 seconds passed without receiving audio RTP\n");
}

JingleRtpChannel::JingleRtpChannel()
{
	d = new JingleRtpChannelPrivate(this);
}

JingleRtpChannel::~JingleRtpChannel()
{
	delete d;
}

bool JingleRtpChannel::packetsAvailable() const
{
	return !d->in.isEmpty();
}

JingleRtp::RtpPacket JingleRtpChannel::read()
{
	return d->in.takeFirst();
}

void JingleRtpChannel::write(const JingleRtp::RtpPacket &packet)
{
	QMutexLocker locker(&d->m);

	if(packet.type == JingleRtp::Audio && d->iceA)
		d->iceA->writeDatagram(packet.portOffset, packet.value);
	else if(packet.type == JingleRtp::Video && d->iceV)
		d->iceV->writeDatagram(packet.portOffset, packet.value);
}

//----------------------------------------------------------------------------
// JingleRtpManager
//----------------------------------------------------------------------------
JingleRtpManagerPrivate::JingleRtpManagerPrivate(XMPP::Client *_client, JingleRtpManager *_q) :
	QObject(_q),
	q(_q),
	client(_client),
	stunPort(-1),
	basePort(-1)
{
	push_task = new JT_PushJingleRtp(client->rootTask());
	connect(push_task, SIGNAL(incomingRequest(const XMPP::Jid &, const QString &, const JingleRtpEnvelope &)), SLOT(push_task_incomingRequest(const XMPP::Jid &, const QString &, const JingleRtpEnvelope &)));
}

JingleRtpManagerPrivate::~JingleRtpManagerPrivate()
{
	delete push_task;
}

QString JingleRtpManagerPrivate::createSid(const XMPP::Jid &peer) const
{
	while(1)
	{
		QString out = randomCredential(16);

		bool found = false;
		for(int n = 0; n < sessions.count(); ++n)
		{
			if(sessions[n]->d->peer == peer && sessions[n]->d->sid == out)
			{
				found = true;
				break;
			}
		}

		if(!found)
			return out;
	}
}

void JingleRtpManagerPrivate::unlink(JingleRtp *sess)
{
	sessions.removeAll(sess);
}

void JingleRtpManagerPrivate::push_task_incomingRequest(const XMPP::Jid &from, const QString &iq_id, const JingleRtpEnvelope &envelope)
{
	printf("incoming request: [%s]\n", qPrintable(envelope.action));

	// don't allow empty sid
	if(envelope.sid.isEmpty())
	{
		push_task->respondError(from, iq_id, 400, QString());
		return;
	}

	if(envelope.action == "session-initiate")
	{
		int at = -1;
		for(int n = 0; n < sessions.count(); ++n)
		{
			if(sessions[n]->d->peer == from && sessions[n]->d->sid == envelope.sid)
			{
				at = n;
				break;
			}
		}

		// duplicate session
		if(at != -1)
		{
			push_task->respondError(from, iq_id, 400, QString());
			return;
		}

		JingleRtp *sess = new JingleRtp;
		sess->d->manager = this;
		sess->d->incoming = true;
		sess->d->peer = from;
		sess->d->sid = envelope.sid;
		sessions += sess;
		printf("new initiate, from=[%s] sid=[%s]\n", qPrintable(from.full()), qPrintable(envelope.sid));
		if(!sess->d->incomingRequest(iq_id, envelope))
		{
			delete sess;
			push_task->respondError(from, iq_id, 400, QString());
			return;
		}

		pending += sess;
		emit q->incomingReady();
	}
	else
	{
		int at = -1;
		for(int n = 0; n < sessions.count(); ++n)
		{
			if(sessions[n]->d->peer == from && sessions[n]->d->sid == envelope.sid)
			{
				at = n;
				break;
			}
		}

		// session not found
		if(at == -1)
		{
			push_task->respondError(from, iq_id, 400, QString());
			return;
		}

		push_task->respondSuccess(from, iq_id);
		sessions[at]->d->incomingRequest(iq_id, envelope);
	}
}

JingleRtpManager::JingleRtpManager(XMPP::Client *client) :
	QObject(0)
{
	d = new JingleRtpManagerPrivate(client, this);
}

JingleRtpManager::~JingleRtpManager()
{
	delete d;
}

JingleRtp *JingleRtpManager::createOutgoing()
{
	JingleRtp *sess = new JingleRtp;
	sess->d->manager = d;
	sess->d->incoming = false;
	d->sessions += sess;
	return sess;
}

JingleRtp *JingleRtpManager::takeIncoming()
{
	return d->pending.takeFirst();
}

void JingleRtpManager::setSelfAddress(const QHostAddress &addr)
{
	d->selfAddr = addr;
}

void JingleRtpManager::setExternalAddress(const QString &host)
{
	d->extHost = host;
}

void JingleRtpManager::setStunHost(const QString &host, int port)
{
	d->stunHost = host;
	d->stunPort = port;
}

void JingleRtpManager::setBasePort(int port)
{
	d->basePort = port;
}

#include "jinglertp.moc"
