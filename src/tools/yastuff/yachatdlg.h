/*
 * yachatdlg.h - custom chat dialog
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

#ifndef YACHATDLG_H
#define YACHATDLG_H

#include <QDateTime>

#include "chatdlg.h"
#include "ui_yachatdialog.h"
#include "xmpp_vcard.h"

class YaChatViewModel;

class YaChatDlg : public ChatDlg
{
	Q_OBJECT
public:
	YaChatDlg(const Jid&, PsiAccount*, TabManager*);
	~YaChatDlg();

	// reimplemented
	QSize minimumSizeHint() const;
	void setJid(const Jid &);

	virtual void receivedPendingMessage();
	void addPendingMessage();

	void addMoodChange(SpooledType spooled, const QString& mood, const QDateTime& timeStamp);

public slots:
	// reimplemented
	virtual void closed();
	virtual void activated();
	virtual void updateContact(const Jid &, bool);

	void restoreLastMessages();

private slots:
	void updateComposingMessage();
	void updateModelNotices();
	void updateContactName();
	void retrieveHistoryFinished();
	void retrieveHistoryFinishedHelper();

	void addContact();
	void authContact();
	void showContactProfile();

protected:
	// reimplemented
	bool eventFilter(QObject* obj, QEvent* e);
	void resizeEvent(QResizeEvent*);
	void showEvent(QShowEvent*);

	XMPP::VCard::Gender gender() const;

protected slots:
	// reimplemented
	virtual void doHistory();
	virtual void doSend();
	virtual void chatEditCreated();

	virtual void optionChanged(const QString& option);

private:
	// reimplemented
	void initUi();
	void initActions();
	void capsChanged();
	bool isEncryptionEnabled() const;
	void contactUpdated(UserListItem* u, int status, const QString& statusString);
	void updateAvatar();
	void optionsUpdate();
	void doClear();
	void aboutToShow();
	void setLooks();

	// reimplemented
	QString colorString(bool local, SpooledType spooled) const;
	void appendSysMsg(const QString &);
	void appendEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags);
	void appendNormalMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags);
	void appendMessageFields(const Message& m);
	void nicksChanged();
	ChatViewClass* chatView() const;
	ChatEdit* chatEdit() const;

	Ui::YaChatDlg ui_;
	YaProfile* selfProfile_;
	YaProfile* contactProfile_;
	YaChatViewModel* model_;
	XMPP::Status lastStatus_;
	bool showAuthButton_;
};

#endif
