/*
 * yaprofile.h
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

#ifndef YAPROFILE_H
#define YAPROFILE_H

#include <QDateTime>
#include <QObject>

class PsiAccount;
class QPixmap;

namespace XMPP {
	class JT_VCard;
}

#include "xmpp_jid.h"
#include "xmpp_vcard.h"
using namespace XMPP;

class YaProfile : public QObject
{
	Q_OBJECT
public:
	YaProfile(PsiAccount* account, const XMPP::Jid& jid);
	YaProfile(const YaProfile &);
	~YaProfile();

	static YaProfile* create(PsiAccount* account, const XMPP::Jid& jid = XMPP::Jid());

signals:
	/**
	 * This is implemented as a <em>signal</em>, rather than making
	 *    <code>setNick()</code> a <em>slot</em> because in general
	 *    the <code>name</code> may be changed from anywhere,
	 *    e.g. from external call thru <code>xmpp://</code> protocol
	 */
	// void nameSet(const QString & old, const QString & neu);

	/**
	 * This signal is emitted when profile's avatar is changed.
	 */
	void avatarChanged();

	/**
	 * This signal is emitted when profile's name is changed.
	 */
	void nameChanged();

public slots:
	void browse() const;
	void browsePhotos() const;

public:
	QPixmap avatar() const;
	void setAvatar(QPixmap);
	// virtual void setName(const QString &);
	const XMPP::Jid& jid() const;
	QString name() const;
	const QString& username() const;
	PsiAccount* account() const;
	XMPP::VCard::Gender gender() const;

	void openProfile();

	bool isOk() const;
	const QString asYaLink(int len = 0) const;

public:
	bool operator!= (const YaProfile &) const;

private slots:
	void avatarChanged(const Jid&);
	void vcardChanged(const Jid&);
	void updateContact(const Jid &j, bool fromPresence);

private:
	void init();
	bool isSelf() const;
	void updateRosterNick();

	PsiAccount* account_;
	XMPP::Jid jid_;
	QString rosterNick_;
	QDateTime lastMessage_;
};

#endif
