/*
 * ndns.cpp - native DNS resolution
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//! \class NDns ndns.h
//! \brief Simple DNS resolution using native system calls
//!
//! This class is to be used when Qt's QDns is not good enough.  Because QDns
//! does not use threads, it cannot make a system call asyncronously.  Thus,
//! QDns tries to imitate the behavior of each platform's native behavior, and
//! generally falls short.
//!
//! NDns uses a thread to make the system call happen in the background.  This
//! gives your program native DNS behavior, at the cost of requiring threads
//! to build.
//!
//! \code
//! #include "ndns.h"
//!
//! ...
//!
//! NDns dns;
//! dns.resolve("psi.affinix.com");
//!
//! // The class will emit the resultsReady() signal when the resolution
//! // is finished. You may then retrieve the results:
//!
//! QHostAddress ip_address = dns.result();
//!
//! // or if you want to get the IP address as a string:
//!
//! QString ip_address = dns.resultString();
//! \endcode

#include "ndns.h"

#include <QCoreApplication>
#include <q3socketdevice.h>
#include <q3ptrlist.h>
#include <qeventloop.h>
//Added by qt3to4:
#include <QCustomEvent>
#include <QEvent>
#include <Q3CString>
#include <QPointer>

#ifdef Q_OS_UNIX
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#include "psilogger.h"

// CS_NAMESPACE_BEGIN

//! \if _hide_doc_
class NDnsWorker : public QThread
{
public:
	NDnsWorker(QObject *, const Q3CString &);
	~NDnsWorker();

	bool success;
	bool cancelled;
	QHostAddress addr;

protected:
	void run();

private:
	Q3CString host;
};
//! \endif

//----------------------------------------------------------------------------
// NDnsManager
//----------------------------------------------------------------------------
#ifndef HAVE_GETHOSTBYNAME_R
#ifndef Q_WS_WIN
static QMutex *workerMutex = 0;
static QMutex *workerCancelled = 0;
#endif
#endif
static NDnsManager *manager_instance = 0;
bool winsock_init = false;

class NDnsManager::Item
{
public:
	NDns *ndns;
	NDnsWorker *worker;
};

class NDnsManager::Private
{
public:
	Item *find(const NDns *n)
	{
		Q3PtrListIterator<Item> it(list);
		for(Item *i; (i = it.current()); ++it) {
			if(i->ndns == n)
				return i;
		}
		return 0;
	}

	Item *find(const NDnsWorker *w)
	{
		Q3PtrListIterator<Item> it(list);
		for(Item *i; (i = it.current()); ++it) {
			if(i->worker == w)
				return i;
		}
		return 0;
	}

	Q3PtrList<Item> list;
};

NDnsManager::NDnsManager()
: QObject(QCoreApplication::instance())
{
#ifndef HAVE_GETHOSTBYNAME_R
#ifndef Q_WS_WIN
	workerMutex = new QMutex;
	workerCancelled = new QMutex;
#endif
#endif

#ifdef Q_OS_WIN32
	if(!winsock_init) {
		winsock_init = true;
		Q3SocketDevice *sd = new Q3SocketDevice;
		delete sd;
	}
#endif

	d = new Private;
	d->list.setAutoDelete(true);

	connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(app_aboutToQuit()));
}

NDnsManager::~NDnsManager()
{
	delete d;

#ifndef HAVE_GETHOSTBYNAME_R
#ifndef Q_WS_WIN
	delete workerMutex;
	workerMutex = 0;
	delete workerCancelled;
	workerCancelled = 0;
#endif
#endif
}

void NDnsManager::resolve(NDns *self, const QString &name)
{
	Item *i = new Item;
	i->ndns = self;
	i->worker = new NDnsWorker(this, name.utf8());
	connect(i->worker, SIGNAL(finished()), SLOT(workerFinished()));
	d->list.append(i);

	i->worker->start();
}

void NDnsManager::stop(NDns *self)
{
	Item *i = d->find(self);
	if(!i)
		return;
	// disassociate
	i->ndns = 0;

#ifndef HAVE_GETHOSTBYNAME_R
#ifndef Q_WS_WIN
	// cancel
	workerCancelled->lock();
	i->worker->cancelled = true;
	workerCancelled->unlock();
#endif
#endif

	d->list.removeRef(i);
}

bool NDnsManager::isBusy(const NDns *self) const
{
	Item *i = d->find(self);
	return (i ? true: false);
}

void NDnsManager::workerFinished()
{
	NDnsWorker* worker = dynamic_cast<NDnsWorker*>(sender());
	Q_ASSERT(worker);
	if (!worker)
		return;
	worker->wait(); // ensure that the thread is terminated

	Item *i = d->find(worker);
	if(i) {
		QHostAddress addr = i->worker->addr;
		QPointer<NDns> ndns = i->ndns;
		d->list.removeRef(i);

		// nuke manager if no longer needed (code that follows MUST BE SAFE!)
		tryDestroy();

		// requestor still around?
		if(ndns) {
			ndns->finished(addr);
		}
	}

	worker->deleteLater();
}

void NDnsManager::tryDestroy()
{
// mblsha: NDnsManager is now singleton
#if 0
	if(d->list.isEmpty()) {
		manager_instance = 0;
		deleteLater();
	}
#endif
}

void NDnsManager::app_aboutToQuit()
{
// mblsha: NDnsManager is now singleton
#if 0
	while(man) {
		QCoreApplication::instance()->processEvents(QEventLoop::WaitForMoreEvents);
	}
#endif
}


//----------------------------------------------------------------------------
// NDns
//----------------------------------------------------------------------------

//! \fn void NDns::resultsReady()
//! This signal is emitted when the DNS resolution succeeds or fails.

//!
//! Constructs an NDns object with parent \a parent.
NDns::NDns(QObject *parent)
:QObject(parent)
{
}

//!
//! Destroys the object and frees allocated resources.
NDns::~NDns()
{
	stop();
	PsiLogger::instance()->log(QString("%1 NDns::~NDns()").arg(LOG_THIS));
}

//!
//! Resolves hostname \a host (eg. psi.affinix.com)
void NDns::resolve(const QString &host)
{
	PsiLogger::instance()->log(QString("%1 NDns::resolve(%2)").arg(LOG_THIS).arg(host));
	stop();
	if(!manager_instance)
		manager_instance = new NDnsManager;
	manager_instance->resolve(this, host);
}

//!
//! Cancels the lookup action.
//! \note This will not stop the underlying system call, which must finish before the next lookup will proceed.
void NDns::stop()
{
	PsiLogger::instance()->log(QString("%1 NDns::stop()").arg(LOG_THIS));
	if(manager_instance)
		manager_instance->stop(this);
}

//!
//! Returns the IP address as QHostAddress.  This will be a Null QHostAddress if the lookup failed.
//! \sa resultsReady()
QHostAddress NDns::result() const
{
	return addr;
}

//!
//! Returns the IP address as a string.  This will be an empty string if the lookup failed.
//! \sa resultsReady()
QString NDns::resultString() const
{
	if (addr.isNull()) 
		return QString();
	else
		return addr.toString();
}

//!
//! Returns TRUE if busy resolving a hostname.
bool NDns::isBusy() const
{
	if(!manager_instance)
		return false;
	return manager_instance->isBusy(this);
}

void NDns::finished(const QHostAddress &a)
{
	PsiLogger::instance()->log(QString("%1 NDns::finished(%2)").arg(LOG_THIS).arg(a.toString()));
	addr = a;
	resultsReady();
}

//----------------------------------------------------------------------------
// NDnsWorker
//----------------------------------------------------------------------------
NDnsWorker::NDnsWorker(QObject *_par, const Q3CString &_host)
: QThread(_par)
{
	success = cancelled = false;
	host = _host.copy(); // do we need this to avoid sharing across threads?
}

NDnsWorker::~NDnsWorker()
{
}

void NDnsWorker::run()
{
	hostent *h = 0;

#ifdef HAVE_GETHOSTBYNAME_R
	hostent buf;
	char char_buf[1024];
	int err;
	gethostbyname_r(host.data(), &buf, char_buf, sizeof(char_buf), &h, &err);
#else
#ifndef Q_WS_WIN
	// lock for gethostbyname
	QMutexLocker locker(workerMutex);

	// check for cancel
	workerCancelled->lock();
	bool cancel = cancelled;
	workerCancelled->unlock();

	if(!cancel)
#endif
		h = gethostbyname(host.data());
#endif

	// FIXME: not ipv6 clean, currently.
	if(!h || h->h_addrtype != AF_INET) {
		success = false;
		return;
	}

	in_addr a = *((struct in_addr *)h->h_addr_list[0]);
	addr.setAddress(ntohl(a.s_addr));
	success = true;
}

// CS_NAMESPACE_END
