/*
 * yatabbednotifier.cpp
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

#include "yatabbednotifier.h"

#include <QEvent>
#include <QPainter>

YaTabbedNotifier::YaTabbedNotifier(QWidget* parent)
	: QFrame(parent)
{
}

YaTabbedNotifier::~YaTabbedNotifier()
{
}

void YaTabbedNotifier::setEventNotifier(QWidget* eventNotifier)
{
	eventNotifier_ = eventNotifier;
	if (!eventNotifier_.isNull())
		eventNotifier_->installEventFilter(this);
	update();
}

bool YaTabbedNotifier::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == eventNotifier_)
		if (event->type() == QEvent::Show ||
		    event->type() == QEvent::Hide)
			update();

	return QFrame::eventFilter(obj, event);
}

void YaTabbedNotifier::paintEvent(QPaintEvent*)
{
#ifdef GEEK_TABS
	QPainter painter(this);
	QPixmap events(":/images/friends_tabs.png");
	int offset = (!eventNotifier_.isNull() && eventNotifier_->isVisible()) ? 0 : 28;
	QRect eventsRect(0, offset, events.width(), events.height() - offset);
	QRect eventsRectDest(width() - events.width(), 0, events.width(), events.height());
	painter.drawPixmap(eventsRectDest, events, eventsRect);

	QPixmap tabs(":/images/contacts_tabs.png");
	QRect tabsRect(0, 0, tabs.width(), tabs.height());
	QRect tabsRectDest(width() - tabs.width(), height() - tabs.height() - 32, tabs.width(), tabs.height());
	painter.drawPixmap(tabsRectDest, tabs, tabsRect);
#endif
}
