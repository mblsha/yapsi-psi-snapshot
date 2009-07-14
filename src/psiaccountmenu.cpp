#include "psiaccountmenu.h"

#include "iconaction.h"
#include "shortcutmanager.h"
#include "psioptions.h"
#include "psiaccount.h"
#include "iconset.h"
#include "psiiconset.h"
#include "statusmenu.h"
#include "serverinfomanager.h"
#include "xmpp_message.h"

class PsiAccountMenu::Private : public QObject
{
	Q_OBJECT

	PsiAccount* account;

public:
	Private(QMenu* menu, PsiAccount* _account)
		: QObject(0)
	{
		account = _account;

		StatusMenu* statusMenu = new StatusMenu(menu);
		statusMenu->setStatus(account->status().type());
		statusMenu->setTitle(tr("&Status"));
		connect(statusMenu, SIGNAL(statusChanged(XMPP::Status::Type)), SLOT(setStatus(XMPP::Status::Type)));

		IconAction* act_mood = new IconAction("", tr("Mood"), 0, this);
		act_mood->setEnabled(account->serverInfoManager()->hasPEP());
		connect(act_mood, SIGNAL(activated()), account, SLOT(actionSetMood()));
		IconAction* act_avatar = new IconAction("", tr("Avatar"), 0, this);
		act_avatar->setEnabled(account->serverInfoManager()->hasPEP());
		connect(act_avatar, SIGNAL(activated()), account, SLOT(actionSetAvatar()));

		IconAction* act_addContact = new IconAction("", "psi/addContact", tr("&Add a contact"), 0, this);
		connect(act_addContact, SIGNAL(activated()), SLOT(addContact()));
		IconAction* act_serviceDiscovery = new IconAction("", "psi/disco", tr("Service &Discovery"), 0, this);
		connect(act_serviceDiscovery, SIGNAL(activated()), SLOT(serviceDiscovery()));
		IconAction* act_sendMessage = new IconAction("", "psi/sendMessage", tr("New &blank message"), 0, this);
		connect(act_sendMessage, SIGNAL(activated()), SLOT(sendMessage()));
		IconAction* act_xmlConsole = new IconAction("", "psi/xml", tr("&XML Console"), 0, this);
		connect(act_xmlConsole, SIGNAL(activated()), SLOT(xmlConsole()));
		IconAction* act_modifyAccount = new IconAction("", "psi/account", tr("&Modify Account..."), 0, this);
		connect(act_modifyAccount, SIGNAL(activated()), SLOT(modifyAccount()));

		IconAction* act_onlineUsers = new IconAction("", "psi/disco", tr("Online Users"), 0, this);
		act_onlineUsers->setEnabled(account->loggedIn());
		connect(act_onlineUsers, SIGNAL(activated()), SLOT(onlineUsers()));
		IconAction* act_sendServerMessage = new IconAction("", "psi/sendMessage", tr("Send server message"), 0, this);
		act_sendServerMessage->setEnabled(account->loggedIn());
		connect(act_sendServerMessage, SIGNAL(activated()), SLOT(sendServerMessage()));
		IconAction* act_setMOTD = new IconAction("", tr("Set MOTD"), 0, this);
		act_setMOTD->setEnabled(account->loggedIn());
		connect(act_setMOTD, SIGNAL(activated()), SLOT(setMOTD()));
		IconAction* act_updateMOTD = new IconAction("", tr("Update MOTD"), 0, this);
		act_updateMOTD->setEnabled(account->loggedIn());
		connect(act_updateMOTD, SIGNAL(activated()), SLOT(updateMOTD()));
		IconAction* act_deleteMOTD = new IconAction("", "psi/remove", tr("Delete MOTD"), 0, this);
		act_deleteMOTD->setEnabled(account->loggedIn());
		connect(act_deleteMOTD, SIGNAL(activated()), SLOT(deleteMOTD()));

		menu->addMenu(statusMenu);
#ifndef YAPSI
		menu->addAction(act_mood);
		menu->addAction(act_avatar);
#endif
		menu->addSeparator();
		menu->addAction(act_addContact);
#if !defined(YAPSI) || defined(YAPSI_DEV)
		menu->addAction(act_serviceDiscovery);
		if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool())
			menu->addAction(act_sendMessage);
		menu->addSeparator();
		menu->addAction(act_xmlConsole);
		menu->addSeparator();
#endif
		menu->addAction(act_modifyAccount);

#ifndef YAPSI
		if (PsiOptions::instance()->getOption("options.ui.menu.account.admin").toBool()) {
			QMenu* adminMenu = menu->addMenu(tr("&Admin"));
			adminMenu->setEnabled(account->loggedIn());
			adminMenu->addAction(act_onlineUsers);
			adminMenu->addAction(act_sendServerMessage);
			adminMenu->addSeparator();
			adminMenu->addAction(act_setMOTD);
			adminMenu->addAction(act_updateMOTD);
			adminMenu->addAction(act_deleteMOTD);
		}
#endif
	}

private:
	void sendAnnounceMessage(QString destination)
	{
		account->actionSendMessage(account->jid().withResource('/' + destination));
	}

private slots:
	void setStatus(XMPP::Status::Type type)
	{
		account->changeStatus(static_cast<int>(type));
	}

	void addContact()
	{
		account->openAddUserDlg();
	}

	void serviceDiscovery()
	{
		XMPP::Jid j = account->jid().domain();
		account->actionDisco(j, "");
	}

	void sendMessage()
	{
		account->actionSendMessage("");
	}

	void xmlConsole()
	{
		account->showXmlConsole();
	}

	void modifyAccount()
	{
		account->modify();
	}

	void onlineUsers()
	{
		// FIXME: will it still work on XMPP servers?
		account->actionDisco(account->jid().withResource('/' + "admin"), "");
	}

	void sendServerMessage()
	{
		sendAnnounceMessage("announce/online");
	}

	void setMOTD()
	{
		sendAnnounceMessage("announce/motd");
	}

	void updateMOTD()
	{
		sendAnnounceMessage("announce/motd/update");
	}

	void deleteMOTD()
	{
		Message m;
		m.setTo(account->jid().withResource('/' + "announce/motd/delete"));
		account->dj_sendMessage(m, false);
	}
};

/**
 * TODO
 */
PsiAccountMenu::PsiAccountMenu(PsiAccount* account, ContactListModel* model)
	: ContactListItemMenu(0, model)
{
	d = new Private(this, account);
}

/**
 * TODO
 */
PsiAccountMenu::~PsiAccountMenu()
{
	delete d;
}

#include "psiaccountmenu.moc"
