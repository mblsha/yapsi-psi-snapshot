/*
 * yaeventnotifier.cpp - event notifier widget
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

#include "yaeventnotifier.h"

#include <QVBoxLayout>

#include "psicon.h"
#include "psievent.h"
#include "globaleventqueue.h"
#include "psiaccount.h"
#include "yacommon.h"
#include "yaeventnotifierframe.h"
#include "yavisualutil.h"
#include "yainformerbutton.h"

//----------------------------------------------------------------------------
// YaEventNotifierInformer
//----------------------------------------------------------------------------

YaEventNotifierInformer::YaEventNotifierInformer(QObject* parent)
	: YaInformer(QModelIndex(), parent)
{
	setActivateShortcut("appwide.activate-event-notifier");

	notifier_ = new YaEventNotifier(0);
	connect(notifier_, SIGNAL(visibilityChanged()), SLOT(notifierVisibilityChanged()));
	notifierVisibilityChanged();
}

YaEventNotifierInformer::~YaEventNotifierInformer()
{
	if (!notifier_.isNull())
		delete notifier_;
}

YaEventNotifier* YaEventNotifierInformer::notifier() const
{
	return notifier_;
}

QWidget* YaEventNotifierInformer::widget()
{
	return notifier_;
}

bool YaEventNotifierInformer::isPersistent() const
{
	return true;
}

QString YaEventNotifierInformer::jid() const
{
	return "ru.ya.yapsi.event-notifier";
}

QIcon YaEventNotifierInformer::icon() const
{
	return Ya::VisualUtil::rosterStatusPixmap(XMPP::Status::Online);
}

bool YaEventNotifierInformer::isOnline() const
{
	return notifier_->shouldBeVisible();
}

QString YaEventNotifierInformer::value() const
{
	int number = notifier_->numUnreadEvents();
	if (!number)
		return QString();
	return QString::number(number);
}

void YaEventNotifierInformer::notifierVisibilityChanged()
{
	bool enabled = notifier_->shouldBeVisible();
	button()->setEnabled(enabled);
	if (enabled)
		emit activated();
	else
		emit deactivated();
	updateButton();
}

int YaEventNotifierInformer::desiredHeight() const
{
	return 105;
}

//----------------------------------------------------------------------------
// YaEventNotifier
//----------------------------------------------------------------------------

YaEventNotifier::YaEventNotifier(QWidget* parent)
	: QWidget(parent)
	, psi_(0)
{
	QVBoxLayout* vbox = new QVBoxLayout(this);
	vbox->setMargin(0);
	eventNotifierFrame_ = new YaEventNotifierFrame(this);
	vbox->addWidget(eventNotifierFrame_);

	connect(eventNotifierFrame_, SIGNAL(skip()), SLOT(skip()));
	connect(eventNotifierFrame_, SIGNAL(read()), SLOT(read()));
	connect(eventNotifierFrame_, SIGNAL(closeClicked()), SLOT(skipAll()));

	connect(GlobalEventQueue::instance(), SIGNAL(queueChanged()), SLOT(update()), Qt::QueuedConnection);
	update();
}

YaEventNotifier::~YaEventNotifier()
{}

void YaEventNotifier::setController(PsiCon* psi)
{
	psi_ = psi;
}

void YaEventNotifier::showEvent(QShowEvent* event)
{
	if (shouldBeVisible())
		QWidget::showEvent(event);
}

int YaEventNotifier::numUnreadEvents() const
{
	return qMax(0, GlobalEventQueue::instance()->ids().count() - skipped_ids_.count());
}

void YaEventNotifier::update()
{
	updateSkippedList();

	eventNotifierFrame_->setEvent(currentEventId(), currentEvent());
	eventNotifierFrame_->setEventCounter(skipped_ids_.count() + 1, GlobalEventQueue::instance()->ids().count());

	if (shouldBeVisible())
		show();
	else
		hide();

	emit visibilityChanged();
}

void YaEventNotifier::skip()
{
	QList<int> ids = unskippedIds();
	Q_ASSERT(!ids.isEmpty());
	// skipped_ids_.append(ids.first());

	PsiEvent* event = GlobalEventQueue::instance()->peek(ids.first());
	Q_ASSERT(event);
	event->account()->eventQueue()->dequeue(event);
	delete event;

	update();
}

void YaEventNotifier::skipAll()
{
	// QList<int> ids = unskippedIds();
	// Q_ASSERT(!ids.isEmpty());
	// skipped_ids_ += ids;

	while (!GlobalEventQueue::instance()->ids().isEmpty()) {
		PsiEvent* event = GlobalEventQueue::instance()->peek(GlobalEventQueue::instance()->ids().first());
		Q_ASSERT(event);
		event->account()->eventQueue()->dequeue(event);
		delete event;
	}

	update();
}

void YaEventNotifier::read()
{
	PsiEvent* event = currentEvent();
	if (!event)
		return;
	Q_ASSERT(psi_);
	psi_->processEvent(event, UserAction);
}

int YaEventNotifier::currentEventId() const
{
	QList<int> ids = unskippedIds();
	if (ids.isEmpty())
		return -1;
	return ids.first();
}

PsiEvent* YaEventNotifier::currentEvent() const
{
	if (currentEventId() < 0)
		return 0;
	return GlobalEventQueue::instance()->peek(currentEventId());
}

void YaEventNotifier::updateSkippedList()
{
	QList<int> ids = GlobalEventQueue::instance()->ids();
	QList<int> new_skipped;
	foreach(int id, skipped_ids_)
		if (ids.contains(id))
			new_skipped.append(id);
	skipped_ids_ = new_skipped;
}

QList<int> YaEventNotifier::unskippedIds() const
{
	QList<int> ids = GlobalEventQueue::instance()->ids();
	foreach(int id, skipped_ids_)
		ids.removeAll(id);
	return ids;
}

bool YaEventNotifier::shouldBeVisible() const
{
#ifdef YAPSI_ACTIVEX_SERVER
	return false;
#endif
	return !unskippedIds().isEmpty();
}

