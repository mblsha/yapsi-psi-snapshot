/*
 * yaprivacymanager.h - an iq:privacy manager with simple API
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

#ifndef YAPRIVACYMANAGER_H
#define YAPRIVACYMANAGER_H

#include "psiprivacymanager.h"

#include <QHash>

#include "xmpp_jid.h"

class PsiAccount;
class PrivacyListItem;

class YaPrivacyManager : public PsiPrivacyManager
{
	Q_OBJECT

public:
	YaPrivacyManager(PsiAccount* account);
	~YaPrivacyManager();

	bool isAvailable() const;

	bool isContactBlocked(const XMPP::Jid& jid) const;
	void setContactBlocked(const XMPP::Jid& jid, bool blocked);

signals:
	void availabilityChanged();
	void listChanged(const QStringList& contacts);

	void simulateContactOffline(const XMPP::Jid& contact);

private slots:
	void listReceived(const PrivacyList& p);
	void listsReceived(const QString& defaultList, const QString& activeList, const QStringList& lists);
	void listsError();

	void accountStateChanged();

	void changeDefaultList_success();
	void changeDefaultList_error();
	void changeActiveList_success();
	void changeActiveList_error();

private:
	PsiAccount* account_;
	bool accountAvailable_;
	bool isAvailable_;
	QHash<QString, PrivacyList*> lists_;
	QHash<QString, bool> isBlocked_;

	void invalidateBlockedListCache();
	void setIsAvailable(bool available);

	void createBlockedList();
	PrivacyList* blockedList() const;
	PrivacyListItem blockItemFor(const XMPP::Jid& jid) const;

	QStringList blockedContacts() const;
	bool isAuthorized(const XMPP::Jid& jid) const;
};

#endif
