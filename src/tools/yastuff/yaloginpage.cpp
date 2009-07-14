/*
 * yaloginpage.cpp
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

#include "yaloginpage.h"

#include <QPainter>

#include "psicontactlist.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psioptions.h"
#include "psilogger.h"
#include "desktoputil.h"
#include "yacommon.h"

YaLoginPage::YaLoginPage()
	: QWidget(0)
	, contactList_(0)
	, shouldBeVisible_(true)
	, disabled_(false)
{
	ui_.setupUi(this);
	connect(ui_.signIn, SIGNAL(clicked()), SLOT(signIn()));
	connect(ui_.signingIn, SIGNAL(clicked()), SLOT(stop()));
	connect(ui_.registerButton, SIGNAL(clicked()), SLOT(registerAccount()));

	connect(ui_.login, SIGNAL(returnPressed()), SLOT(updateFocus()));
	connect(ui_.password, SIGNAL(returnPressed()), SLOT(updateFocus()));

	connect(ui_.login, SIGNAL(textChanged(const QString&)), SLOT(textChanged()));
	connect(ui_.password, SIGNAL(textChanged(const QString&)), SLOT(textChanged()));
	textChanged();

	YaPushButton::initAllButtons(this);
}

YaLoginPage::~YaLoginPage()
{
}

void YaLoginPage::setContactList(PsiContactList* contactList)
{
	contactList_ = contactList;
	Q_ASSERT(contactList_);
	connect(contactList_, SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
	connect(contactList_, SIGNAL(loadedAccounts()), SLOT(updatedActivity()));
	if (contactList_->accountsLoaded())
		accountCountChanged();
}

bool YaLoginPage::shouldBeVisible()
{
#ifdef YAPSI_ACTIVEX_SERVER
	return false;
#endif
	return shouldBeVisible_;
}

void YaLoginPage::accountCountChanged()
{
	QList<PsiAccount*> accounts;

	Q_ASSERT(contactList_);
	foreach(PsiAccount* account, contactList_->accounts()) {
		if (knownAccounts_.contains(account))
			continue;

		connect(account, SIGNAL(updatedActivity()), this, SLOT(updatedActivity()));
		connect(account, SIGNAL(connectionError(const QString&)), this, SLOT(connectionError(const QString&)));
		accounts << account;
	}

	knownAccounts_ = accounts;
	updatedActivity();
}

void YaLoginPage::updatedActivity()
{
	Q_ASSERT(contactList_);
	if (!contactList_->accountsLoaded())
		return;

	bool shouldBeVisible = true;

	if (contactList_->enabledAccounts().count() > 1) {
		shouldBeVisible = false;
	}
	else {
		foreach(PsiAccount* account, contactList_->enabledAccounts()) {
			if (account->isAvailable())
				shouldBeVisible = false;
		}
	}

	if (!shouldBeVisible)
		disabled_ = true;
	if (disabled_)
		shouldBeVisible = false;

	setShouldBeVisible(shouldBeVisible);
}

void YaLoginPage::setShouldBeVisible(bool shouldBeVisible)
{
	if (shouldBeVisible != shouldBeVisible_) {
		shouldBeVisible_ = shouldBeVisible;
		emit updateVisibility();
	}

	if (shouldBeVisible_) {
		bool enableControls = true;
		PsiAccount* account = 0;
		if (login().isEmpty())
			account = getLoginAccount();
		else
			account = getAccount();
		if (account) {
			UserAccount acc = account->userAccount();
			QString jid = Ya::stripYaRuHostname(acc.jid);

			ui_.login->setText(jid);
			ui_.password->setText(acc.pass);

			if (account->isActive())
				enableControls = false;
		}

		if (enableControls)
			ui_.buttonStack->setCurrentWidget(ui_.signInPage);
		else
			ui_.buttonStack->setCurrentWidget(ui_.signingInPage);

		ui_.login->setEnabled(enableControls);
		ui_.password->setEnabled(enableControls);
		ui_.signIn->setEnabled(signInEnabled());
		ui_.busy->setActive(!enableControls);

		if (login().isEmpty()) {
			ui_.login->setFocus();
		}
		else {
			ui_.password->setFocus();
			ui_.password->selectAll();
		}
	}
}

void YaLoginPage::connectionError(const QString& error)
{
	ui_.errorLabel->setText(error);
}

void YaLoginPage::signIn()
{
	ui_.errorLabel->setText(QString());

	// on new account creation accountCountChanged() is triggered, so we back up our fields
	QString jid  = login();
	QString pass = password();

	Q_ASSERT(contactList_);
	PsiAccount* account = getAccount();
	if (!account) {
		account = contactList_->psi()->createAccount();
	}
	Q_ASSERT(account);

	foreach(PsiAccount* acc, contactList_->accounts()) {
		disconnect(acc, SIGNAL(connectionError(const QString&)), this, SLOT(connectionError(const QString&)));
	}
	connect(account, SIGNAL(connectionError(const QString&)), this, SLOT(connectionError(const QString&)));

	UserAccount acc = account->userAccount();
	acc.jid  = jid;
	acc.pass = pass;
	acc.opt_host = false;
	account->setUserAccount(acc);
	account->setEnabled(true);
	account->autoLogin();
}

void YaLoginPage::stop()
{
	PsiLogger::instance()->log(QString("%1 YaLoginPage()::stop()").arg(LOG_THIS));

	Q_ASSERT(contactList_);
	PsiAccount* account = getAccount();
	if (account)
		account->setStatus(XMPP::Status::Offline);
}

bool YaLoginPage::signInEnabled() const
{
	return ui_.login->isEnabled() &&
	       !ui_.login->text().isEmpty() &&
	       !ui_.password->text().isEmpty();
}

void YaLoginPage::textChanged()
{
	ui_.signIn->setEnabled(signInEnabled());
}

void YaLoginPage::updateFocus()
{
	if (ui_.login->hasFocus()) {
		ui_.password->setFocus();
		ui_.password->selectAll();
	}
	else if (ui_.password->hasFocus()) {
		ui_.signIn->setFocus();
		ui_.signIn->click();
	}
	else {
		ui_.login->setFocus();
		ui_.login->selectAll();
	}
}

QString YaLoginPage::login() const
{
	QString result = ui_.login->text().trimmed();
	if (!result.isEmpty() && !result.contains("@"))
		result += "@ya.ru";
	return result;
}

QString YaLoginPage::password() const
{
	return ui_.password->text();
}

PsiAccount* YaLoginPage::getLoginAccount() const
{
	Q_ASSERT(login().isEmpty());

	if (!contactList_->enabledAccounts().isEmpty())
		return contactList_->enabledAccounts().first();

	if (!contactList_->accounts().isEmpty())
		return contactList_->accounts().first();

	return 0;
}

PsiAccount* YaLoginPage::getAccount() const
{
	Q_ASSERT(contactList_);

	foreach(PsiAccount* account, contactList_->accounts()) {
		if (account->jid().userHost() == login())
			return account;
	}

	if ((contactList_->accounts().count() == 1) ||
	    (PsiOptions::instance()->getOption("options.ui.account.single").toBool() && !contactList_->accounts().isEmpty()))
		return contactList_->accounts().first();

	return 0;
}

void YaLoginPage::registerAccount()
{
	DesktopUtil::openYaUrl("http://passport.yandex.ru/passport?mode=register");
}

void YaLoginPage::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.fillRect(rect(), Qt::white);
}
