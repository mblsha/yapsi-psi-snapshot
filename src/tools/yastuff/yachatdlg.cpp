/*
 * yachatdlg.cpp - custom chat dialog
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

#include <QPainter>
#include <QIcon>
#include <QTextDocument> // for Qt::escape()
#include <QMouseEvent>

#include "yachatdlg.h"
#include "yatrayicon.h"

#include "iconselect.h"
#include "textutil.h"
#include "iconselect.h"
#include "yacommon.h"
#include "yacontactlabel.h"
#include "yaselflabel.h"
#include "psicon.h"
#include "psicontact.h"
#include "psicon.h"
#include "psiaccount.h"
#include "userlist.h"
#include "yachattooltip.h"
#include "iconset.h"
#include "yachatviewmodel.h"
#include "globaleventqueue.h"
#include "xmpp_message.h"
#include "yavisualutil.h"
#include "yaprivacymanager.h"
#include "applicationinfo.h"
#include "tabdlg.h"
#include "yapushbutton.h"
#include "psioptions.h"
#include "psiiconset.h"
#include "deliveryconfirmationmanager.h"
#include "yahistorycachemanager.h"
#include "psilogger.h"
#include "vcardfactory.h"
#include "xmpp_yadatetime.h"
#include "yachatseparator.h"
#include "msgmle.h"

static const QString emoticonsEnabledOptionPath = "options.ya.emoticons-enabled";
static const QString enableTypographyOptionPath = "options.ya.typography.enable";
static const QString textColorOptionPath = "options.ya.chat-window.text-color";
static const QString spellCheckEnabledOptionPath = "options.ui.spell-check.enabled";
static const QString sendButtonEnabledOptionPath = "options.ya.chat-window.send-button.enabled";

class YaChatDlgShared : public QObject
{
	Q_OBJECT
public:
	QAction* typographyAction() const { return typographyAction_; }
	QAction* emoticonsAction() const { return emoticonsAction_; }
	QAction* checkSpellingAction() const { return checkSpellingAction_; }
	QAction* sendButtonEnabledAction() const { return sendButtonEnabledAction_; }

	static YaChatDlgShared* instance()
	{
		if (!instance_) {
			instance_ = new YaChatDlgShared();
		}
		return instance_;
	}

private slots:
	void typographyActionTriggered(bool enabled)
	{
		PsiOptions::instance()->setOption(enableTypographyOptionPath, enabled);
	}

	void emoticonsActionTriggered(bool enabled)
	{
		PsiOptions::instance()->setOption("options.ui.emoticons.use-emoticons", enabled);
		PsiOptions::instance()->setOption(emoticonsEnabledOptionPath, enabled);
	}

	void checkSpellingActionTriggered(bool enabled)
	{
		PsiOptions::instance()->setOption(spellCheckEnabledOptionPath, enabled);
	}

	void sendButtonEnabledActionTriggered(bool enabled)
	{
		PsiOptions::instance()->setOption(sendButtonEnabledOptionPath, enabled);
	}

	void optionChanged(const QString& option)
	{
		if (option == emoticonsEnabledOptionPath) {
			emoticonsAction_->setChecked(PsiOptions::instance()->getOption(emoticonsEnabledOptionPath).toBool());
		}
		else if (option == enableTypographyOptionPath) {
			typographyAction_->setChecked(PsiOptions::instance()->getOption(enableTypographyOptionPath).toBool());
		}
		else if (option == spellCheckEnabledOptionPath) {
			checkSpellingAction_->setChecked(PsiOptions::instance()->getOption(spellCheckEnabledOptionPath).toBool());
		}
		else if (option == sendButtonEnabledOptionPath) {
			sendButtonEnabledAction_->setChecked(PsiOptions::instance()->getOption(sendButtonEnabledOptionPath).toBool());
		}
	}

private:
	YaChatDlgShared()
		: QObject(QCoreApplication::instance())
	{
		typographyAction_ = new QAction(tr("Typographica"), this);
		typographyAction_->setCheckable(true);
		connect(typographyAction_, SIGNAL(triggered(bool)), SLOT(typographyActionTriggered(bool)));

		emoticonsAction_ = new QAction(tr("Enable emoticons"), this);
		emoticonsAction_->setCheckable(true);
		connect(emoticonsAction_, SIGNAL(triggered(bool)), SLOT(emoticonsActionTriggered(bool)));

		checkSpellingAction_ = new QAction(tr("Check spelling"), this);
		checkSpellingAction_->setCheckable(true);
		connect(checkSpellingAction_, SIGNAL(triggered(bool)), SLOT(checkSpellingActionTriggered(bool)));

		sendButtonEnabledAction_ = new QAction(tr("Show 'Send' button"), this);
		sendButtonEnabledAction_->setCheckable(true);
		connect(sendButtonEnabledAction_, SIGNAL(triggered(bool)), SLOT(sendButtonEnabledActionTriggered(bool)));

		connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
		optionChanged(emoticonsEnabledOptionPath);
		optionChanged(enableTypographyOptionPath);
		optionChanged(spellCheckEnabledOptionPath);
		optionChanged(sendButtonEnabledOptionPath);
	}

	static YaChatDlgShared* instance_;
	QPointer<QAction> typographyAction_;
	QPointer<QAction> emoticonsAction_;
	QPointer<QAction> checkSpellingAction_;
	QPointer<QAction> sendButtonEnabledAction_;
};

YaChatDlgShared* YaChatDlgShared::instance_ = 0;

//----------------------------------------------------------------------------
// YaChatDlg
//----------------------------------------------------------------------------

YaChatDlg::YaChatDlg(const Jid& jid, PsiAccount* acc, TabManager* tabManager)
	: ChatDlg(jid, acc, tabManager)
	, selfProfile_(YaProfile::create(acc))
	, contactProfile_(YaProfile::create(acc, jid))
	, model_(0)
	, showAuthButton_(true)
{
	model_ = new YaChatViewModel(acc->deliveryConfirmationManager());
	connect(this, SIGNAL(invalidateTabInfo()), SLOT(updateComposingMessage()));

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
	connect(account(), SIGNAL(updatedActivity()), SLOT(updateModelNotices()));
	connect(account(), SIGNAL(enabledChanged()), SLOT(updateModelNotices()));
	QTimer::singleShot(0, this, SLOT(updateModelNotices()));
}

YaChatDlg::~YaChatDlg()
{
	LOG_TRACE;
	delete model_;
}

void YaChatDlg::initUi()
{
	// setFrameStyle(QFrame::StyledPanel);
	ui_.setupUi(this);

	YaChatContactStatus* contactStatus = new YaChatContactStatus(ui_.contactStatus->parentWidget());
	replaceWidget(ui_.contactStatus, contactStatus);
	ui_.contactStatus = contactStatus;

	// connect(ui_.mle, SIGNAL(textEditCreated(QTextEdit*)), SLOT(chatEditCreated()));
	chatEditCreated();

#if 0
	YaChatContactInfo *contactInfoOld = ui_.contactInfo;
	ui_.contactInfo = new YaChatContactInfo(ui_.contactInfo->parentWidget());
	ui_.contactInfo->setMaximumSize(1, 1);
	replaceWidget(contactInfoOld, ui_.contactInfo);
	ui_.contactInfo->raiseExtraInWidgetStack();
#endif
	ui_.contactInfo->setMode(YaChatContactInfoExtra::Button);

	connect(ui_.contactInfo, SIGNAL(clicked()), SLOT(showContactProfile()));

	// connect(ui_.sendButton, SIGNAL(clicked()), SLOT(doSend()));
	// connect(ui_.historyButton, SIGNAL(clicked()), SLOT(doHistory()));
	// connect(ui_.buzzButton, SIGNAL(clicked()), SLOT(sendBuzz()));
	// connect(ui_.profileButton, SIGNAL(clicked()), SLOT(doInfo()));
	// connect(account()->psi()->iconSelectPopup(), SIGNAL(textSelected(QString)), SLOT(addEmoticon(QString)));
	// ui_.smileysButton->setMenu(account()->psi()->iconSelectPopup());
	// 
	// ui_.selfUserpic->setMode(YaSelfAvatarLabel::OpenProfile);
	// 
	// ui_.fontButton->hide();
	// ui_.backgroundButton->hide();
	// 
	// ui_.selfUserpic->setContactList(account()->psi()->contactList());
	// ui_.selfName->setContactList(account()->psi()->contactList());
	ui_.contactUserpic->setProfile(contactProfile_);
	ui_.contactName->setProfile(contactProfile_);
	ui_.chatView->setModel(model_);
	QTimer::singleShot(0, this, SLOT(updateContactName()));

	ui_.contactName->setMinimumSize(100, 30);

	connect(ui_.bottomFrame->separator(), SIGNAL(textSelected(QString)), SLOT(addEmoticon(QString)));
	connect(ui_.bottomFrame->separator(), SIGNAL(addContact()), SLOT(addContact()));
	connect(ui_.bottomFrame->separator(), SIGNAL(authContact()), SLOT(authContact()));

	{
		if (PsiIconset::instance()->yaEmoticonSelectorIconset()) {
			ui_.bottomFrame->separator()->setIconset(*PsiIconset::instance()->yaEmoticonSelectorIconset());
		}
	}

	foreach(QPushButton* button, findChildren<QPushButton*>())
		if (button->objectName().startsWith("dummyButton"))
			button->setVisible(false);
	// ui_.settingsButton->setVisible(false);

	// workaround for Qt bug
	// http://www.trolltech.com/developer/task-tracker/index_html?id=150562&method=entry
	// Since we're not using tables all that much, the problem doesn't occur anymore even on Qt 4.3.0
	// chatView()->setWordWrapMode(QTextOption::WrapAnywhere);

	resize(sizeHint());
	doClear();
}

void YaChatDlg::initActions()
{
	ChatDlg::initActions();
	ui_.bottomFrame->setSendAction(actionSend());
}

void YaChatDlg::restoreLastMessages()
{
	account()->psi()->yaHistoryCacheManager()->getMessagesFor(account(), jid(), this, "retrieveHistoryFinished");

	// we need to run this after PsiAccount::processChatsHelper() finishes its work
	// so we won't end up with dupes in chatlog
	QTimer::singleShot(0, this, SLOT(retrieveHistoryFinishedHelper()));
}

void YaChatDlg::retrieveHistoryFinished()
{
	retrieveHistoryFinishedHelper();
	model_->setHistoryReceived(true);
}

void YaChatDlg::retrieveHistoryFinishedHelper()
{
	SpooledType spooled = model_->historyReceived() ? Spooled_Sync : Spooled_History;

	QList<YaHistoryCacheManager::Message> list = account()->psi()->yaHistoryCacheManager()->getCachedMessagesFor(account(), jid());
	qSort(list.begin(), list.end(), yaHistoryCacheManagerMessageMoreThan);
	foreach(const YaHistoryCacheManager::Message& msg, list) {
		if (!msg.isMood) {
			if (isEmoteText(msg.body))
				appendEmoteMessage(spooled, msg.timeStamp, msg.originLocal, false, QString(), XMPP::ReceiptNone, messageText(msg.body, true), msg.timeStamp, YaChatViewModel::NoFlags);
			else
				appendNormalMessage(spooled, msg.timeStamp, msg.originLocal, false, QString(), XMPP::ReceiptNone, messageText(msg.body, false), msg.timeStamp, YaChatViewModel::NoFlags);
		}
		else {
			Q_ASSERT(!msg.originLocal);
			addMoodChange(spooled, msg.body, msg.timeStamp);
		}
	}
}

void YaChatDlg::doHistory()
{
	Ya::showHistory(selfProfile_->account(), contactProfile_->jid());
}

void YaChatDlg::doSend()
{
	if (!couldSendMessages()) {
		return;
	}

#if 0 // ONLINE-1988
	YaPrivacyManager* privacyManager = dynamic_cast<YaPrivacyManager*>(account()->privacyManager());
	if (privacyManager && privacyManager->isContactBlocked(jid())) {
		privacyManager->setContactBlocked(jid(), false);
	}
#endif

	ChatDlg::doSend();
}

void YaChatDlg::showEvent(QShowEvent* e)
{
	optionsUpdate();
	ChatDlg::showEvent(e);
}

void YaChatDlg::capsChanged()
{
}

bool YaChatDlg::isEncryptionEnabled() const
{
	return false;
}

void YaChatDlg::contactUpdated(UserListItem* u, int status, const QString& statusString)
{
	ChatDlg::contactUpdated(u, status, statusString);
	PsiContact* contact = account()->findContact(jid().bare());

	XMPP::Status::Type statusType = XMPP::Status::Offline;
	if (status != -1)
		statusType = static_cast<XMPP::Status::Type>(status);

	ui_.contactStatus->realWidget()->setStatus(contact ? contact->status().type() : statusType, statusString, gender());
	ui_.contactUserpic->setStatus(statusType);

	if (lastStatus_.type() != statusType) {
		model_->setStatusTypeChangedNotice(statusType);
	}

	lastStatus_ = XMPP::Status(statusType, statusString);

	updateModelNotices();
	// ui_.addToFriendsButton->setEnabled(!Ya::isInFriends(u));
}

void YaChatDlg::addMoodChange(SpooledType spooled, const QString& mood, const QDateTime& timeStamp)
{
#if 0 // ONLINE-1932
	model_->addMoodChange(static_cast<YaChatViewModel::SpooledType>(spooled), mood, timeStamp);
#endif
}

void YaChatDlg::updateModelNotices()
{
	PsiContact* contact = account()->findContact(jid().bare());

	model_->setUserIsBlockedNoticeVisible(contact && contact->isBlocked());
	model_->setUserIsOfflineNoticeVisible(ui_.contactStatus->realWidget()->status() == XMPP::Status::Offline);
	model_->setUserNotInListNoticeVisible(!contact || !contact->inList());
	model_->setNotAuthorizedToSeeUserStatusNoticeVisible(contact && !contact->authorizesToSeeStatus());
	model_->setAccountIsOfflineNoticeVisible(!account()->isAvailable());
	model_->setAccountIsDisabledNoticeVisible(!account()->enabled());

	ui_.bottomFrame->separator()->setShowAddButton(contact && (contact->addAvailable() || (contact->isBlocked() && account()->isAvailable())));
	ui_.bottomFrame->separator()->setShowAuthButton(!ui_.bottomFrame->separator()->showAddButton() && contact && contact->authAvailable() && showAuthButton_);
}

void YaChatDlg::updateAvatar()
{
	// TODO
}

void YaChatDlg::optionsUpdate()
{
	ChatDlg::optionsUpdate();
}

QString YaChatDlg::colorString(bool local, ChatDlg::SpooledType spooled) const
{
	return Ya::colorString(local, spooled);
}

void YaChatDlg::appendSysMsg(const QString &str)
{
	// TODO: add a new type to the model
#ifndef YAPSI
	appendNormalMessage(Spooled_None, QDateTime::currentDateTime(), false, str);
#else
	appendNormalMessage(Spooled_None, QDateTime::currentDateTime(), false, 0, QString(), XMPP::ReceiptNone, str, XMPP::YaDateTime(), YaChatViewModel::NoFlags);
#endif
}

void YaChatDlg::appendEmoteMessage(ChatDlg::SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags)
{
	model_->addEmoteMessage(static_cast<YaChatViewModel::SpooledType>(spooled), time, local, spamFlag, id, messageReceipt, txt, yaTime, yaFlags);
}

void YaChatDlg::appendNormalMessage(ChatDlg::SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags)
{
	model_->addMessage(static_cast<YaChatViewModel::SpooledType>(spooled), time, local, spamFlag, id, messageReceipt, txt, yaTime, yaFlags);
}

void YaChatDlg::appendMessageFields(const Message& m)
{
	QString txt;
	if (!m.subject().isEmpty()) {
		txt += QString("<b>") + tr("Subject:") + "</b> " + QString("%1").arg(Qt::escape(m.subject()));
	}
	if (!m.urlList().isEmpty()) {
		UrlList urls = m.urlList();
		txt += QString("<i>") + tr("-- Attached URL(s) --") + "</i>";
		for (QList<Url>::ConstIterator it = urls.begin(); it != urls.end(); ++it) {
			const Url &u = *it;
			txt += QString("<b>") + tr("URL:") + "</b> " + QString("%1").arg(TextUtil::linkify(Qt::escape(u.url())));
			txt += QString("<b>") + tr("Desc:") + "</b> " + QString("%1").arg(u.desc());
		}
	}

	if (txt.isEmpty()) {
		return;
	}

	ChatDlg::SpooledType spooledType = m.spooled() ?
	        Spooled_OfflineStorage :
	        Spooled_None;
#ifndef YAPSI
	appendNormalMessage(spooledType, m.timeStamp(), false, txt);
#else
	appendNormalMessage(spooledType, m.timeStamp(), false, m.spamFlag(), m.id(), m.messageReceipt(), txt, XMPP::YaDateTime::fromYaTime_t(m.yaMessageId()), m.yaFlags());
#endif
}

ChatViewClass* YaChatDlg::chatView() const
{
	return ui_.chatView;
}

ChatEdit* YaChatDlg::chatEdit() const
{
	return ui_.bottomFrame->chatEdit();
}

void YaChatDlg::showContactProfile()
{
	PsiContact* contact = account()->findContact(jid().bare());
	// FIXME: won't work after contact was deleted
	if (contact) {
#if 0
		QRect rect = ui_.contactInfo->extraGeometry();
#else
		QRect rect = QRect(ui_.contactInfo->mapToGlobal(ui_.contactInfo->rect().topLeft()),
		                   ui_.contactInfo->mapToGlobal(ui_.contactInfo->rect().bottomRight()));
#endif
		YaChatToolTip::instance()->showText(rect, contact, 0, 0);
	}
}

bool YaChatDlg::eventFilter(QObject* obj, QEvent* e)
{
	return ChatDlg::eventFilter(obj, e);
}

void YaChatDlg::doClear()
{
	model_->doClear();
}

void YaChatDlg::nicksChanged()
{
	ui_.chatView->nicksChanged(whoNick(true), whoNick(false));
}

XMPP::VCard::Gender YaChatDlg::gender() const
{
	PsiContact* contact = account()->findContact(jid());
	return contact ? contact->gender() : XMPP::VCard::UnknownGender;
}

void YaChatDlg::updateContact(const Jid &j, bool fromPresence)
{
	ChatDlg::updateContact(j, fromPresence);
	model_->setUserGender(gender());
}

void YaChatDlg::updateComposingMessage()
{
	bool enable = state() == TabbableWidget::StateComposing;
	// if (enable != model_->composingEventVisible())
	// 	model_->setComposingEventVisible(enable);
	if (enable != ui_.chatView->composingEventVisible())
		ui_.chatView->setComposingEventVisible(enable);
}

/**
 * Makes sure widget don't nastily overlap when trying to resize the dialog
 * to the smallest possible size
 */
QSize YaChatDlg::minimumSizeHint() const
{
	QSize sh = ChatDlg::minimumSizeHint();
	sh.setWidth(qMax(200, sh.width()));
	return sh;
}

void YaChatDlg::setJid(const Jid& j)
{
	ChatDlg::setJid(j);
	model_->setJid(j);
}

void YaChatDlg::aboutToShow()
{
	ChatDlg::aboutToShow();

	// FIXME: it'd be good to have updateChatEditHeight() here for cases
	// when number of tab rows changed since this YaChatDlg was last
	// visible to avoid jump when it appears, but it doesn't actually work
	// when YaChatDlg is created for the first time due to font size
	// not instantly propagated to LineEdit (due to QCSS?)
	// ui_.bottomFrame->separator()->updateChatEditHeight();

	ui_.bottomFrame->layout()->activate();
	ui_.chatSplitter->layout()->activate();
}

void YaChatDlg::receivedPendingMessage()
{
#ifndef YAPSI_ACTIVEX_SERVER
	ChatDlg::receivedPendingMessage();
#endif
}

void YaChatDlg::addPendingMessage()
{
	ChatDlg::receivedPendingMessage();
}

void YaChatDlg::resizeEvent(QResizeEvent* e)
{
	ChatDlg::resizeEvent(e);
	updateContactName();
}

void YaChatDlg::updateContactName()
{
	if (getManagingTabDlg()) {
		ui_.contactName->updateEffectiveWidth(getManagingTabDlg()->windowExtra());
	}
}

void YaChatDlg::chatEditCreated()
{
	ChatDlg::chatEditCreated();

	ui_.bottomFrame->separator()->setChatWidgets(chatEdit(), chatView());
	chatEdit()->setTypographyAction(YaChatDlgShared::instance()->typographyAction());
	chatEdit()->setEmoticonsAction(YaChatDlgShared::instance()->emoticonsAction());
	chatEdit()->setCheckSpellingAction(YaChatDlgShared::instance()->checkSpellingAction());
	chatEdit()->setSendButtonEnabledAction(YaChatDlgShared::instance()->sendButtonEnabledAction());
	optionChanged(textColorOptionPath);
}

void YaChatDlg::setLooks()
{
	ChatDlg::setLooks();

	ui_.bottomFrame->separator()->updateChatEditHeight();
}

void YaChatDlg::addContact()
{
	// TODO: need some sort of central dispatcher for this kind of stuff
	emit YaRosterToolTip::instance()->addContact(jid(), account(), QStringList(), QString());
}

void YaChatDlg::authContact()
{
	PsiContact* contact = account()->findContact(jid().bare());
	if (contact) {
		contact->rerequestAuthorizationFrom();
	}

	showAuthButton_ = false;
	updateModelNotices();
}

void YaChatDlg::closed()
{
	showAuthButton_ = true;
	updateModelNotices();

	ChatDlg::closed();
}

void YaChatDlg::activated()
{
	ChatDlg::activated();
	updateContactName();

	if (model_->historyReceived()) {
		account()->psi()->yaHistoryCacheManager()->getMessagesFor(account(), jid(), this, "retrieveHistoryFinished");
	}
}

void YaChatDlg::optionChanged(const QString& option)
{
	if (option == textColorOptionPath) {
		chatView()->setTextColor(PsiOptions::instance()->getOption(textColorOptionPath).value<QColor>());
	}
}

#include "yachatdlg.moc"
