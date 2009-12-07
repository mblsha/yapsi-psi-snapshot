/*
 * yachatviewmodel.cpp - stores items of custom chatlog
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

#include "yachatviewmodel.h"

#include <QDateTime>
#include <QTextDocument> // for Qt::mightBeRichText()
#include <QTimer>

#ifndef NO_TEXTUTIL
#include "textutil.h"
#endif
#ifndef NO_YACOMMON
#include "yacommon.h"
#endif
#ifndef NO_DELIVERY_CONFIRMATION
#include "deliveryconfirmationmanager.h"
#endif
#include "psioptions.h"

static const int mergeInterval = 5; // in minutes

//----------------------------------------------------------------------------
// YaChatViewModelNotice
//----------------------------------------------------------------------------

YaChatViewModelNotice::YaChatViewModelNotice(YaChatViewModel* parent, YaChatViewModel::Type type)
	: QObject(parent)
	, type_(type)
	, shouldBeVisible_(false)
	, priority_(0)
	, persistent_(true)
	, model_(parent)
	, item_(0)
{
}

int YaChatViewModelNotice::priority() const
{
	return priority_;
}

void YaChatViewModelNotice::setPriority(int priority)
{
	priority_ = priority;
}

bool YaChatViewModelNotice::shouldBeVisible() const
{
	return shouldBeVisible_;
}

void YaChatViewModelNotice::setShouldBeVisible(bool visible)
{
	shouldBeVisible_ = visible;
}

bool YaChatViewModelNotice::isVisible() const
{
	return item_;
}

QString YaChatViewModelNotice::message() const
{
	return message_;
}

void YaChatViewModelNotice::setMessage(const QString& message)
{
	if (message_ != message) {
		message_ = message;
		if (isVisible()) {
			setVisible(false);
			setVisible(true);
		}
	}
}

bool YaChatViewModelNotice::persistent() const
{
	return persistent_;
}

void YaChatViewModelNotice::setPersistent(bool persistent)
{
	persistent_ = persistent;
}

void YaChatViewModelNotice::setVisible(bool visible)
{
	if (visible == isVisible())
		return;

	if (!visible && isVisible()) {
		model_->invisibleRootItem()->removeRow(item_->row());
		item_ = 0;
	}

	if (visible && !isVisible()) {
		item_ = new QStandardItem(message_);
		item_->setData(QVariant(type_), YaChatViewModel::TypeRole);
		item_->setData(QVariant(persistent_), YaChatViewModel::PersistentNoticeRole);
		item_->setData(QVariant(false), YaChatViewModel::MessagePlainTextRole);
		item_->setData(QVariant(true), YaChatViewModel::IncomingRole);
		item_->setData(QVariant(true), YaChatViewModel::MergeRole);
		item_->setData(QVariant(true), YaChatViewModel::EmoteRole);
		item_->setData(QVariant(QDateTime::currentDateTime()), YaChatViewModel::DateTimeRole);
		item_->setData(QVariant(YaChatViewModel::Spooled_None), YaChatViewModel::SpooledRole);
		model_->invisibleRootItem()->appendRow(item_);
	}
}

void YaChatViewModelNotice::updateMessage()
{
	setMessage(messageByType(type_));
}

bool YaChatViewModelNotice::useMaleMsg() const
{
#ifndef NO_YACOMMON
	return Ya::useMaleMessage(model_->userGender());
#else
	return true;
#endif
}

QString YaChatViewModelNotice::messageByType(YaChatViewModel::Type type) const
{
	QString msg;
	switch (type) {
	case YaChatViewModel::ContactComposing:
		msg = tr(" is composing a message...");
		break;
	case YaChatViewModel::UserIsOfflineNotice:
		msg = useMaleMsg() ?
		      tr(" is currently offline. You could send a message and it"
		         " will be delivered when the contact comes back online.", "contact is male") :
		      tr(" is currently offline. You could send a message and it"
		         " will be delivered when the contact comes back online.", "contact is female");
		break;
	case YaChatViewModel::UserIsBlockedNotice:
		msg = useMaleMsg() ?
		      tr(" is blocked. That is you are unable to send the messages"
		         " and to view the online status of each other. To unblock"
		         " the contact simply add it to your roster.", "contact is male") :
		      tr(" is blocked. That is you are unable to send the messages"
		         " and to view the online status of each other. To unblock"
		         " the contact simply add it to your roster.", "contact is female");
		break;
	case YaChatViewModel::UserNotInListNotice:
		msg = useMaleMsg() ?
		      tr(" is not in contact list. You need to add him to your "
		         "contact list in order to see the online status of each "
		         "other.", "contact is male") :
		      tr(" is not in contact list. You need to add him to your "
		         "contact list in order to see the online status of each "
		         "other.", "contact is female");
		break;
	case YaChatViewModel::NotAuthorizedToSeeUserStatusNotice:
		msg = useMaleMsg() ?
		      tr("You're not authorized to see the online "
		         "status of this contact.", "contact is male") :
		      tr("You're not authorized to see the online "
		         "status of this contact.", "contact is female");
		break;
	case YaChatViewModel::AccountIsOfflineNotice:
		msg = tr("You're currently offline. You can't send or receive"
		         " messages unless you became online.");
		break;
	case YaChatViewModel::AccountIsDisabledNotice:
		msg = tr("This account is currently disabled. You can't send or"
		         " receive messages unless you enable it.");
		break;
	case YaChatViewModel::StatusTypeChangedNotice:
		msg = message_;
		break;
	case YaChatViewModel::EmptyChatViewModelNotice:
		msg = tr("Your message could be here :-)");
		break;
	default:
		Q_ASSERT(false);
	}
	return msg;
}

//----------------------------------------------------------------------------
// YaChatViewModel
//----------------------------------------------------------------------------

YaChatViewModel::YaChatViewModel(DeliveryConfirmationManager* deliveryConfirmationManager)
	: deliveryConfirmationManager_(deliveryConfirmationManager)
	, updateNoticesTimer_(0)
	, composingEvent_(0)
	, userIsOfflineNotice_(0)
	, userIsBlockedNotice_(0)
	, userNotInListNotice_(0)
	, notAuthorizedToSeeUserStatusNotice_(0)
	, accountIsOfflineNotice_(0)
	, accountIsDisabledNotice_(0)
	, statusTypeChangedNotice_(0)
	, emptyChatViewModelNotice_(0)
	, dummyHeader_(0)
	, userGender_(XMPP::VCard::UnknownGender)
	, historyReceived_(false)
	, messageCount_(0)
{
#ifndef NO_DELIVERY_CONFIRMATION
	connect(deliveryConfirmationManager_, SIGNAL(messageError(const QString&, const QString&)), SLOT(messageError(const QString&, const QString&)));
	connect(deliveryConfirmationManager_, SIGNAL(deliveryConfirmationUpdated(const QString&, YaChatViewModel::DeliveryConfirmationType)), SLOT(deliveryConfirmationUpdated(const QString&, YaChatViewModel::DeliveryConfirmationType)));
#endif
	connect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), SLOT(doRowsAboutToBeRemoved(const QModelIndex&, int, int)));

	connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateEmptyChatViewModelNotice()));
	connect(this, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateEmptyChatViewModelNotice()));

	addDummyHeader();

	composingEvent_ = new YaChatViewModelNotice(this, ContactComposing);
	composingEvent_->setPersistent(false);
	userIsOfflineNotice_ = new YaChatViewModelNotice(this, UserIsOfflineNotice);
	userIsOfflineNotice_->setPriority(5);
	userIsBlockedNotice_ = new YaChatViewModelNotice(this, UserIsBlockedNotice);
	userIsBlockedNotice_->setPriority(2);
	userNotInListNotice_ = new YaChatViewModelNotice(this, UserNotInListNotice);
	userNotInListNotice_->setPriority(3);
	notAuthorizedToSeeUserStatusNotice_ = new YaChatViewModelNotice(this, NotAuthorizedToSeeUserStatusNotice);
	notAuthorizedToSeeUserStatusNotice_->setPriority(4);
	accountIsOfflineNotice_ = new YaChatViewModelNotice(this, AccountIsOfflineNotice);
	accountIsOfflineNotice_->setPriority(1);
	accountIsDisabledNotice_ = new YaChatViewModelNotice(this, AccountIsDisabledNotice);
	accountIsDisabledNotice_->setPriority(0);
	statusTypeChangedNotice_ = new YaChatViewModelNotice(this, StatusTypeChangedNotice);
	statusTypeChangedNotice_->setPersistent(false);
	emptyChatViewModelNotice_ = new YaChatViewModelNotice(this, EmptyChatViewModelNotice);
	emptyChatViewModelNotice_->setPriority(6);

	notices_ << composingEvent_
	         << userIsOfflineNotice_
	         << userIsBlockedNotice_
	         << userNotInListNotice_
	         << notAuthorizedToSeeUserStatusNotice_
	         << accountIsOfflineNotice_
	         << accountIsDisabledNotice_
	         << statusTypeChangedNotice_
	         << emptyChatViewModelNotice_;

	qStableSort(notices_.begin(), notices_.end(), YaChatViewModel::yaChatViewModelNoticeLessThan);

	updateNoticesTimer_ = new QTimer(this);
	updateNoticesTimer_->setInterval(10);
	updateNoticesTimer_->setSingleShot(true);
	connect(updateNoticesTimer_, SIGNAL(timeout()), SLOT(updateNotices()));

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
}

YaChatViewModel::~YaChatViewModel()
{
	doClear();
}

bool YaChatViewModel::canRelyOnTimestamps()
{
	// TMP: ONLINE-1885
	return false;
}

void YaChatViewModel::addDummyHeader()
{
	if (dummyHeader_) {
		invisibleRootItem()->removeRow(dummyHeader_->row());
		dummyHeader_ = 0;
	}

	dummyHeader_ = new QStandardItem();
	dummyHeader_->setData(QVariant(DummyHeader), TypeRole);
	invisibleRootItem()->insertRow(0, dummyHeader_);
}

void YaChatViewModel::doClear()
{
	foreach(YaChatViewModelNotice* notice, notices_) {
		notice->setVisible(false);
	}

	Q_ASSERT(dummyHeader_);
	dummyHeader_ = 0;

	// we need to make sure YaChatView::rowsAboutToBeRemoved() are really called
	QStandardItem* parentItem = invisibleRootItem();
	while (parentItem->rowCount() > 0) {
		parentItem->removeRow(0);
	}

	messageCount_ = 0;

	clear();
	addDummyHeader();
	updateNotices();
}

YaChatViewModel::Type YaChatViewModel::childType(const QStandardItem* item) const
{
	return static_cast<YaChatViewModel::Type>(item->data(YaChatViewModel::TypeRole).toInt());
}

QStandardItem* YaChatViewModel::lastMessageOrDateHeader() const
{
	QStandardItem* parentItem = invisibleRootItem();
	if (parentItem->rowCount() > 0) {
		for (int row = parentItem->rowCount() - 1; row >= 0; --row) {
			QStandardItem* child = parentItem->child(row);
			switch (childType(child)) {
			case Message:
			case MoodChanged:
			case StatusTypeChangedNotice:
			case DateHeader:
				return child;
			default:
				;
			}
		}
	}
	return 0;
}

QStandardItem* YaChatViewModel::lastMessage() const
{
	QStandardItem* result = lastMessageOrDateHeader();
	if (!result || childType(result) == DateHeader) {
		result = 0;
	}
	return result;
}

bool YaChatViewModel::ensureDateHeader(const QDateTime& time, const XMPP::YaDateTime& yaTime)
{
	if (shouldAddDateHeader(time, yaTime)) {
		addDateHeader(time, yaTime);
		return true;
	}
	return false;
}

bool YaChatViewModel::shouldAddDateHeader(const QDateTime& time, const XMPP::YaDateTime& yaTime) const
{
	QStandardItem* item = lastMessageOrDateHeader();
	if (item) {
		if (childType(item) == DateHeader) {
			invisibleRootItem()->removeRow(item->row());
			return true;
		}

		QDateTime lastDate = item->data(DateTimeRole).toDateTime();
		return lastDate.daysTo(time) != 0;
	}

	return true;
}

bool YaChatViewModel::shouldMergeWith(const QDateTime& time, bool emote, bool local, YaChatViewModel::SpooledType spooled) const
{
	return shouldMergeWith(lastMessage(), time, emote, local, spooled);
}

bool YaChatViewModel::shouldMergeWith(QStandardItem* message, const QDateTime& time, bool emote, bool local, YaChatViewModel::SpooledType spooled) const
{
	if (emote)
		return false;

	if (message) {
		if (message->data(TypeRole) != Message &&
		    message->data(TypeRole) != MoodChanged)
		{
			return false;
		}

		if (message->data(IncomingRole) == !local &&
		    spooledType(message) == spooled)
		{
			QDateTime lastTime = message->data(DateTimeRole).toDateTime();
			if (lastTime.secsTo(time) < mergeInterval * 60)
				return true;
		}
	}

	return false;
}

void YaChatViewModel::updateMergeRole(int indexToUpdate, int indexPrev)
{
	QStandardItem* toUpdate = invisibleRootItem()->child(indexToUpdate);
	QStandardItem* prev = invisibleRootItem()->child(indexPrev);
	if (toUpdate && prev) {
		if (shouldMergeWith(prev,
		                    toUpdate->data(DateTimeRole).toDateTime(),
		                    toUpdate->data(EmoteRole).toBool(),
		                    !toUpdate->data(IncomingRole).toBool(),
		                    static_cast<YaChatViewModel::SpooledType>(toUpdate->data(SpooledRole).toInt())))
		{
			toUpdate->setData(QVariant(true), MergeRole);
		}
	}
}

#ifndef YAPSI
void YaChatViewModel::addMessage(bool incoming, const QString& msg)
{
	addMessage(false, QDateTime::currentDateTime(), !incoming, msg);
}

void YaChatViewModel::addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt)
{
	addMessageHelper(spooled, time, local, txt, true);
}

void YaChatViewModel::addMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt)
{
	addMessageHelper(spooled, time, local, txt, false);
}
#else
// void YaChatViewModel::addMessage(bool incoming, const QString& msg)
// {
// 	addMessage(YaChatViewModel::Spooled_None, QDateTime::currentDateTime(), !incoming, 0, QString(), XMPP::ReceiptNone, msg, yaTime);
// }

void YaChatViewModel::addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags)
{
	addMessageHelper(spooled, time, local, txt, true, spamFlag, id, messageReceipt, yaTime, yaFlags);
}

void YaChatViewModel::addMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, QString txt, const XMPP::YaDateTime& yaTime, int yaFlags)
{
	addMessageHelper(spooled, time, local, txt, false, spamFlag, id, messageReceipt, yaTime, yaFlags);
}
#endif

bool YaChatViewModel::sameDateTime(const XMPP::YaDateTime& ydt1, const QDateTime& dt1, const XMPP::YaDateTime& ydt2, const QDateTime& dt2) const
{
	if (!canRelyOnTimestamps()) {
		return true;
	}

	static int maxSecsDifference = 30 * 60; // 30 minutes
	bool result;
	if (!ydt1.isNull() && !ydt2.isNull()) {
		if (canRelyOnTimestamps()) {
			result = ydt1 == ydt2;
		}
		else {
			result = ydt1.secsTo(ydt2) < maxSecsDifference;
		}
	}
	else if (!ydt1.isNull() || !ydt2.isNull()) {
		if (!ydt1.isNull()) {
			result = ydt1.secsTo(dt2) < maxSecsDifference;
		}
		else {
			Q_ASSERT(!ydt2.isNull());
			result = dt1.secsTo(ydt2) < maxSecsDifference;
		}
	}
	else {
		if (canRelyOnTimestamps()) {
			result = dt1 == dt2;
		}
		else {
			result = dt1.secsTo(dt2) < maxSecsDifference;
		}
	}
	return result;
}

bool YaChatViewModel::isNext(QStandardItem* i1, QStandardItem* i2) const
{
	XMPP::YaDateTime ydt1 = i1->data(YaDateTimeRole).value<XMPP::YaDateTime>();
	XMPP::YaDateTime ydt2 = i2->data(YaDateTimeRole).value<XMPP::YaDateTime>();
	if (!ydt1.isNull() && !ydt2.isNull())
		return ydt1 > ydt2;

	return i1->data(DateTimeRole).toDateTime() > i2->data(DateTimeRole).toDateTime();
}

bool YaChatViewModel::isPrev(QStandardItem* i1, QStandardItem* i2) const
{
	XMPP::YaDateTime ydt1 = i1->data(YaDateTimeRole).value<XMPP::YaDateTime>();
	XMPP::YaDateTime ydt2 = i2->data(YaDateTimeRole).value<XMPP::YaDateTime>();
	if (!ydt1.isNull() && !ydt2.isNull())
		return ydt1 < ydt2;

	return i1->data(DateTimeRole).toDateTime() < i2->data(DateTimeRole).toDateTime();
}

bool YaChatViewModel::processSpooledItem(QStandardItem* i1, QStandardItem* i2) const
{
	if (sameItem(i1, i2)) {
		if (static_cast<YaMessageFlagType>(i1->data(YaFlagsRole).toInt()) == OutgoingMessage) {
			i1->setData(i2->data(YaDateTimeRole), YaDateTimeRole);
			i1->setData(NoFlags, YaFlagsRole);
		}

		delete i2;
		return true;
	}

	return false;
}

bool YaChatViewModel::sameItem(QStandardItem* i1, QStandardItem* i2) const
{
	bool sameDateTime = this->sameDateTime(i1->data(YaDateTimeRole).value<XMPP::YaDateTime>(),
	                                       i1->data(DateTimeRole).toDateTime(),
	                                       i2->data(YaDateTimeRole).value<XMPP::YaDateTime>(),
	                                       i2->data(DateTimeRole).toDateTime());

	Q_ASSERT(static_cast<YaMessageFlagType>(i2->data(YaFlagsRole).toInt()) != OutgoingMessage);
	if (static_cast<YaMessageFlagType>(i1->data(YaFlagsRole).toInt()) == OutgoingMessage) {
		sameDateTime = true;
	}

	return (sameDateTime &&
	        i1->data(Qt::DisplayRole) == i2->data(Qt::DisplayRole) &&
	        i1->data(TypeRole)        == i2->data(TypeRole)        &&
	        i1->data(IncomingRole)    == i2->data(IncomingRole)    &&
	        i1->data(EmoteRole)       == i2->data(EmoteRole));
}

#ifndef YAPSI
void YaChatViewModel::addMessageHelper(SpooledType spooled, const QDateTime& time, bool local, QString _txt, bool emote)
#else
void YaChatViewModel::addMessageHelper(SpooledType spooled, const QDateTime& time, bool local, QString _txt, bool emote, int spamFlag, QString id, XMPP::MessageReceipt messageReceipt, const XMPP::YaDateTime& yaTime, int yaFlags)
#endif
{
	if (_txt.isEmpty()) {
		qWarning("YaChatViewModel::addMessageHelper: dropping empty message");
		return;
	}

	if (!local) {
		if (!yaTime.isNull() && timeStamps_.contains(yaTime)) {
			// qWarning("YaChatViewModel::addMessageHelper(): duplicate incoming message: '%s' '%s' '%s'", qPrintable(yaTime.toYaIsoTime()), qPrintable(jid().full()), qPrintable(_txt));
			return;
		}
	}

	QString txt = _txt;
	QStandardItem* item = new QStandardItem(txt);
	item->setData(QVariant(Message), TypeRole);
	item->setData(QVariant(!Qt::mightBeRichText(txt)), MessagePlainTextRole);
	item->setData(QVariant(!local), IncomingRole);
	item->setData(QVariant(false), MergeRole);
	item->setData(QVariant(time), DateTimeRole);
	item->setData(QVariant(spooled), SpooledRole);
	item->setData(QVariant(emote), EmoteRole);
	item->setData(QVariant(id), IdRole);
	item->setData(QVariant(yaFlags), YaFlagsRole);
	{
		QVariant tmp;
		tmp.setValue(yaTime);
		item->setData(tmp, YaDateTimeRole);
	}

#ifdef YAPSI
	item->setData(QVariant(spamFlag), SpamRole);
#endif

	int newMessageRow = this->newMessageRow();
	if (spooled == Spooled_History || spooled == Spooled_OfflineStorage) {
		int dateHeaderRow = -1;
		newMessageRow = -1;

		if (!canRelyOnTimestamps()) {
			for (int row = 0; row < invisibleRootItem()->rowCount(); ++row) {
				QStandardItem* i = invisibleRootItem()->child(row);
				if (sameItem(i, item)) {
					delete item;
					return;
				}
			}
		}

		for (int row = 0; row < invisibleRootItem()->rowCount(); ++row) {
			QStandardItem* i = invisibleRootItem()->child(row);

			if (i->data(TypeRole) == Message ||
			    i->data(TypeRole) == MoodChanged ||
			    i->data(TypeRole) == DateHeader)
			{
				if (i->data(TypeRole) == DateHeader && !i->data(DateTimeRole).toDateTime().daysTo(time)) {
					dateHeaderRow = row;
				}

				if (processSpooledItem(i, item)) {
					return;
				}

				if (isNext(i, item)) {
					if (dateHeaderRow == row) {
						i->setData(QVariant(time), DateTimeRole);
						i->setData(item->data(YaDateTimeRole), YaDateTimeRole);
						newMessageRow = row + 1;
					}
					else {
						newMessageRow = row;
					}
					break;
				}
			}

			if (newMessageRow == -1) {
				newMessageRow = invisibleRootItem()->rowCount();
			}
		}

		newMessageRow = qMax(0, newMessageRow);

		if (dateHeaderRow == -1) {
			addDateHeader(newMessageRow, time, yaTime);
			newMessageRow++;
		}
	}
	else if (spooled == Spooled_Sync /*|| spooled == Spooled_None*/) {
		int dateHeaderRow = -1;
		bool foundSameDay = false;
		newMessageRow = -1;
		item->setData(QVariant(Spooled_None), SpooledRole);

		if (!canRelyOnTimestamps()) {
			for (int row = invisibleRootItem()->rowCount() - 1; row >= 0; --row) {
				QStandardItem* i = invisibleRootItem()->child(row);
				if (sameItem(i, item)) {
					delete item;
					return;
				}
			}
		}

		for (int row = invisibleRootItem()->rowCount() - 1; row >= 0; --row) {
			QStandardItem* i = invisibleRootItem()->child(row);

			if (i->data(TypeRole) == Message ||
			    i->data(TypeRole) == MoodChanged ||
			    i->data(TypeRole) == DateHeader)
			{
				if (i->data(TypeRole) == DateHeader && !i->data(DateTimeRole).toDateTime().daysTo(time)) {
					foundSameDay = true;
					newMessageRow = row + 1;
					i->setData(QVariant(time), DateTimeRole);
					i->setData(item->data(YaDateTimeRole), YaDateTimeRole);
					break;
				}

				if (processSpooledItem(i, item)) {
					return;
				}

				if (isPrev(i, item)) {
					if (i->data(DateTimeRole).toDateTime().daysTo(time) == 0) {
						foundSameDay = true;
						newMessageRow = row + 1;
					}
					else {
						dateHeaderRow = row + 1;
						newMessageRow = row + 1;
					}
					break;
				}
			}
		}

		newMessageRow = qMax(0, newMessageRow);

		if (dateHeaderRow != -1 || !foundSameDay) {
			addDateHeader(newMessageRow, time, yaTime);
			newMessageRow++;
		}
	}
	else {
		if (ensureDateHeader(time, yaTime)) {
			newMessageRow++;
			item->setData(QVariant(false), MergeRole);
		}
	}

	if (!id.isEmpty() && local) {
		ids_[id] = item;
	}

	DeliveryConfirmationType deliveryConfirmation = DeliveryConfirmation_Unknown;
#ifndef NO_DELIVERY_CONFIRMATION
	if (messageReceipt == XMPP::ReceiptRequest && local) {
		Q_ASSERT(!id.isEmpty());
		if (deliveryConfirmationManager_->shouldQueryWithoutTimeout(jid())) {
			deliveryConfirmation = DeliveryConfirmation_QueryingWithoutTimeout;
		}
		else {
			deliveryConfirmation = DeliveryConfirmation_Querying;
		}
		deliveryConfirmationManager_->start(id);
	}
#else
	Q_UNUSED(messageReceipt);
#endif
	item->setData(QVariant(deliveryConfirmation), DeliveryConfirmationRole);

	if (!yaTime.isNull()) {
		timeStamps_[yaTime] = true;
	}

	messageCount_++;
	invisibleRootItem()->insertRow(newMessageRow, item);
	updateMergeRole(newMessageRow, newMessageRow - 1);
	updateMergeRole(newMessageRow + 1, newMessageRow);
	updateMergeRole(100, -100);
	addDummyHeader();
}

void YaChatViewModel::doRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	for (int i = start; i <= end; ++i) {
		QString id = index(i, 0, parent).data(IdRole).toString();
		if (!id.isEmpty()) {
			ids_.remove(id);
		}

		XMPP::YaDateTime ydt = index(i, 0, parent).data(YaDateTimeRole).value<XMPP::YaDateTime>();
		if (!ydt.isNull()) {
			timeStamps_.remove(ydt);
		}
	}
}

void YaChatViewModel::addDateHeader(const QDateTime& time, const XMPP::YaDateTime& yaTime)
{
	addDateHeader(newMessageRow(), time, yaTime);
}

void YaChatViewModel::addDateHeader(int index, const QDateTime& time, const XMPP::YaDateTime& yaTime) const
{
	QStandardItem* item = new QStandardItem();
	item->setData(QVariant(DateHeader), TypeRole);
	item->setData(QVariant(time), DateTimeRole);
	{
		QVariant tmp;
		tmp.setValue(yaTime);
		item->setData(tmp, YaDateTimeRole);
	}

	invisibleRootItem()->insertRow(index, item);
}

int YaChatViewModel::newMessageRow() const
{
	int rowNum = 0;
	for (int row = invisibleRootItem()->rowCount() - 1; row >= 0; --row) {
		if (invisibleRootItem()->child(row)->data(YaChatViewModel::PersistentNoticeRole).toBool()) {
			continue;
		}
		else {
			rowNum = row + 1;
			break;
		}
	}

	// QStandardItem* message = lastMessage();
	// if (message)
	// 	rowNum = message->row() + 1;

	return rowNum;
}

bool YaChatViewModel::composingEventVisible() const
{
	return composingEvent_->isVisible();
}

void YaChatViewModel::setComposingEventVisible(bool visible)
{
	composingEvent_->setVisible(visible);
	composingEvent_->setShouldBeVisible(visible);
}

bool YaChatViewModel::userIsOfflineNoticeVisible() const
{
	return userIsOfflineNotice_->shouldBeVisible();
}

void YaChatViewModel::setUserIsOfflineNoticeVisible(bool visible)
{
	userIsOfflineNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::userIsBlockedNoticeVisible() const
{
	return userIsBlockedNotice_->shouldBeVisible();
}

void YaChatViewModel::setUserIsBlockedNoticeVisible(bool visible)
{
	userIsBlockedNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::userNotInListNoticeVisible() const
{
	return userNotInListNotice_->shouldBeVisible();
}

void YaChatViewModel::setUserNotInListNoticeVisible(bool visible)
{
	userNotInListNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::notAuthorizedToSeeUserStatusNoticeVisible() const
{
	return notAuthorizedToSeeUserStatusNotice_->shouldBeVisible();
}

void YaChatViewModel::setNotAuthorizedToSeeUserStatusNoticeVisible(bool visible)
{
	notAuthorizedToSeeUserStatusNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::accountIsOfflineNoticeVisible() const
{
	return accountIsOfflineNotice_->shouldBeVisible();
}

void YaChatViewModel::setAccountIsOfflineNoticeVisible(bool visible)
{
	accountIsOfflineNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::accountIsDisabledNoticeVisible() const
{
	return accountIsDisabledNotice_->shouldBeVisible();
}

void YaChatViewModel::setAccountIsDisabledNoticeVisible(bool visible)
{
	accountIsDisabledNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

void YaChatViewModel::setStatusTypeChangedNotice(XMPP::Status::Type status)
{
	if (status == XMPP::Status::Offline) {
		// persistent notice will be shown for this status type anyway
		// so there's no need for duplicate information
		statusTypeChangedNotice_->setVisible(false);
		statusTypeChangedNotice_->setShouldBeVisible(false);
		return;
	}

	ensureDateHeader(QDateTime::currentDateTime(), QDateTime::currentDateTime());

#ifndef NO_YACOMMON
	statusTypeChangedNotice_->setMessage(
	    tr(" is currently %1").arg(Ya::statusFullName(status, userGender()))
	);
#endif
	statusTypeChangedNotice_->setVisible(true);
	statusTypeChangedNotice_->setShouldBeVisible(true);
}

// FIXME: merge with addMessageHelper in order to get full Spooled_History & Spooled_Sync support
void YaChatViewModel::addMoodChange(SpooledType spooled, const QString& _mood, const QDateTime& timeStamp, const XMPP::YaDateTime& yaTime)
{
	Q_ASSERT(false);

#ifndef NO_TEXTUTIL
	QString mood = TextUtil::plain2richSimple(_mood.trimmed());
#else
	QString mood = _mood.trimmed();
#endif
	QString message;
	if (mood.isEmpty()) {
		message = tr("is not in a mood");
	}
	else {
		message = (userGender() != XMPP::VCard::Female) ?
		          tr("has changed mood to: %1", "contact is male").arg(mood) :
		          tr("has changed mood to: %1", "contact is female").arg(mood);
	}
	QStandardItem* item = new QStandardItem(message);
	item->setData(QVariant(MoodChanged), TypeRole);
	item->setData(QVariant(false), PersistentNoticeRole);
	item->setData(QVariant(true), MessagePlainTextRole);
	item->setData(QVariant(true), IncomingRole);
	item->setData(QVariant(false), MergeRole);
	item->setData(QVariant(true), EmoteRole);
	item->setData(QVariant(timeStamp), DateTimeRole);
	item->setData(QVariant(spooled), SpooledRole);
	{
		QVariant tmp;
		tmp.setValue(yaTime);
		item->setData(tmp, YaDateTimeRole);
	}

	ensureDateHeader(item->data(DateTimeRole).toDateTime(), item->data(YaDateTimeRole).value<XMPP::YaDateTime>());

	invisibleRootItem()->appendRow(item);
}

YaChatViewModel::Type YaChatViewModel::type(const QModelIndex& index)
{
	return static_cast<YaChatViewModel::Type>(index.data(YaChatViewModel::TypeRole).toInt());
}

YaChatViewModel::SpooledType YaChatViewModel::spooledType(const QStandardItem* item)
{
	return static_cast<YaChatViewModel::SpooledType>(item->data(YaChatViewModel::SpooledRole).toInt());
}

YaChatViewModel::SpooledType YaChatViewModel::spooledType(const QModelIndex& index)
{
	return static_cast<YaChatViewModel::SpooledType>(index.data(YaChatViewModel::SpooledRole).toInt());
}

void YaChatViewModel::updateNotices()
{
	bool foundVisible = false;
	foreach(YaChatViewModelNotice* notice, notices_) {
		if (!notice->persistent()) {
			notice->setVisible(notice->shouldBeVisible());
			continue;
		}

		notice->setVisible(!foundVisible && notice->shouldBeVisible());

		if (notice->isVisible()) {
			foundVisible = true;
		}
	}
}

void YaChatViewModel::messageError(const QString& id, const QString& error)
{
	if (ids_.contains(id)) {
		ids_[id]->setData(QVariant(error), ErrorRole);
	}
}

void YaChatViewModel::deliveryConfirmationUpdated(const QString& id, YaChatViewModel::DeliveryConfirmationType deliveryConfirmation)
{
	if (ids_.contains(id)) {
		QStandardItem* item = ids_[id];
		if (deliveryConfirmation == DeliveryConfirmation_Timeout) {
			YaChatViewModel::DeliveryConfirmationType dc = static_cast<YaChatViewModel::DeliveryConfirmationType>(item->data(YaChatViewModel::DeliveryConfirmationRole).toInt());
			if (dc == DeliveryConfirmation_QueryingWithoutTimeout) {
				return;
			}
		}

		item->setData(QVariant(deliveryConfirmation), DeliveryConfirmationRole);
		if (deliveryConfirmation != DeliveryConfirmation_Error && !item->data(ErrorRole).toString().isEmpty()) {
			item->setData(QVariant(), ErrorRole);
		}
	}
}

bool YaChatViewModel::yaChatViewModelNoticeLessThan(const YaChatViewModelNotice* n1, const YaChatViewModelNotice* n2)
{
	return n1->priority() < n2->priority();
}

const XMPP::Jid& YaChatViewModel::jid() const
{
	return jid_;
}

void YaChatViewModel::setJid(const XMPP::Jid& jid)
{
	jid_ = jid;
}

XMPP::VCard::Gender YaChatViewModel::userGender() const
{
	return userGender_;
}

void YaChatViewModel::setUserGender(XMPP::VCard::Gender gender)
{
	userGender_ = gender;
	foreach(YaChatViewModelNotice* notice, notices_) {
		notice->updateMessage();
	}
}

bool YaChatViewModel::historyReceived() const
{
	return historyReceived_;
}

void YaChatViewModel::setHistoryReceived(bool historyReceived)
{
	historyReceived_ = historyReceived;
	updateEmptyChatViewModelNotice();
}

void YaChatViewModel::updateEmptyChatViewModelNotice()
{
	bool shouldBeVisible = (messageCount_ == 0) && historyReceived_;
	if (emptyChatViewModelNotice_ && emptyChatViewModelNotice_->shouldBeVisible() != shouldBeVisible) {
		emptyChatViewModelNotice_->setShouldBeVisible(shouldBeVisible);
		updateNoticesTimer_->start();
	}
}

/**
 * Ensures that emptyChatViewModelNotice_ gets invalidated in order to make
 * sure the text we show in YaChatViewDelegate is up-to-date.
 */
void YaChatViewModel::optionChanged(const QString& option)
{
	if (option == "options.shortcuts.chat.send" &&
	    emptyChatViewModelNotice_ &&
	    emptyChatViewModelNotice_->isVisible())
	{
		emptyChatViewModelNotice_->setVisible(false);
		emptyChatViewModelNotice_->setVisible(true);
	}
}
