/*
 * chatdlg.cpp - dialog for handling chats
 * Copyright (C) 2001-2007  Justin Karneges, Michail Pishchagin
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

#include "chatdlg.h"

#include <QLabel>
#include <QCursor>
#include <QLineEdit>
#include <QToolButton>
#include <QLayout>
#include <QSplitter>
#include <QToolBar>
#include <QTimer>
#include <QDateTime>
#include <QPixmap>
#include <QColor>
#include <Qt>
#include <QCloseEvent>
#include <QList>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QDropEvent>
#include <QList>
#include <QMessageBox>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QDragEnterEvent>
#include <QTextDocument> // for Qt::escape()
#include <QScrollBar>

#include "psiaccount.h"
#include "userlist.h"
#include "stretchwidget.h"
#include "psiiconset.h"
#include "iconwidget.h"
#include "textutil.h"
#include "xmpp_message.h"
#include "xmpp_htmlelement.h"
#include "fancylabel.h"
#include "msgmle.h"
#include "iconselect.h"
#include "psicon.h"
#include "iconlabel.h"
#include "capsmanager.h"
#include "iconaction.h"
#include "avatars.h"
#include "jidutil.h"
#include "tabdlg.h"
#include "psioptions.h"
#include "psitooltip.h"
#include "shortcutmanager.h"
#include "psicontactlist.h"
#include "accountlabel.h"
#include "psicontact.h"
#include "mcmdmanager.h"
#include "psilogger.h"
#ifdef HAVE_PGPUTIL
#include "pgputil.h"
#endif
#include "psirichtext.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#ifdef YAPSI
#include "yachatdlg.h"
#include "yacommon.h"
#include "yachatviewmodel.h"
#else
#include "psichatdlg.h"
#endif

ChatDlg* ChatDlg::create(const Jid& jid, PsiAccount* account, TabManager* tabManager)
{
#ifdef YAPSI
	YaChatDlg* chat = new YaChatDlg(jid, account, tabManager);
#else
	ChatDlg* chat = new PsiChatDlg(jid, account, tabManager);
#endif
	chat->init();
	chat->setJid(jid);
#ifdef YAPSI
	chat->restoreLastMessages();
#endif
	return chat;
}

ChatDlg::ChatDlg(const Jid& jid, PsiAccount* pa, TabManager* tabManager)
	: TabbableWidget(jid, pa, tabManager)
	, highlightersInstalled_(false)
{
	if (PsiOptions::instance()->getOption("options.ui.mac.use-brushed-metal-windows").toBool()) {
		setAttribute(Qt::WA_MacMetalStyle);
	}

	pending_ = 0;
	keepOpen_ = false;
	warnSend_ = false;
	selfDestruct_ = 0;
	transid_ = -1;
	key_ = "";
	lastWasEncrypted_ = false;

	status_ = -1;

	// Message events
	contactChatState_ = XMPP::StateNone;
	lastChatState_ = XMPP::StateNone;
	sendComposingEvents_ = false;
	isComposing_ = false;
	composingTimer_ = 0;
}

void ChatDlg::init()
{
	initUi();
	initActions();
	setShortcuts();

	connect(chatView(), SIGNAL(selectionChanged()), SLOT(logSelectionChanged()));

	connect(account(), SIGNAL(updatedActivity()), SLOT(updateSendAction()));
	connect(account(), SIGNAL(enabledChanged()), SLOT(updateSendAction()));
	updateSendAction();

	// SyntaxHighlighters modify the QTextEdit in a QTimer::singleShot(0, ...) call
	// so we need to install our hooks after it fired for the first time
	QTimer::singleShot(10, this, SLOT(initComposing()));
	connect(this, SIGNAL(composing(bool)), SLOT(updateIsComposing(bool)));

#ifdef FILETRANSFER
	setAcceptDrops(true);
#else
	setAcceptDrops(false);
#endif
	updateContact(jid(), true);

	X11WM_CLASS("chat");
	setLooks();

	updatePGP();

	connect(account(), SIGNAL(pgpKeyChanged()), SLOT(updatePGP()));
	connect(account(), SIGNAL(encryptedMessageSent(int, bool, int, const QString &)), SLOT(encryptedMessageSent(int, bool, int, const QString &)));
	account()->dialogRegister(this, jid());

	chatView()->setFocusPolicy(Qt::NoFocus);
	chatEdit()->setFocus();

#ifndef YAPSI
	// TODO: port to restoreSavedSize() (and adapt it from restoreSavedGeometry())
	QSize size = PsiOptions::instance()->getOption("options.ui.chat.size").toSize();
	if (!size.isEmpty()) {
		resize(size);
	} else {
		resize(defaultSize());
	}
#endif
}

ChatDlg::~ChatDlg()
{
	account()->dialogUnregister(this);
}

void ChatDlg::initComposing()
{
	highlightersInstalled_ = true;
	chatEditCreated();
}

void ChatDlg::initActions()
{
	act_send_ = new QAction(tr("Send"), this);
	addAction(act_send_);
	connect(act_send_, SIGNAL(triggered()), SLOT(doSend()));

	act_close_ = new QAction(this);
	addAction(act_close_);
	connect(act_close_, SIGNAL(triggered()), SLOT(close()));

	act_scrollup_ = new QAction(this);
	addAction(act_scrollup_);
	connect(act_scrollup_, SIGNAL(triggered()), SLOT(scrollUp()));

	act_scrolldown_ = new QAction(this);
	addAction(act_scrolldown_);
	connect(act_scrolldown_, SIGNAL(triggered()), SLOT(scrollDown()));
}

void ChatDlg::ensureTabbedCorrectly() {
	TabbableWidget::ensureTabbedCorrectly();
	setShortcuts();
}


void ChatDlg::setShortcuts()
{
	act_send_->setShortcuts(ShortcutManager::instance()->shortcuts("chat.send"));
	act_scrollup_->setShortcuts(ShortcutManager::instance()->shortcuts("common.scroll-up"));
	act_scrolldown_->setShortcuts(ShortcutManager::instance()->shortcuts("common.scroll-down"));

	if(!isTabbed()) {
		act_close_->setShortcuts(ShortcutManager::instance()->shortcuts("common.close"));
	} else {
		act_close_->QAction::setShortcuts (QList<QKeySequence>());
	}
}

void ChatDlg::scrollUp()
{
	chatView()->verticalScrollBar()->setValue(chatView()->verticalScrollBar()->value() - chatView()->verticalScrollBar()->pageStep() / 2);
}

void ChatDlg::scrollDown()
{
	chatView()->verticalScrollBar()->setValue(chatView()->verticalScrollBar()->value() + chatView()->verticalScrollBar()->pageStep() / 2);
}

void ChatDlg::keyPressEvent(QKeyEvent *e)
{
	QKeySequence key = e->key() + (e->modifiers() & ~Qt::KeypadModifier);
	if (key.toString(QKeySequence::PortableText).contains("Enter") ||
	    key.toString(QKeySequence::PortableText).contains("Return"))
	{
		chatEdit()->append("\n");
	}
	else {
		e->ignore();
	}
}

void ChatDlg::resizeEvent(QResizeEvent *e)
{
#ifndef YAPSI
	if (PsiOptions::instance()->getOption("options.ui.remember-window-sizes").toBool()) {
		PsiOptions::instance()->setOption("options.ui.chat.size", e->size());
	}
#endif
}

void ChatDlg::closeEvent(QCloseEvent *e)
{
	if (readyToHide()) {
		e->accept();
	}
	else {
		e->ignore();
	}
}

/**
 * Runs all the gumph necessary before hiding a chat.
 * (checking new messages, setting the autodelete, cancelling composing etc)
 * \return ChatDlg is ready to be hidden.
 */
bool ChatDlg::readyToHide()
{
	// really lame way of checking if we are encrypting
	if (!chatEdit()->isEnabled()) {
		return false;
	}

#ifndef YAPSI
	if (keepOpen_) {
		QMessageBox mb(QMessageBox::Information,
			tr("Warning"),
			tr("A new chat message was just received.\nDo you still want to close the window?"),
			QMessageBox::Cancel,
			this);
		mb.addButton(tr("Close"), QMessageBox::AcceptRole);
		if (mb.exec() == QMessageBox::Cancel) {
			return false;
		}
	}
#endif
	keepOpen_ = false; // tabdlg calls readyToHide twice on tabdlg close, only display message once.

#ifndef YAPSI
	// destroy the dialog if delChats is dcClose
	if (PsiOptions::instance()->getOption("options.ui.chat.delete-contents-after").toString() == "instant") {
		setAttribute(Qt::WA_DeleteOnClose);
	}
	else {
		if (PsiOptions::instance()->getOption("options.ui.chat.delete-contents-after").toString() == "hour") {
			setSelfDestruct(60);
		}
		else if (PsiOptions::instance()->getOption("options.ui.chat.delete-contents-after").toString() == "day") {
			setSelfDestruct(60 * 24);
		}
	}
#endif

	// Reset 'contact is composing' & cancel own composing event
	resetComposing();
	setChatState(StateGone);
	if (contactChatState_ == XMPP::StateComposing || contactChatState_ == XMPP::StateInactive) {
		setContactChatState(StatePaused);
	}

#ifndef YAPSI
	emit messagesRead(jid());

	if (pending_ > 0) {
		pending_ = 0;
		invalidateTab();
	}
	doFlash(false);

	chatEdit()->setFocus();
#endif
	return true;
}

void ChatDlg::capsChanged(const Jid& j)
{
	if (jid().compare(j, false)) {
		capsChanged();
	}
}

void ChatDlg::capsChanged()
{
}

void ChatDlg::hideEvent(QHideEvent* e)
{
	if (isMinimized()) {
		resetComposing();
		setChatState(StateInactive);
	}
	TabbableWidget::hideEvent(e);

	// hideEvent is called both when current chat tab becomes inactive
	// and when TabDlg closes it but decides not to call close() on it
	slotScroll();
}

void ChatDlg::showEvent(QShowEvent *)
{
	setSelfDestruct(0);
}

void ChatDlg::logSelectionChanged()
{
#ifdef Q_WS_MAC
	// A hack to only give the message log focus when text is selected
	if (chatView()->textCursor().hasSelection()) {
		chatView()->setFocus();
	}
	else {
		chatEdit()->setFocus();
	}
#endif
}

void ChatDlg::deactivated()
{
	TabbableWidget::deactivated();
}

void ChatDlg::activated()
{
	TabbableWidget::activated();

	emit messagesRead(jid());

	if (pending_ > 0) {
		pending_ = 0;
		invalidateTab();
	}
	doFlash(false);

	chatEdit()->setFocus();
}

void ChatDlg::dropEvent(QDropEvent* event)
{
	QStringList files;
	if (account()->loggedIn() && event->mimeData()->hasUrls()) {
		foreach(QUrl url, event->mimeData()->urls()) {
			if (!url.toLocalFile().isEmpty()) {
				files << url.toLocalFile();
			}
		}
	}

	if (!files.isEmpty()) {
		account()->actionSendFiles(jid(), files);
	}
}

void ChatDlg::dragEnterEvent(QDragEnterEvent* event)
{
	Q_ASSERT(event);
	//bool accept = false;
	if (account()->loggedIn() && event->mimeData()->hasUrls()) {
		foreach(QUrl url, event->mimeData()->urls()) {
			if (!url.toLocalFile().isEmpty()) {
				event->accept();
				break;
			}
		}
	}
}

void ChatDlg::setJid(const Jid &j)
{
	if (!j.compare(jid())) {
		account()->dialogUnregister(this);
		TabbableWidget::setJid(j);
		account()->dialogRegister(this, jid());
		updateContact(jid(), false);
	}
}

const QString& ChatDlg::getDisplayName()
{
	return dispNick_;
}

QSize ChatDlg::defaultSize()
{
	return QSize(320, 280);
}

struct UserStatus {
	UserStatus()
		: userListItem(0)
		, statusType(XMPP::Status::Offline)
	{}

	UserListItem* userListItem;
	XMPP::Status::Type statusType;
	QString status;
	QString publicKeyID;
};

UserStatus userStatusFor(const Jid& jid, QList<UserListItem*> ul, bool forceEmptyResource)
{
	if (ul.isEmpty())
		return UserStatus();

	UserStatus u;

	u.userListItem = ul.first();
	if (jid.resource().isEmpty() || forceEmptyResource) {
		// use priority
		if (u.userListItem->isAvailable()) {
			const UserResource &r = *u.userListItem->userResourceList().priority();
			u.statusType = r.status().type();
			u.status = r.status().status();
			u.publicKeyID = r.publicKeyID();
		}
	}
	else {
		// use specific
		UserResourceList::ConstIterator rit = u.userListItem->userResourceList().find(jid.resource());
		if (rit != u.userListItem->userResourceList().end()) {
			u.statusType = (*rit).status().type();
			u.status = (*rit).status().status();
			u.publicKeyID = (*rit).publicKeyID();
		}
	}

	if (u.statusType == XMPP::Status::Offline)
		u.status = u.userListItem->lastUnavailableStatus().status();

	return u;
}

void ChatDlg::updateContact(const Jid &j, bool fromPresence)
{
	// if groupchat, only update if the resource matches
	if (account()->findGCContact(j) && !jid().compare(j)) {
		return;
	}

	if (jid().compare(j, false)) {
		QList<UserListItem*> ul = account()->findRelevant(j);
		UserStatus userStatus = userStatusFor(jid(), ul, false);
#ifdef YAPSI
		if (userStatus.statusType == XMPP::Status::Offline)
			userStatus = userStatusFor(jid(), ul, true);
#endif
		if (userStatus.statusType == XMPP::Status::Offline)
			contactChatState_ = XMPP::StateNone;

		bool statusChanged = false;
		if (status_ != userStatus.statusType || statusString_ != userStatus.status) {
			statusChanged = true;
			status_ = userStatus.statusType;
			statusString_ = userStatus.status;
		}

		contactUpdated(userStatus.userListItem, userStatus.statusType, userStatus.status);

		{
			QString jidText = userStatus.userListItem ?
			                  userStatus.userListItem->jid().full() :
			                  jid().full();
			dispNick_ = JIDUtil::nickOrJid(userStatus.userListItem ? userStatus.userListItem->name() : dispNick_, jidText);
#ifdef YAPSI
			dispNick_ = Ya::contactName(dispNick_, jidText);
#endif
			nicksChanged();
			invalidateTab();

			key_ = userStatus.publicKeyID;
			updatePGP();

			if (fromPresence && statusChanged) {
				QString msg = tr("%1 is %2").arg(Qt::escape(dispNick_)).arg(status2txt(status_));
				if (!statusString_.isEmpty()) {
					QString ss = TextUtil::linkify(TextUtil::plain2rich(statusString_));
					ss = TextUtil::emoticonify(ss);
					ss = TextUtil::legacyFormat(ss);
					msg += QString(" [%1]").arg(ss);
				}
#ifndef YAPSI
				appendSysMsg(msg);
#endif
			}
		}

		// Update capabilities
		capsChanged(jid());

		// Reset 'is composing' event if the status changed
		if (statusChanged && contactChatState_ != XMPP::StateNone) {
			if (contactChatState_ == XMPP::StateComposing || contactChatState_ == XMPP::StateInactive) {
				setContactChatState(XMPP::StatePaused);
			}
		}
	}
}

void ChatDlg::contactUpdated(UserListItem* u, int status, const QString& statusString)
{
	Q_UNUSED(u);
	Q_UNUSED(status);
	Q_UNUSED(statusString);

	if (!sendComposingEvents_) {
		sendComposingEvents_ = canChatState();
	}
}

void ChatDlg::doVoice()
{
	aVoice(jid());
}

void ChatDlg::updateAvatar(const Jid& j)
{
	if (j.compare(jid(), false))
		updateAvatar();
}

void ChatDlg::setLooks()
{
	// update the font
	QFont f;
	f.fromString(PsiOptions::instance()->getOption("options.ui.look.font.chat").toString());
	chatView()->setFont(f);
	chatEdit()->setFont(f);

	// update contact info
	status_ = -2; // sick way of making it redraw the status
	updateContact(jid(), false);

	// update the widget icon
#ifndef Q_WS_MAC
	setWindowIcon(IconsetFactory::icon("psi/start-chat").icon());
#endif

	/*QBrush brush;
	brush.setPixmap( QPixmap( LEGOPTS.chatBgImage ) );
	chatView()->setPaper(brush);
	chatView()->setStaticBackground(true);*/

	setWindowOpacity(double(qMax(MINIMUM_OPACITY, PsiOptions::instance()->getOption("options.ui.chat.opacity").toInt())) / 100);
}

void ChatDlg::optionsUpdate()
{
	setLooks();
	setShortcuts();

	if (isHidden()) {
		if (PsiOptions::instance()->getOption("options.ui.chat.delete-contents-after").toString() == "instant") {
			LOG_TRACE;
			deleteLater();
			return;
		}
		else {
			if (PsiOptions::instance()->getOption("options.ui.chat.delete-contents-after").toString() == "hour") {
				setSelfDestruct(60);
			}
			else if (PsiOptions::instance()->getOption("options.ui.chat.delete-contents-after").toString() == "day") {
				setSelfDestruct(60 * 24);
			}
			else {
				setSelfDestruct(0);
			}
		}
	}
}

void ChatDlg::updatePGP()
{
}

void ChatDlg::doInfo()
{
	aInfo(jid());
}

void ChatDlg::doHistory()
{
	aHistory(jid());
}

void ChatDlg::doFile()
{
	aFile(jid());
}

void ChatDlg::doClear()
{
#ifndef YAPSI
	chatView()->clear();
#endif
}

void ChatDlg::setKeepOpenFalse()
{
	keepOpen_ = false;
}

void ChatDlg::setWarnSendFalse()
{
	warnSend_ = false;
}

void ChatDlg::setSelfDestruct(int minutes)
{
#ifdef YAPSI
	return;
#endif

	if (minutes <= 0) {
		if (selfDestruct_) {
			delete selfDestruct_;
			selfDestruct_ = 0;
		}
		return;
	}

	if (!selfDestruct_) {
		LOG_TRACE;
		selfDestruct_ = new QTimer(this);
		connect(selfDestruct_, SIGNAL(timeout()), SLOT(deleteLater()));
	}

	selfDestruct_->start(minutes * 60000);
}

QString ChatDlg::desiredCaption() const
{
	QString cap = "";

	if (pending_ > 0) {
		cap += "* ";
		if (pending_ > 1) {
			cap += QString("[%1] ").arg(pending_);
		}
	}
	cap += dispNick_;

	if (contactChatState_ == XMPP::StateComposing) {
		cap = tr("%1 (Composing ...)").arg(cap);
	}
	else if (contactChatState_ == XMPP::StateInactive) {
		cap = tr("%1 (Inactive)").arg(cap);
	}

	return cap;
}

void ChatDlg::invalidateTab()
{
	TabbableWidget::invalidateTab();
}

bool ChatDlg::isEncryptionEnabled() const
{
	return false;
}

bool ChatDlg::couldSendMessages() const
{
	return chatEdit()->isEnabled() &&
	       !chatEdit()->toPlainText().isEmpty() &&
	       account()->isAvailable();
}

void ChatDlg::updateSendAction()
{
#ifndef YAPSI
	act_send_->setEnabled(couldSendMessages());
#endif
}

void ChatDlg::doSend()
{
	if (!act_send_->isEnabled()) {
		return;
	}

	if (chatEdit()->toPlainText().trimmed().isEmpty()) {
		chatEdit()->clear();
		return;
	}

	if (chatEdit()->toPlainText() == "/clear") {
		chatEdit()->clear();
		doClear();
		QString line1,line2;
		MiniCommand_Depreciation_Message("/clear", "clear", line1, line2);
		appendSysMsg(line1);
		appendSysMsg(line2);
		return;
	}

	if (warnSend_) {
		warnSend_ = false;
		int n = QMessageBox::information(this, tr("Warning"), tr(
		                                     "<p>Encryption was recently disabled by the remote contact.  "
		                                     "Are you sure you want to send this message without encryption?</p>"
		                                 ), tr("&Yes"), tr("&No"));
		if (n != 0) {
			return;
		}
	}

	Message m(jid());
	m.setType("chat");
	m.setBody(chatEdit()->toPlainText());
	m.setTimeStamp(QDateTime::currentDateTime());
#ifdef YAPSI
	m.setYaFlags(YaChatViewModel::OutgoingMessage);
#endif
	if (isEncryptionEnabled()) {
		m.setWasEncrypted(true);
	}

	// we want id's to be readily available in case we need
	// to highlight an error
	m.setId(account()->client()->genUniqueId());

	// XEP-0184 Message Receipts
	// since it's fairly important to get delivery confirmation in as many
	// cases as possible, we always ask for confirmation even when
	// our chances of positive reply are slim
	m.setMessageReceipt(ReceiptRequest);

	// Request events
	if (PsiOptions::instance()->getOption("options.messages.send-composing-events").toBool()) {
		// Only request more events when really necessary
		if (sendComposingEvents_) {
			m.addEvent(ComposingEvent);
		}
		m.setChatState(XMPP::StateActive);
	}

	// Update current state
	setChatState(XMPP::StateActive);

	m_ = m;

	if (isEncryptionEnabled()) {
		chatEdit()->setEnabled(false);
		transid_ = account()->sendMessageEncrypted(m);
		if (transid_ == -1) {
			chatEdit()->setEnabled(true);
			chatEdit()->setFocus();
			return;
		}
	}
	else {
		aSend(m);
		doneSend(m);
	}

	chatEdit()->setFocus();
}

void ChatDlg::doneSend(const XMPP::Message& m)
{
	appendMessage(m, true);
	disconnect(chatEdit(), SIGNAL(textChanged()), this, SLOT(setComposing()));
	chatEdit()->clear();

	// Reset composing timer
	connect(chatEdit(), SIGNAL(textChanged()), this, SLOT(setComposing()));
	// Reset composing timer
	resetComposing();
}

void ChatDlg::encryptedMessageSent(int x, bool b, int e, const QString &dtext)
{
#ifdef HAVE_PGPUTIL
	if (transid_ == -1 || transid_ != x) {
		return;
	}
	transid_ = -1;
	if (b) {
		doneSend(m_);
	}
	else {
		PGPUtil::showDiagnosticText(static_cast<QCA::SecureMessage::Error>(e), dtext);
	}
	chatEdit()->setEnabled(true);
	chatEdit()->setFocus();
#else
	Q_ASSERT(false);
#endif
}

void ChatDlg::incomingMessage(const Message &m)
{
	if (m.body().isEmpty() && m.subject().isEmpty() && m.urlList().isEmpty()) {
		// Event message
		if (m.containsEvent(CancelEvent)) {
			setContactChatState(XMPP::StatePaused);
		}
		else if (m.containsEvent(ComposingEvent)) {
			setContactChatState(XMPP::StateComposing);
		}

		if (m.chatState() != XMPP::StateNone) {
			setContactChatState(m.chatState());
		}
	}
	else {
		// Normal message
		// Check if user requests event messages
		sendComposingEvents_ = m.containsEvent(ComposingEvent);
		if (!m.eventId().isEmpty()) {
			eventId_ = m.eventId();
		}
		if (m.containsEvents() || m.chatState() != XMPP::StateNone) {
			setContactChatState(XMPP::StateActive);
		}
		else {
			setContactChatState(XMPP::StateNone);
		}
		appendMessage(m);
	}
}

void ChatDlg::setPGPEnabled(bool enabled)
{
	Q_UNUSED(enabled);
}

QString ChatDlg::whoNick(bool local) const
{
	QString result;

	if (local) {
		result = account()->nick();
	}
	else {
		result = dispNick_;
	}

	return TextUtil::escape(result);
}

void ChatDlg::receivedPendingMessage()
{
	if (isActiveTab())
		return;

	++pending_;
	invalidateTab();
	if (PsiOptions::instance()->getOption("options.ui.flash-windows").toBool()) {
		doFlash(true);
	}
	if (PsiOptions::instance()->getOption("options.ui.chat.raise-chat-windows-on-new-messages").toBool()) {
		if (isTabbed()) {
			TabDlg* tabSet = getManagingTabDlg();
			tabSet->selectTab(this);
			::bringToFront(tabSet, false);
		}
		else {
			::bringToFront(this, false);
		}
	}
}

void ChatDlg::appendMessage(const Message &m, bool local)
{
	// figure out the encryption state
	bool encChanged = false;
	bool encEnabled = false;
	if (lastWasEncrypted_ != m.wasEncrypted()) {
		encChanged = true;
	}
	lastWasEncrypted_ = m.wasEncrypted();
	encEnabled = lastWasEncrypted_;

	if (encChanged) {
		if (encEnabled) {
			appendSysMsg(QString("<icon name=\"psi/cryptoYes\"> ") + tr("Encryption Enabled"));
			if (!local) {
				setPGPEnabled(true);
			}
		}
		else {
			appendSysMsg(QString("<icon name=\"psi/cryptoNo\"> ") + tr("Encryption Disabled"));
			if (!local) {
				setPGPEnabled(false);

				// enable warning
				warnSend_ = true;
				QTimer::singleShot(3000, this, SLOT(setWarnSendFalse()));
			}
		}
	}

	QString txt = messageText(m);
	QString subject = messageSubject(m);

	ChatDlg::SpooledType spooledType = m.spooled() ?
	                                   ChatDlg::Spooled_OfflineStorage :
	                                   ChatDlg::Spooled_None;
#ifdef YAPSI
	if (isEmoteMessage(m))
		appendEmoteMessage(spooledType, m.timeStamp(), local, m.spamFlag(), m.id(), m.messageReceipt(), txt, XMPP::YaDateTime::fromYaTime_t(m.yaMessageId()), m.yaFlags());
	else
		appendNormalMessage(spooledType, m.timeStamp(), local, m.spamFlag(), m.id(), m.messageReceipt(), txt, XMPP::YaDateTime::fromYaTime_t(m.yaMessageId()), m.yaFlags());
#else
	if (isEmoteMessage(m))
		appendEmoteMessage(spooledType, m.timeStamp(), local, txt, subject);
	else
		appendNormalMessage(spooledType, m.timeStamp(), local, txt, subject);
#endif

	appendMessageFields(m);

#ifndef YAPSI
	if (local) {
#endif
		deferredScroll();
#ifndef YAPSI
	}
#endif

	// if we're not active, notify the user by changing the title
	receivedPendingMessage();

	if (!local) {
		keepOpen_ = true;
		QTimer::singleShot(1000, this, SLOT(setKeepOpenFalse()));
	}
}

void ChatDlg::deferredScroll()
{
	QTimer::singleShot(250, this, SLOT(slotScroll()));
}

void ChatDlg::slotScroll()
{
	chatView()->scrollToBottom();
}

void ChatDlg::updateIsComposing(bool b)
{
	setChatState(b ? XMPP::StateComposing : XMPP::StatePaused);
}

void ChatDlg::setChatState(ChatState state)
{
	if (PsiOptions::instance()->getOption("options.messages.send-composing-events").toBool() && (sendComposingEvents_ || (contactChatState_ != XMPP::StateNone))) {
		// Don't send to offline resource
		QList<UserListItem*> ul = account()->findRelevant(jid());
		if (ul.isEmpty()) {
			sendComposingEvents_ = false;
			lastChatState_ = XMPP::StateNone;
			return;
		}

		UserListItem *u = ul.first();
		if (!u->isAvailable()) {
			sendComposingEvents_ = canChatState();
			lastChatState_ = XMPP::StateNone;
			return;
		}

		// Transform to more privacy-enabled chat states if necessary
		if (!PsiOptions::instance()->getOption("options.messages.send-inactivity-events").toBool() && (state == XMPP::StateGone || state == XMPP::StateInactive)) {
			state = XMPP::StatePaused;
		}

		if (lastChatState_ == XMPP::StateNone && (state != XMPP::StateActive && state != XMPP::StateComposing && state != XMPP::StateGone)) {
			//this isn't a valid transition, so don't send it, and don't update laststate
			return;
		}

		// Check if we should send a message
		if (state == lastChatState_ || state == XMPP::StateActive || (lastChatState_ == XMPP::StateActive && state == XMPP::StatePaused)) {
			lastChatState_ = state;
			return;
		}

		// Build event message
		Message m(jid());
		if (sendComposingEvents_) {
			m.setEventId(eventId_);
			if (state == XMPP::StateComposing) {
				m.addEvent(ComposingEvent);
			}
			else if (lastChatState_ == XMPP::StateComposing) {
				m.addEvent(CancelEvent);
			}
		}
		if (contactChatState_ != XMPP::StateNone) {
			if (lastChatState_ != XMPP::StateGone) {
				if ((state == XMPP::StateInactive && lastChatState_ == XMPP::StateComposing) || (state == XMPP::StateComposing && lastChatState_ == XMPP::StateInactive)) {
					// First go to the paused state
					Message m(jid());
					m.setType("chat");
					m.setChatState(XMPP::StatePaused);
					if (account()->isAvailable()) {
						account()->dj_sendMessage(m, false);
					}
				}
				m.setChatState(state);
			}
		}

		// Send event message
		if (m.containsEvents() || m.chatState() != XMPP::StateNone) {
			m.setType("chat");
			if (account()->isAvailable()) {
				account()->dj_sendMessage(m, false);
			}
		}

		// Save last state
		if (lastChatState_ != XMPP::StateGone || state == XMPP::StateActive)
			lastChatState_ = state;
	}
}

void ChatDlg::setContactChatState(ChatState state)
{
	contactChatState_ = state;
	if (state == XMPP::StateGone) {
#ifndef YAPSI
		appendSysMsg(tr("%1 ended the conversation").arg(Qt::escape(dispNick_)));
#endif
	}
	else {
		// Activate ourselves
		if (lastChatState_ == XMPP::StateGone) {
			setChatState(XMPP::StateActive);
		}
	}
	invalidateTab();
}

bool ChatDlg::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		keyPressEvent(static_cast<QKeyEvent*>(event));
		if (event->isAccepted())
			return true;
	}

	if (chatView()->handleCopyEvent(obj, event, chatEdit()))
		return true;

	return QWidget::eventFilter(obj, event);
}

void ChatDlg::addEmoticon(QString text)
{
	if (!isActiveTab())
		return;

	PsiRichText::addEmoticon(chatEdit(), text);
}

/**
 * Records that the user is composing
 */
void ChatDlg::setComposing()
{
	if (!composingTimer_) {
		/* User (re)starts composing */
		composingTimer_ = new QTimer(this);
		connect(composingTimer_, SIGNAL(timeout()), SLOT(checkComposing()));
		composingTimer_->start(2000); // FIXME: magic number
		emit composing(true);
	}
	isComposing_ = true;
}

/**
 * Checks if the user is still composing
 */
void ChatDlg::checkComposing()
{
	if (!isComposing_) {
		// User stopped composing
		composingTimer_->deleteLater();
		composingTimer_ = 0;
		emit composing(false);
	}
	isComposing_ = false; // Reset composing
}

void ChatDlg::resetComposing()
{
	if (composingTimer_) {
		delete composingTimer_;
		composingTimer_ = 0;
		isComposing_ = false;
	}
}

void ChatDlg::nicksChanged()
{
	// this function is intended to be reimplemented in subclasses
}

static const QString me_cmd = "/me ";

bool ChatDlg::isEmoteText(const QString& text)
{
	return text.startsWith(me_cmd);
}

bool ChatDlg::isEmoteMessage(const XMPP::Message& m)
{
	if (isEmoteText(m.body()) || isEmoteText(m.html().text().trimmed()))
		return true;

	return false;
}

QString ChatDlg::messageText(const XMPP::Message& m)
{
	bool emote = isEmoteMessage(m);
	QString txt;

	// TMP: ONLINE-1885
	// if (m.containsHTML() && PsiOptions::instance()->getOption("options.html.chat.render").toBool() && !m.html().text().isEmpty()) {
	// 	return messageText(m.html().toString("span"), emote, true);
	// }

	return messageText(m.body(), emote, false);
}

QString ChatDlg::messageText(const QString& text, bool isEmote, bool isHtml)
{
	return TextUtil::prepareMessageText(text, isEmote, isHtml);
}

QString ChatDlg::messageSubject(const XMPP::Message& m)
{
	QString txt = m.subject();

	if (!txt.isEmpty()) {
		txt = TextUtil::plain2rich(txt);
		txt = TextUtil::linkify(txt);
		txt = TextUtil::emoticonify(txt);
		txt = TextUtil::legacyFormat(txt);
	}
	return txt;
}

void ChatDlg::chatEditCreated()
{
	chatView()->setDialog(this);
	chatEdit()->setDialog(this);
	chatEdit()->setSendAction(act_send_);
	chatEdit()->installEventFilter(this);
	disconnect(chatEdit(), SIGNAL(textChanged()), this, SLOT(updateSendAction()));
	connect(chatEdit(), SIGNAL(textChanged()), this, SLOT(updateSendAction()));

	if (highlightersInstalled_) {
		connect(chatEdit(), SIGNAL(textChanged()), this, SLOT(setComposing()));
	}
}

TabbableWidget::State ChatDlg::state() const
{
	return contactChatState_ == XMPP::StateComposing ?
	       TabbableWidget::StateComposing :
	       TabbableWidget::StateNone;
}

int ChatDlg::unreadMessageCount() const
{
	return pending_;
}

QAction* ChatDlg::actionSend() const
{
	return act_send_;
}

bool ChatDlg::canChatState() const
{
	if (!account() || !account()->capsManager())
		return false;

	XMPP::Jid j = jid();
	if (j.resource().isEmpty()) {
		QList<UserListItem*> ul = account()->findRelevant(jid());
		if (!ul.isEmpty()) {
			UserListItem* u = ul.first();
			UserResourceList::Iterator rit = u->userResourceList().priority();
			if (rit != u->userResourceList().end()) {
				j = j.withResource((*rit).name());
			}
		}
	}

	return account()->capsManager()->features(j).canChatState();
}
