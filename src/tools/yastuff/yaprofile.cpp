/*
 * yaprofile.cpp
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

#include "yaprofile.h"

#include <QUrl>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

#include "psiaccount.h"
#include "vcardfactory.h"
#include "avatars.h"
#include "xmpp_vcard.h"
#include "xmpp_tasks.h"
#include "yacommon.h"
#include "urlobject.h"
#include "desktoputil.h"
#include "userlist.h"
#include "jidutil.h"
#include "psicontact.h"

using namespace XMPP;

/** \todo Make this configurable */
static const QString S_URL_TEMPLATE = "http://%1.ya.ru/index_profile.xml?yasoft=online";
static const QString S_PHOTOS_URL_TEMPLATE = "http://fotki.yandex.ru/users/%1/?yasoft=online";
static const QString S_SELF_NICK_TEMPLATE = "<span style='color: red;'>%2</span><span style='color: black;'>%3</span><!--%1-->";
static const QString S_NICK_TEMPLATE = "<a href='%1'><span style='color: red;'>%2</span><span style='color: black;'>%3</span>";

// YaProfile::YaProfile(QObject * parent)
// 	: QObject(parent), userName_(QString()), nick_(QString())
// {}

YaProfile::YaProfile(PsiAccount* account, const XMPP::Jid& jid)
	: QObject(account)
	, account_(account)
	, jid_(jid.userHost())
	, lastMessage_(QDateTime::currentDateTime().addDays(-1))
{
	init();
}

YaProfile::YaProfile(const YaProfile & profile)
	: QObject(profile.parent())
	, account_(profile.account_)
	, jid_(profile.jid_)
	, lastMessage_(profile.lastMessage_)
{
	init();
}

YaProfile::~YaProfile()
{}

void YaProfile::init()
{
	updateRosterNick();
	connect(account(), SIGNAL(updateContact(const Jid &, bool)), SLOT(updateContact(const Jid &, bool)));
	connect(account()->avatarFactory(), SIGNAL(avatarChanged(const Jid&)), SLOT(avatarChanged(const Jid&)));
	connect(VCardFactory::instance(), SIGNAL(vcardChanged(const Jid&)), SLOT(vcardChanged(const Jid&)));
}

PsiAccount* YaProfile::account() const
{
	return account_;
}

/**
 * Creates new YaProfile. If specified \param jid is null, then profile will act
 * as an account for self contact.
 */
YaProfile* YaProfile::create(PsiAccount* account, const XMPP::Jid& jid)
{
	return new YaProfile(account, jid);
}

QPixmap YaProfile::avatar() const
{
	QPixmap pix = account()->avatarFactory()->getAvatar(jid());
	// if (pix.isNull())
	// 	pix = Ya::noAvatarPixmap();
	return pix;
}

void YaProfile::setAvatar(QPixmap avatar)
{
	const VCard* profileVCard = VCardFactory::instance()->vcard(jid());
	VCard vcard;
	if (profileVCard)
		vcard = *profileVCard;

	vcard.setPhoto(avatar.toImage());
	const PsiAccount* a = account();
	const VCard v = vcard;
	VCardFactory::instance()->setVCard(a, v);
}

bool YaProfile::isSelf() const
{
	return jid_.isNull();
}

const Jid& YaProfile::jid() const
{
	if (!jid_.isNull())
		return jid_;

	return account()->jid();
}

// FIXME: Should probably use roster name, then vcard name, and only then JID,
// in this priority.
// Also should connect to PsiAccount::updateContact signal.
QString YaProfile::name() const
{
	return JIDUtil::nickOrJid(rosterNick_, jid().full());

	// if (!rosterNick_.isEmpty())
	// 	return rosterNick_;
	// 
	// const VCard* vcard = VCardFactory::instance()->vcard(jid());
	// if (vcard) {
	// 	if (!vcard->nickName().isEmpty())
	// 		return vcard->nickName();
	// 	if (!vcard->fullName().isEmpty())
	// 		return vcard->fullName();
	// }
	// return username();
}

const QString& YaProfile::username() const
{
	return jid().user();
}

XMPP::VCard::Gender YaProfile::gender() const
{
	PsiContact* contact = account()->findContact(jid());
	return contact ? contact->gender() : XMPP::VCard::UnknownGender;
}

// void YaProfile::setName(const QString & neu)
// {
// 	// this is needed because we plan to emit nameSet
// 	if (neu != name_) {
// 		emit(nameSet(name_, neu));
// 		name_ = neu;
// 		emit nameChanged();
// 	}
// }

bool YaProfile::isOk() const
{
	return !username().isEmpty();
}

void YaProfile::browse() const
{
	DesktopUtil::openYaUrl(S_URL_TEMPLATE.arg(username()));
}

void YaProfile::browsePhotos() const
{
	DesktopUtil::openYaUrl(S_PHOTOS_URL_TEMPLATE.arg(username()));
}

const QString YaProfile::asYaLink(int len) const
{
	QString templ = S_NICK_TEMPLATE;
	if (!Ya::isYaJid(jid()))
		templ = S_SELF_NICK_TEMPLATE;
	return templ.arg(
	           S_URL_TEMPLATE.arg(username()),
	           name().left(1),
	           Ya::truncate(name().right(name().length() - 1), len));
}

bool YaProfile::operator!= (const YaProfile & profile) const
{
	return !(profile.name() == name() && profile.username() == username());
}

void YaProfile::avatarChanged(const Jid& j)
{
	if (!j.compare(jid(), false))
		return;
	emit avatarChanged();
}

void YaProfile::vcardChanged(const Jid& j)
{
	if (!j.compare(jid(), false))
		return;
	emit nameChanged();
}

/*
	TODO Doesn't seem to work
*/
void YaProfile::updateContact(const Jid &j, bool fromPresence)
{
	Q_UNUSED(fromPresence);
	// if groupchat, only update if the resource matches
	if (account()->findGCContact(j) && !jid().compare(j))
		return;

	if (jid().compare(j, false)) {
		updateRosterNick();
	}
}

void YaProfile::updateRosterNick()
{
	QList<UserListItem*> ul = account()->findRelevant(jid());
	UserListItem* u = 0;
	if (!ul.isEmpty())
		u = ul.first();

	rosterNick_ = QString();
	if (u) {
		rosterNick_ = u->name();
	}

	emit nameChanged();
}

void YaProfile::openProfile()
{
	account()->actionInfo(jid());
}
