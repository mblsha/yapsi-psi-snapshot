/*
 * psigrowlnotifier.cpp: Psi's interface to Growl
 * Copyright (C) 2005  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * You can also redistribute and/or modify this program under the
 * terms of the Psi License, specified in the accompanied COPYING
 * file, as published by the Psi Project; either dated January 1st,
 * 2005, or (at your option) any later version.
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

#include <QPixmap>
#include <QStringList>
#include <QCoreApplication>
#include "common.h"
#include "psiaccount.h"
#include "avatars.h"
#include "growlnotifier.h"
#include "psigrowlnotifier.h"
#include "psievent.h"
#include "userlist.h"

#ifdef YAPSI
#include "yacommon.h"
#include "psioptions.h"
#include "psicontact.h"
#include "yavisualutil.h"
#endif

/**
 * A class representing the notification context, which will be passed to
 * Growl, and then passed back when a notification is clicked.
 */
class NotificationContext
{
public:
	NotificationContext(PsiAccount* a, Jid j) : account_(a), jid_(j), deleteCount_(0) { }
	PsiAccount* account() { return account_; }
	Jid jid() { return jid_; }

private:
	PsiAccount* account_;
	Jid jid_;

public:
	int deleteCount_;
};


/**
 * (Private) constructor of the PsiGrowlNotifier.
 * Initializes notifications and registers with Growl through GrowlNotifier.
 */
PsiGrowlNotifier::PsiGrowlNotifier() : QObject(QCoreApplication::instance())
{
	// Initialize all notifications
	QStringList nots;
	nots << QObject::tr("Contact becomes Available");
	nots << QObject::tr("Contact becomes Unavailable");
	nots << QObject::tr("Contact changes Status");
	nots << QObject::tr("Incoming Message");
	nots << QObject::tr("Incoming Headline");
	nots << QObject::tr("Incoming File");
#ifdef YAPSI
	nots << QObject::tr("Mood Changed");
	nots << QObject::tr("Connection Successful");
	nots << QObject::tr("Connection Error");
#endif

	// Initialize default notifications
	QStringList defaults;
#ifndef YAPSI
	defaults << QObject::tr("Contact becomes Available");
#endif
	defaults << QObject::tr("Incoming Message");
	defaults << QObject::tr("Incoming Headline");
	defaults << QObject::tr("Incoming File");
#ifdef YAPSI
	defaults << QObject::tr("Mood Changed");
	defaults << QObject::tr("Connection Successful");
	defaults << QObject::tr("Connection Error");
#endif

	// Register with Growl
	QString appName = "Psi";
#ifdef YAPSI
	appName = QObject::tr("Ya.Online");
#endif
	gn_ = new GrowlNotifier(this, nots, defaults, appName);
}


/**
 * Requests the global PsiGrowlNotifier instance.
 * If PsiGrowlNotifier wasn't initialized yet, it is initialized.
 *
 * \see GrowlNotifier()
 * \return A pointer to the PsiGrowlNotifier instance
 */
PsiGrowlNotifier* PsiGrowlNotifier::instance() 
{
	if (!instance_) 
		instance_ = new PsiGrowlNotifier();
	
	return instance_;
}

#ifdef YAPSI
static QPixmap getAvatar(PsiAccount* account, const XMPP::Jid& jid)
{
	Q_ASSERT(account);
	QPixmap result = account->avatarFactory()->getAvatar(jid.bare());
	if (result.isNull()) {
		PsiContact* contact = account->findContact(jid);
		XMPP::VCard::Gender gender = contact ? contact->gender() : XMPP::VCard::UnknownGender;
		result = Ya::VisualUtil::noAvatarPixmapFileName(gender);
	}
	return result;
}
#endif

/**
 * Requests a popup to be sent to Growl.
 *
 * \param account The requesting account.
 * \param type The type of popup to be sent.
 * \param jid The originating jid
 * \param uli The originating userlist item. Can be NULL.
 * \param event The originating event. Can be NULL.
 */
void PsiGrowlNotifier::popup(PsiAccount* account, PsiPopup::PopupType type, const Jid& jid, const Resource& r, const UserListItem* uli, PsiEvent* event)
{
	QString name;
	QString title, desc, contact;
	QString statusTxt = status2txt(makeSTATUS(r.status()));
	QString statusMsg = r.status().status();
#ifdef YAPSI
	QPixmap icon = getAvatar(account, jid);
#else
	QPixmap icon = account->avatarFactory()->getAvatar(jid.bare());
#endif
	if (uli) {
		contact = uli->name();
	}
	else if (event->type() == PsiEvent::Auth) {
		contact = ((AuthEvent*) event)->nick();
	}
	else if (event->type() == PsiEvent::Message) {
		contact = ((MessageEvent*) event)->nick();
	}

	if (contact.isEmpty())
		contact = jid.bare();

	// Default value for the title
	title = contact;

	switch(type) {
		case PsiPopup::AlertOnline:
			name = QObject::tr("Contact becomes Available");
			title = QString("%1 (%2)").arg(contact).arg(statusTxt);
			desc = statusMsg;
			//icon = PsiIconset::instance()->statusPQString(jid, r.status());
			break;
		case PsiPopup::AlertOffline:
			name = QObject::tr("Contact becomes Unavailable");
			title = QString("%1 (%2)").arg(contact).arg(statusTxt);
			desc = statusMsg;
			//icon = PsiIconset::instance()->statusPQString(jid, r.status());
			break;
		case PsiPopup::AlertStatusChange:
			name = QObject::tr("Contact changes Status");
			title = QString("%1 (%2)").arg(contact).arg(statusTxt);
			desc = statusMsg;
			//icon = PsiIconset::instance()->statusPQString(jid, r.status());
			break;
		case PsiPopup::AlertMessage:
		case PsiPopup::AlertChat: {
			name = QObject::tr("Incoming Message");
			if (type == PsiPopup::AlertMessage) {
				title = QObject::tr("%1 says:").arg(contact);
			}
			const Message* jmessage = &((MessageEvent *)event)->message();
			desc = jmessage->body();
			if (type == PsiPopup::AlertMessage) {
				//icon = IconsetFactory::iconPQString("psi/message");
			}
			else {
				//icon = IconsetFactory::iconPQString("psi/start-chat");
			}
#ifdef YAPSI
			desc = Ya::messageNotifierText(desc);

			if (!PsiOptions::instance()->getOption("options.ya.popups.message.enable").toBool()) {
				return;
			}
#endif
			break;
		}
		case PsiPopup::AlertHeadline: {
			name = QObject::tr("Incoming Headline");
			const Message* jmessage = &((MessageEvent *)event)->message();
			if ( !jmessage->subject().isEmpty())
				title = jmessage->subject();
			desc = jmessage->body();
			//icon = IconsetFactory::iconPQString("psi/headline");
			break;
		}
		case PsiPopup::AlertFile:
			name = QObject::tr("Incoming File");
			desc = QObject::tr("[Incoming File]");
			//icon = IconsetFactory::iconPQString("psi/file");
			break;
		default:
			break;
	}

	// Notify Growl
	NotificationContext* context = new NotificationContext(account, jid);
	gn_->notify(name, title, desc, icon, false, this, SLOT(notificationClicked(void*)), SLOT(notificationTimedOut(void*)), context);
}

void PsiGrowlNotifier::cleanup()
{
	// try to keep the garbage bin no larger than 50 entries
	while (contexts_.count() > 50) {
		delete contexts_.takeFirst();
	}
}

void PsiGrowlNotifier::tryDeleteContext(NotificationContext* context)
{
	// instead of deleting the context right away, let's put it in a list
	//   to be garbage collected, and bump the delete counter
	if (context->deleteCount_ == 0) {
		contexts_ += context;
	}
	++(context->deleteCount_);

	// perform routine cleanup
	cleanup();
}

void PsiGrowlNotifier::notificationClicked(void* c)
{
	NotificationContext* context = (NotificationContext*) c;
	context->account()->actionDefault(context->jid());
	//delete context;
	tryDeleteContext(context);
}

void PsiGrowlNotifier::notificationTimedOut(void* c)
{
	NotificationContext* context = (NotificationContext*) c;
	//delete context;
	tryDeleteContext(context);
}

#ifdef YAPSI
void PsiGrowlNotifier::moodChanged(PsiAccount* account, const Jid& jid, const QString& name, const QString& mood)
{
	QString notifyName = QObject::tr("Mood Changed");
	QString title = tr("%1 changed mood to:").arg(name);
	QString desc = mood;
	QPixmap icon = getAvatar(account, jid);
	NotificationContext* context = new NotificationContext(account, jid);
	gn_->notify(notifyName, title, desc, icon, false, this, SLOT(notificationClicked(void*)), SLOT(notificationTimedOut(void*)), context);
}

void PsiGrowlNotifier::showSuccessfullyConnectedToaster(PsiAccount* account)
{
	if (!PsiOptions::instance()->getOption("options.ya.popups.connection.enable").toBool()) {
		return;
	}

	QString notifyName = QObject::tr("Connection Successful");
	QString title = account->jid().bare();
	QString desc = tr("Successfully connected to server.");
	QPixmap icon = getAvatar(account, account->jid());
	NotificationContext* context = new NotificationContext(account, account->jid());
	gn_->notify(notifyName, title, desc, icon, false, this, 0, 0, context);
}

void PsiGrowlNotifier::showConnectionErrorToaster(PsiAccount* account, const QString& error)
{
	if (!PsiOptions::instance()->getOption("options.ya.popups.connection.enable").toBool()) {
		return;
	}

	QString notifyName = QObject::tr("Connection Error");
	QString title = account->jid().bare();
	QString desc = tr("%1").arg(error);
	QPixmap icon = getAvatar(account, account->jid());
	NotificationContext* context = new NotificationContext(account, account->jid());
	gn_->notify(notifyName, title, desc, icon, false, this, 0, 0, context);
}
#endif

PsiGrowlNotifier* PsiGrowlNotifier::instance_ = 0;
