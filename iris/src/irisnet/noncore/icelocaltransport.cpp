/*
 * Copyright (C) 2009,2010  Barracuda Networks, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 *
 */

#include "icelocaltransport.h"

#include <QHostAddress>
#include <QUdpSocket>
#include <QtCrypto>
#include "objectsession.h"
#include "stunmessage.h"
#include "stuntransaction.h"
#include "stunbinding.h"
#include "stunallocate.h"
#include "turnclient.h"

// don't queue more incoming packets than this per transmit path
#define MAX_PACKET_QUEUE 64

namespace XMPP {

enum
{
	Direct,
	Relayed
};

//----------------------------------------------------------------------------
// SafeUdpSocket
//----------------------------------------------------------------------------
// DOR-safe wrapper for QUdpSocket
class SafeUdpSocket : public QObject
{
	Q_OBJECT

private:
	ObjectSession sess;
	QUdpSocket *sock;
	int writtenCount;

public:
	SafeUdpSocket(QUdpSocket *_sock, QObject *parent = 0) :
		QObject(parent),
		sess(this),
		sock(_sock)
	{
		sock->setParent(this);
		connect(sock, SIGNAL(readyRead()), SLOT(sock_readyRead()));
		connect(sock, SIGNAL(bytesWritten(qint64)), SLOT(sock_bytesWritten(qint64)));

		writtenCount = 0;
	}

	~SafeUdpSocket()
	{
		if(sock)
		{
			QUdpSocket *out = release();
			out->deleteLater();
		}
	}

	QUdpSocket *release()
	{
		sock->disconnect(this);
		sock->setParent(0);
		QUdpSocket *out = sock;
		sock = 0;
		return out;
	}

	QHostAddress localAddress() const
	{
		return sock->localAddress();
	}

	quint16 localPort() const
	{
		return sock->localPort();
	}

	bool hasPendingDatagrams() const
	{
		return sock->hasPendingDatagrams();
	}

	QByteArray readDatagram(QHostAddress *address = 0, quint16 *port = 0)
	{
		if(!sock->hasPendingDatagrams())
			return QByteArray();

		QByteArray buf;
		buf.resize(sock->pendingDatagramSize());
		sock->readDatagram(buf.data(), buf.size(), address, port);
		return buf;
	}

	void writeDatagram(const QByteArray &buf, const QHostAddress &address, quint16 port)
	{
		sock->writeDatagram(buf, address, port);
	}

signals:
	void readyRead();
	void datagramsWritten(int count);

private slots:
	void sock_readyRead()
	{
		emit readyRead();
	}

	void sock_bytesWritten(qint64 bytes)
	{
		Q_UNUSED(bytes);

		++writtenCount;
		sess.deferExclusive(this, "processWritten");
	}

	void processWritten()
	{
		int count = writtenCount;
		writtenCount = 0;

		emit datagramsWritten(count);
	}
};

//----------------------------------------------------------------------------
// IceLocalTransport
//----------------------------------------------------------------------------
class IceLocalTransport::Private : public QObject
{
	Q_OBJECT

public:
	class WriteItem
	{
	public:
		enum Type
		{
			Direct,
			Pool,
			Turn
		};

		Type type;
		QHostAddress addr;
		int port;
	};

	class Written
	{
	public:
		QHostAddress addr;
		int port;
		int count;
	};

	class Datagram
	{
	public:
		QHostAddress addr;
		int port;
		QByteArray buf;
	};

	IceLocalTransport *q;
	ObjectSession sess;
	QUdpSocket *extSock;
	SafeUdpSocket *sock;
	StunTransactionPool *pool;
	StunBinding *stunBinding;
	TurnClient *turn;
	bool turnActivated;
	QHostAddress addr;
	int port;
	QHostAddress refAddr;
	int refPort;
	QHostAddress relAddr;
	int relPort;
	QHostAddress stunAddr;
	int stunPort;
	IceLocalTransport::StunServiceType stunType;
	QString stunUser;
	QCA::SecureArray stunPass;
	QString clientSoftware;
	QList<Datagram> in;
	QList<Datagram> inRelayed;
	QList<WriteItem> pendingWrites;
	int retryCount;
	bool stopping;

	Private(IceLocalTransport *_q) :
		QObject(_q),
		q(_q),
		sess(this),
		extSock(0),
		sock(0),
		pool(0),
		stunBinding(0),
		turn(0),
		turnActivated(false),
		port(-1),
		refPort(-1),
		relPort(-1),
		retryCount(0),
		stopping(false)
	{
	}

	~Private()
	{
		reset();
	}

	void reset()
	{
		sess.reset();

		delete stunBinding;
		stunBinding = 0;

		delete turn;
		turn = 0;
		turnActivated = false;

		if(sock)
		{
			if(extSock)
			{
				sock->release();
				extSock = 0;
			}

			delete sock;
			sock = 0;
		}

		addr = QHostAddress();
		port = -1;

		refAddr = QHostAddress();
		refPort = -1;

		relAddr = QHostAddress();
		relPort = -1;

		in.clear();
		inRelayed.clear();
		pendingWrites.clear();

		retryCount = 0;
		stopping = false;
	}

	void start()
	{
		Q_ASSERT(!sock);

		sess.defer(this, "postStart");
	}

	void stop()
	{
		Q_ASSERT(sock);
		Q_ASSERT(!stopping);

		stopping = true;

		if(turn)
			turn->close();
		else
			sess.defer(this, "postStop");
	}

	void stunStart()
	{
		Q_ASSERT(!pool);

		pool = new StunTransactionPool(StunTransaction::Udp, this);
		connect(pool, SIGNAL(outgoingMessage(const QByteArray &, const QHostAddress &, int)), SLOT(pool_outgoingMessage(const QByteArray &, const QHostAddress &, int)));
		connect(pool, SIGNAL(needAuthParams()), SLOT(pool_needAuthParams()));

		pool->setLongTermAuthEnabled(true);
		if(!stunUser.isEmpty())
		{
			pool->setUsername(stunUser);
			pool->setPassword(stunPass);
		}

		bool useStun = false;
		bool useTurn = false;
		if(stunType == IceLocalTransport::Basic)
			useStun = true;
		else if(stunType == IceLocalTransport::Relay)
			useTurn = true;
		else // Auto
		{
			useStun = true;
			useTurn = true;
		}

		if(useStun)
		{
			stunBinding = new StunBinding(pool);
			connect(stunBinding, SIGNAL(success()), SLOT(binding_success()));
			connect(stunBinding, SIGNAL(error(XMPP::StunBinding::Error)), SLOT(binding_error(XMPP::StunBinding::Error)));
			stunBinding->start();
		}

		if(useTurn)
		{
			do_turn();
		}
	}

	void do_turn()
	{
		turn = new TurnClient(this);
		connect(turn, SIGNAL(connected()), SLOT(turn_connected()));
		connect(turn, SIGNAL(tlsHandshaken()), SLOT(turn_tlsHandshaken()));
		connect(turn, SIGNAL(closed()), SLOT(turn_closed()));
		connect(turn, SIGNAL(activated()), SLOT(turn_activated()));
		connect(turn, SIGNAL(packetsWritten(int, const QHostAddress &, int)), SLOT(turn_packetsWritten(int, const QHostAddress &, int)));
		connect(turn, SIGNAL(error(XMPP::TurnClient::Error)), SLOT(turn_error(XMPP::TurnClient::Error)));
		connect(turn, SIGNAL(outgoingDatagram(const QByteArray &)), SLOT(turn_outgoingDatagram(const QByteArray &)));
		connect(turn, SIGNAL(debugLine(const QString &)), SLOT(turn_debugLine(const QString &)));

		turn->setClientSoftwareNameAndVersion(clientSoftware);

		turn->connectToHost(pool);
	}

private:
	// note: emits signal on error
	QUdpSocket *createSocket()
	{
		QUdpSocket *qsock = new QUdpSocket(this);
		if(!qsock->bind(addr, 0))
		{
			delete qsock;
			emit q->error(IceLocalTransport::ErrorBind);
			return 0;
		}

		return qsock;
	}

	void prepareSocket()
	{
		addr = sock->localAddress();
		port = sock->localPort();

		connect(sock, SIGNAL(readyRead()), SLOT(sock_readyRead()));
		connect(sock, SIGNAL(datagramsWritten(int)), SLOT(sock_datagramsWritten(int)));
	}

	// return true if we are retrying, false if we should error out
	bool handleRetry()
	{
		// don't allow retrying if activated or stopping)
		if(turnActivated || stopping)
			return false;

		++retryCount;
		if(retryCount)
		{
			printf("retrying...\n");

			delete sock;
			sock = 0;

			// to receive this error, it is a Relay, so change
			//   the mode
			stunType = IceLocalTransport::Relay;

			QUdpSocket *qsock = createSocket();
			if(!qsock)
			{
				// signal emitted in this case.  bail.
				//   (return true so caller takes no action)
				return true;
			}

			sock = new SafeUdpSocket(qsock, this);

			prepareSocket();

			refAddr = QHostAddress();
			refPort = -1;

			relAddr = QHostAddress();
			relPort = -1;

			do_turn();

			// tell the world that our local address probably
			//   changed, and that we lost our reflexive address
			emit q->addressesChanged();
			return true;
		}

		return false;
	}

	// return true if data packet, false if pool or nothing
	bool processIncomingStun(const QByteArray &buf, Datagram *dg)
	{
		QByteArray data;
		QHostAddress fromAddr;
		int fromPort;

		bool notStun;
		if(!pool->writeIncomingMessage(buf, &notStun) && turn)
		{
			data = turn->processIncomingDatagram(buf, notStun, &fromAddr, &fromPort);
			if(!data.isNull())
			{
				dg->addr = fromAddr;
				dg->port = fromPort;
				dg->buf = data;
				return true;
			}
			else
				printf("Warning: server responded with what doesn't seem to be a STUN or data packet, skipping.\n");
		}

		return false;
	}

private slots:
	void postStart()
	{
		if(stopping)
			return;

		if(extSock)
		{
			sock = new SafeUdpSocket(extSock, this);
		}
		else
		{
			QUdpSocket *qsock = createSocket();
			if(!qsock)
			{
				// signal emitted in this case.  bail
				return;
			}

			sock = new SafeUdpSocket(qsock, this);
		}

		prepareSocket();

		emit q->started();
	}

	void postStop()
	{
		reset();
		emit q->stopped();
	}

	void sock_readyRead()
	{
		ObjectSessionWatcher watch(&sess);

		QList<Datagram> dreads;
		QList<Datagram> rreads;

		while(sock->hasPendingDatagrams())
		{
			QHostAddress from;
			quint16 fromPort;

			Datagram dg;

			QByteArray buf = sock->readDatagram(&from, &fromPort);
			if(from == stunAddr && fromPort == stunPort)
			{
				bool haveData = processIncomingStun(buf, &dg);

				// processIncomingStun could cause signals to
				//   emit.  for example, stopped()
				if(!watch.isValid())
					return;

				if(haveData)
					rreads += dg;
			}
			else
			{
				dg.addr = from;
				dg.port = fromPort;
				dg.buf = buf;
				dreads += dg;
			}
		}

		if(dreads.count() > 0)
		{
			in += dreads;
			emit q->readyRead(Direct);
			if(!watch.isValid())
				return;
		}

		if(rreads.count() > 0)
		{
			inRelayed += rreads;
			emit q->readyRead(Relayed);
		}
	}

	void sock_datagramsWritten(int count)
	{
		QList<Written> dwrites;
		int twrites = 0;

		while(count > 0)
		{
			Q_ASSERT(!pendingWrites.isEmpty());
			WriteItem wi = pendingWrites.takeFirst();
			--count;

			if(wi.type == WriteItem::Direct)
			{
				int at = -1;
				for(int n = 0; n < dwrites.count(); ++n)
				{
					if(dwrites[n].addr == wi.addr && dwrites[n].port == wi.port)
					{
						at = n;
						break;
					}
				}

				if(at != -1)
				{
					++dwrites[at].count;
				}
				else
				{
					Written wr;
					wr.addr = wi.addr;
					wr.port = wi.port;
					wr.count = 1;
					dwrites += wr;
				}
			}
			else if(wi.type == WriteItem::Turn)
				++twrites;
		}

		if(dwrites.isEmpty() && twrites == 0)
			return;

		ObjectSessionWatcher watch(&sess);

		if(!dwrites.isEmpty())
		{
			foreach(const Written &wr, dwrites)
			{
				emit q->datagramsWritten(Direct, wr.count, wr.addr, wr.port);
				if(!watch.isValid())
					return;
			}
		}

		if(twrites > 0)
		{
			// note: this will invoke turn_packetsWritten()
			turn->outgoingDatagramsWritten(twrites);
		}
	}

	void pool_outgoingMessage(const QByteArray &packet, const QHostAddress &toAddress, int toPort)
	{
		// we aren't using IP-associated transactions
		Q_UNUSED(toAddress);
		Q_UNUSED(toPort);

		// warning: read StunTransactionPool docs before modifying
		//   this function

		WriteItem wi;
		wi.type = WriteItem::Pool;
		pendingWrites += wi;
		sock->writeDatagram(packet, stunAddr, stunPort);
	}

	void pool_needAuthParams()
	{
		// we can get this signal if the user did not provide
		//   creds to us.  however, since this class doesn't support
		//   prompting just continue on as if we had a blank
		//   user/pass
		pool->continueAfterParams();
	}

	void binding_success()
	{
		refAddr = stunBinding->reflexiveAddress();
		refPort = stunBinding->reflexivePort();

		delete stunBinding;
		stunBinding = 0;

		emit q->addressesChanged();
	}

	void binding_error(XMPP::StunBinding::Error e)
	{
		Q_UNUSED(e);

		delete stunBinding;
		stunBinding = 0;

		//if(stunType == IceLocalTransport::Basic || (stunType == IceLocalTransport::Auto && !turn))
		//	emit q->addressesChanged();
	}

	void turn_connected()
	{
		printf("turn_connected\n");
	}

	void turn_tlsHandshaken()
	{
		printf("turn_tlsHandshaken\n");
	}

	void turn_closed()
	{
		printf("turn_closed\n");

		delete turn;
		turn = 0;
		turnActivated = false;

		postStop();
	}

	void turn_activated()
	{
		StunAllocate *allocate = turn->stunAllocate();

		refAddr = allocate->reflexiveAddress();
		refPort = allocate->reflexivePort();
		printf("Server says we are %s;%d\n", qPrintable(refAddr.toString()), refPort);

		relAddr = allocate->relayedAddress();
		relPort = allocate->relayedPort();
		printf("Server relays via %s;%d\n", qPrintable(relAddr.toString()), relPort);

		turnActivated = true;

		emit q->addressesChanged();
	}

	void turn_packetsWritten(int count, const QHostAddress &addr, int port)
	{
		emit q->datagramsWritten(Relayed, count, addr, port);
	}

	void turn_error(XMPP::TurnClient::Error e)
	{
		printf("turn_error: %s\n", qPrintable(turn->errorString()));

		delete turn;
		turn = 0;
		bool wasActivated = turnActivated;
		turnActivated = false;

		if(e == TurnClient::ErrorMismatch)
		{
			if(!extSock && handleRetry())
				return;
		}

		// this means our relay died on us.  in the future we might
		//   consider reporting this
		if(wasActivated)
			return;

		//if(stunType == IceLocalTransport::Relay || (stunType == IceLocalTransport::Auto && !stunBinding))
		//	emit q->addressesChanged();
	}

	void turn_outgoingDatagram(const QByteArray &buf)
	{
		WriteItem wi;
		wi.type = WriteItem::Turn;
		pendingWrites += wi;
		sock->writeDatagram(buf, stunAddr, stunPort);
	}

	void turn_debugLine(const QString &line)
	{
		printf("turn_debugLine: %s\n", qPrintable(line));
	}
};

IceLocalTransport::IceLocalTransport(QObject *parent) :
	IceTransport(parent)
{
	d = new Private(this);
}

IceLocalTransport::~IceLocalTransport()
{
	delete d;
}

void IceLocalTransport::setClientSoftwareNameAndVersion(const QString &str)
{
	d->clientSoftware = str;
}

void IceLocalTransport::start(QUdpSocket *sock)
{
	d->extSock = sock;
	d->start();
}

void IceLocalTransport::start(const QHostAddress &addr)
{
	d->addr = addr;
	d->start();
}

void IceLocalTransport::stop()
{
	d->stop();
}

void IceLocalTransport::setStunService(const QHostAddress &addr, int port, StunServiceType type)
{
	d->stunAddr = addr;
	d->stunPort = port;
	d->stunType = type;
}

void IceLocalTransport::setStunUsername(const QString &user)
{
	d->stunUser = user;
}

void IceLocalTransport::setStunPassword(const QCA::SecureArray &pass)
{
	d->stunPass = pass;
}

void IceLocalTransport::stunStart()
{
	d->stunStart();
}

QHostAddress IceLocalTransport::localAddress() const
{
	return d->addr;
}

int IceLocalTransport::localPort() const
{
	return d->port;
}

QHostAddress IceLocalTransport::serverReflexiveAddress() const
{
	return d->refAddr;
}

int IceLocalTransport::serverReflexivePort() const
{
	return d->refPort;
}

QHostAddress IceLocalTransport::relayedAddress() const
{
	return d->relAddr;
}

int IceLocalTransport::relayedPort() const
{
	return d->relPort;
}

void IceLocalTransport::addChannelPeer(const QHostAddress &addr, int port)
{
	if(d->turn)
		d->turn->addChannelPeer(addr, port);
}

bool IceLocalTransport::hasPendingDatagrams(int path) const
{
	if(path == Direct)
		return !d->in.isEmpty();
	else if(path == Relayed)
		return !d->inRelayed.isEmpty();
	else
	{
		Q_ASSERT(0);
		return false;
	}
}

QByteArray IceLocalTransport::readDatagram(int path, QHostAddress *addr, int *port)
{
	QList<Private::Datagram> *in = 0;
	if(path == Direct)
		in = &d->in;
	else if(path == Relayed)
		in = &d->inRelayed;
	else
		Q_ASSERT(0);

	if(!in->isEmpty())
	{
		Private::Datagram datagram = in->takeFirst();
		*addr = datagram.addr;
		*port = datagram.port;
		return datagram.buf;
	}
	else
		return QByteArray();
}

void IceLocalTransport::writeDatagram(int path, const QByteArray &buf, const QHostAddress &addr, int port)
{
	if(path == Direct)
	{
		Private::WriteItem wi;
		wi.type = Private::WriteItem::Direct;
		wi.addr = addr;
		wi.port = port;
		d->pendingWrites += wi;
		d->sock->writeDatagram(buf, addr, port);
	}
	else if(path == Relayed)
	{
		if(d->turn && d->turnActivated)
			d->turn->write(buf, addr, port);
	}
	else
		Q_ASSERT(0);
}

}

#include "icelocaltransport.moc"
