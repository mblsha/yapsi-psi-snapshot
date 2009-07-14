/*
 * eventdb.h - asynchronous I/O event database
 * Copyright (C) 2001, 2002  Justin Karneges
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

#ifndef EVENTDB_H
#define EVENTDB_H

#include <qobject.h>
#include <qtimer.h>
#include <qfile.h>
#include <q3ptrlist.h>

#include "xmpp_jid.h"

class PsiEvent;
class PsiAccount;

class EDBItem
{
public:
	EDBItem(PsiEvent *, const QString &id, const QString &nextId, const QString &prevId);
	~EDBItem();

	PsiEvent *event() const;
	const QString & id() const;
	const QString & nextId() const;
	const QString & prevId() const;

private:
	QString v_id, v_prevId, v_nextId;
	PsiEvent *e;
};

class EDBResult : public Q3PtrList<EDBItem>
{
public:
	EDBResult() {}
};

class EDB;
class EDBHandle : public QObject
{
	Q_OBJECT
public:
	enum { Read, Write, Erase };
	EDBHandle(EDB *);
	~EDBHandle();

	// operations
	void getLatest(const PsiAccount* account, const XMPP::Jid &, int len);
	void getOldest(const PsiAccount* account, const XMPP::Jid &, int len);
	void get(const PsiAccount* account, const XMPP::Jid &jid, const QString &id, int direction, int len);
	void find(const PsiAccount* account, const QString &, const XMPP::Jid &, const QString &id, int direction);
	void append(const PsiAccount* account, const XMPP::Jid &, PsiEvent *);
	void erase(const PsiAccount* account, const XMPP::Jid &);

	bool busy() const;
	const EDBResult *result() const;
	bool writeSuccess() const;
	int lastRequestType() const;

signals:
	void finished();

private:
	class Private;
	Private *d;

	friend class EDB;
	void edb_resultReady(EDBResult *);
	void edb_writeFinished(bool);
	int listeningFor() const;
};

class EDB : public QObject
{
	Q_OBJECT
public:
	enum { Forward, Backward };
	EDB();
	virtual ~EDB()=0;

	virtual bool hasPendingRequests() const=0;
	virtual void closeHandles()=0;

protected:
	int genUniqueId() const;
	virtual int getLatest(const PsiAccount* account, const XMPP::Jid &, int len)=0;
	virtual int getOldest(const PsiAccount* account, const XMPP::Jid &, int len)=0;
	virtual int get(const PsiAccount* account, const XMPP::Jid &jid, const QString &id, int direction, int len)=0;
	virtual int append(const PsiAccount* account, const XMPP::Jid &, PsiEvent *)=0;
	virtual int find(const PsiAccount* account, const QString &, const XMPP::Jid &, const QString &id, int direction)=0;
	virtual int erase(const PsiAccount* account, const XMPP::Jid &)=0;
	void resultReady(int, EDBResult *);
	void writeFinished(int, bool);

private:
	class Private;
	Private *d;

	friend class EDBHandle;
	void reg(EDBHandle *);
	void unreg(EDBHandle *);

	int op_getLatest(const PsiAccount* account, const XMPP::Jid &, int len);
	int op_getOldest(const PsiAccount* account, const XMPP::Jid &, int len);
	int op_get(const PsiAccount* account, const XMPP::Jid &, const QString &id, int direction, int len);
	int op_find(const PsiAccount* account, const QString &, const XMPP::Jid &, const QString &id, int direction);
	int op_append(const PsiAccount* account, const XMPP::Jid &, PsiEvent *);
	int op_erase(const PsiAccount* account, const XMPP::Jid &);
};

class EDBFlatFile : public EDB
{
	Q_OBJECT
public:
	EDBFlatFile();
	~EDBFlatFile();

	int getLatest(const PsiAccount* account, const XMPP::Jid &, int len);
	int getOldest(const PsiAccount* account, const XMPP::Jid &, int len);
	int get(const PsiAccount* account, const XMPP::Jid &jid, const QString &id, int direction, int len);
	int find(const PsiAccount* account, const QString &, const XMPP::Jid &, const QString &id, int direction);
	int append(const PsiAccount* account, const XMPP::Jid &, PsiEvent *);
	int erase(const PsiAccount* account, const XMPP::Jid &);

	bool hasPendingRequests() const;
	void closeHandles();

	class File;

private slots:
	void performRequests();
	void file_timeout();

private:
	class Private;
	Private *d;

	File *findFile(const PsiAccount* account, const XMPP::Jid &) const;
	File *ensureFile(const PsiAccount* account, const XMPP::Jid &);
	bool deleteFile(const PsiAccount* account, const XMPP::Jid &);

	enum Type {
		Type_MessageSingle = 0,
		Type_MessageChat = 1,
		Type_MessageError = 4,
		Type_MessageHeadline = 5,

		// legacy
		Type_SystemMessage = 2,

		Type_AuthSubscribe = 3,
		Type_AuthSubscribed = 6,
		Type_AuthUnsubscribe = 7,
		Type_AuthUnsubscribed = 8

#ifdef YAPSI
		, Type_Mood = 42
#endif
	};
};

class EDBFlatFile::File : public QObject
{
	Q_OBJECT
public:
	File(const PsiAccount* account, const XMPP::Jid &_j);
	~File();

	int total() const;
	void touch();
	PsiEvent *get(int);
	bool append(PsiEvent *);

#ifdef YAPSI
	enum FileType {
		FileType_Normal = 0,
		FileType_HumanReadable
	};
	FileType type;
#endif

	static bool historyExists(const PsiAccount* account, const XMPP::Jid &);
#ifdef YAPSI
	static QString accountYaHistoryDir(const PsiAccount* account);
#endif
	static QString accountHistoryDir(const PsiAccount* account);
	static bool removeAccountHistoryDir(const PsiAccount* account, bool removeNonYaHistoryOnly = false);
#ifdef YAPSI
	static bool removeOldHistoryDir();
	static QString getFileName(const PsiAccount* account, const XMPP::Jid &, EDBFlatFile::File::FileType* type);
	static QString jidToYaFileName(const PsiAccount* account, const XMPP::Jid &);
#endif
	static QString jidToFileName(const PsiAccount* account, const XMPP::Jid &);
	static QString legacyJidToFileName(const XMPP::Jid &);

signals:
	void timeout();

private slots:
	void timer_timeout();

public:
	const PsiAccount* account;
	XMPP::Jid j;
	QString fname;
	QFile f;
	bool valid;
	QTimer *t;

	class Private;
	Private *d;

private:
	PsiEvent *lineToEvent(const QString &);
	QString eventToLine(PsiEvent *);
	void ensureIndex();
};

#endif
