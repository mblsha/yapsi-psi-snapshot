/*
 * yachatviewmodel.h - stores items of custom chatlog
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

#ifndef YACHATVIEWMODEL_H
#define YACHATVIEWMODEL_H

#include <QStandardItemModel>
#include <QHash>

#include "xmpp_status.h"
#include "xmpp_receipts.h"
#include "xmpp_vcard.h"
#include "xmpp_yadatetime.h"

class DeliveryConfirmationManager;
class YaChatViewModelNotice;

class YaChatViewModel : public QStandardItemModel
{
	Q_OBJECT
public:
	YaChatViewModel(DeliveryConfirmationManager* deliveryConfirmationManager);
	~YaChatViewModel();

	static bool canRelyOnTimestamps();

	enum YaMessageFlagType {
		NoFlags = 0,
		OutgoingMessage
	};

	enum {
		TypeRole         = Qt::UserRole + 0,
		DelegateDataRole = Qt::UserRole + 1,
		PersistentNoticeRole = Qt::UserRole + 2, // bool

		// Type = Message
		// DisplayRole stores the actual message text (plain-text or XHTML-IM)
		IncomingRole         = Qt::UserRole + 3, // bool
		MergeRole            = Qt::UserRole + 4, // bool, if following the message with the same incoming value (automatically calculated when appending message)
		DateTimeRole         = Qt::UserRole + 5, // QDateTime
		MessagePlainTextRole = Qt::UserRole + 6, // bool, true if MessageRole is plain-text (probably should ditch it)
		SpooledRole          = Qt::UserRole + 7, // SpooledType, true for offline messages and queued history loads
		EmoteRole            = Qt::UserRole + 8, // bool, true for /me messages
		IdRole               = Qt::UserRole + 9, // QString
		DeliveryConfirmationRole = Qt::UserRole + 10, // DeliveryConfirmationType
		ErrorRole            = Qt::UserRole + 11, // QString
#ifdef YAPSI
		SpamRole             = Qt::UserRole + 12, // int, 0 for not spam, >0 for marked as spam
#endif
		YaDateTimeRole       = Qt::UserRole + 13, // YaDateTime
		YaFlagsRole          = Qt::UserRole + 14  // YaMessageFlagType

		// Type = DateHeader
		// DateTimeRole is reused here

		// Type = FileTransfer
		// Type = SomethingElse
	};

	enum Type {
		DummyHeader,
		Message,
		DateHeader,
		ContactComposing,
		UserIsOfflineNotice,
		UserIsBlockedNotice,
		UserNotInListNotice,
		NotAuthorizedToSeeUserStatusNotice,
		AccountIsOfflineNotice,
		AccountIsDisabledNotice,
		StatusTypeChangedNotice,
		MoodChanged,
		EmptyChatViewModelNotice
	};

	enum SpooledType {
		Spooled_None,
		Spooled_OfflineStorage,
		Spooled_History,

		Spooled_Sync
	};

	enum DeliveryConfirmationType {
		DeliveryConfirmation_Unknown,
		DeliveryConfirmation_Querying,
		DeliveryConfirmation_QueryingWithoutTimeout,
		DeliveryConfirmation_Verified,
		DeliveryConfirmation_Error,
		DeliveryConfirmation_Timeout
	};

	void doClear();

	const XMPP::Jid& jid() const;
	void setJid(const XMPP::Jid& jid);

#ifndef YAPSI
	void addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt);
	void addMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt);
#else
	void addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags);
	void addMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags);
#endif
	// void addMessage(bool incoming, const QString& msg);

	bool composingEventVisible() const;
	void setComposingEventVisible(bool visible);

	bool userIsOfflineNoticeVisible() const;
	void setUserIsOfflineNoticeVisible(bool visible);

	bool userIsBlockedNoticeVisible() const;
	void setUserIsBlockedNoticeVisible(bool visible);

	bool userNotInListNoticeVisible() const;
	void setUserNotInListNoticeVisible(bool visible);

	bool notAuthorizedToSeeUserStatusNoticeVisible() const;
	void setNotAuthorizedToSeeUserStatusNoticeVisible(bool visible);

	bool accountIsOfflineNoticeVisible() const;
	void setAccountIsOfflineNoticeVisible(bool visible);

	bool accountIsDisabledNoticeVisible() const;
	void setAccountIsDisabledNoticeVisible(bool visible);

	void setStatusTypeChangedNotice(XMPP::Status::Type status);
	void addMoodChange(SpooledType spooled, const QString& mood, const QDateTime& timeStamp, const XMPP::YaDateTime& yaTime);

	XMPP::VCard::Gender userGender() const;
	void setUserGender(XMPP::VCard::Gender gender);

	bool historyReceived() const;
	void setHistoryReceived(bool historyReceived);

	static YaChatViewModel::Type type(const QModelIndex& index);
	static YaChatViewModel::SpooledType spooledType(const QStandardItem* item);
	static YaChatViewModel::SpooledType spooledType(const QModelIndex& index);

private slots:
	void updateNotices();
	void messageError(const QString& id, const QString& error);
	void deliveryConfirmationUpdated(const QString& id, YaChatViewModel::DeliveryConfirmationType deliveryConfirmation);
	void doRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
	void updateEmptyChatViewModelNotice();
	void optionChanged(const QString& option);

private:
	Type childType(const QStandardItem* item) const;
	QStandardItem* lastMessageOrDateHeader() const;
	QStandardItem* lastMessage() const;
	bool ensureDateHeader(const QDateTime& time, const XMPP::YaDateTime& yaTime);
	bool shouldAddDateHeader(const QDateTime& time, const XMPP::YaDateTime& yaTime) const;
	bool shouldMergeWith(QStandardItem* message, const QDateTime& time, bool emote, bool local, YaChatViewModel::SpooledType spooled) const;
	bool shouldMergeWith(const QDateTime& time, bool emote, bool local, YaChatViewModel::SpooledType spooled) const;
	void updateMergeRole(int indexToUpdate, int indexPrev);
	bool sameDateTime(const XMPP::YaDateTime& ydt1, const QDateTime& dt1, const XMPP::YaDateTime& ydt2, const QDateTime& dt2) const;
	bool processSpooledItem(QStandardItem* i1, QStandardItem* i2) const;
	bool sameItem(QStandardItem* i1, QStandardItem* i2) const;
	bool isNext(QStandardItem* i1, QStandardItem* i2) const;
	bool isPrev(QStandardItem* i1, QStandardItem* i2) const;
#ifndef YAPSI
	void addMessageHelper(YaChatViewModel::SpooledType spooled, const QDateTime& time, bool local, QString txt, bool emote);
#else
	void addMessageHelper(YaChatViewModel::SpooledType spooled, const QDateTime& time, bool local, QString txt, bool emote, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, const XMPP::YaDateTime& yaTime, int yaFlags);
#endif
	void addDateHeader(int index, const QDateTime& time, const XMPP::YaDateTime& yaTime) const;
	void addDateHeader(const QDateTime& time, const XMPP::YaDateTime& yatime);
	int newMessageRow() const;
	void addDummyHeader();

	void appendNotice(QStandardItem* item);

	static bool yaChatViewModelNoticeLessThan(const YaChatViewModelNotice* n1, const YaChatViewModelNotice* n2);

	DeliveryConfirmationManager* deliveryConfirmationManager_;
	QTimer* updateNoticesTimer_;
	YaChatViewModelNotice* composingEvent_;
	YaChatViewModelNotice* userIsOfflineNotice_;
	YaChatViewModelNotice* userIsBlockedNotice_;
	YaChatViewModelNotice* userNotInListNotice_;
	YaChatViewModelNotice* notAuthorizedToSeeUserStatusNotice_;
	YaChatViewModelNotice* accountIsOfflineNotice_;
	YaChatViewModelNotice* accountIsDisabledNotice_;
	YaChatViewModelNotice* statusTypeChangedNotice_;
	YaChatViewModelNotice* emptyChatViewModelNotice_;
	QStandardItem* dummyHeader_;
	QList<YaChatViewModelNotice*> notices_;
	QHash<QString, QStandardItem*> ids_;
	QHash<XMPP::YaDateTime, bool> timeStamps_;
	XMPP::Jid jid_;
	XMPP::VCard::Gender userGender_;
	bool historyReceived_;
	int messageCount_;
};

class YaChatViewModelNotice : public QObject
{
	Q_OBJECT
public:
	YaChatViewModelNotice(YaChatViewModel* parent, YaChatViewModel::Type type);

	int priority() const;
	void setPriority(int priority);

	bool isVisible() const;
	void setVisible(bool visible);

	bool shouldBeVisible() const;
	void setShouldBeVisible(bool visible);

	void updateMessage();
	QString message() const;
	void setMessage(const QString& message);

	bool persistent() const;
	void setPersistent(bool persistent);

private:
	YaChatViewModel::Type type_;
	bool shouldBeVisible_;
	int priority_;
	QString message_;
	bool persistent_;
	YaChatViewModel* model_;
	QStandardItem* item_;

	bool useMaleMsg() const;
	QString messageByType(YaChatViewModel::Type type) const;
};

#endif
