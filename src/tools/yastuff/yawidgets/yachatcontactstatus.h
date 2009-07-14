/*
 * yachatcontactstatus.h - contact status widget
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

#ifndef YACHATCONTACTSTATUS_H
#define YACHATCONTACTSTATUS_H

#include <QLabel>
#include "overlaywidget.h"

#include "xmpp_status.h"
#include "xmpp_vcard.h"

class YaChatContactStatusExtra : public QWidget
{
	Q_OBJECT
public:
	YaChatContactStatusExtra(QWidget *parent);

	XMPP::Status::Type status() const;
	bool shouldBeVisible() const;

	void setStatus(XMPP::Status::Type status, const QString& msg, XMPP::VCard::Gender gender);

	// reimplemented
	QSize sizeHint() const;

signals:
	void invalidateExtraPosition();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent* e);

	bool showStatusDescription(XMPP::Status::Type status) const;
	QString statusText(XMPP::Status::Type status) const;

private:
	QString labelText_;
	XMPP::Status::Type status_;
	XMPP::VCard::Gender gender_;
	QLabel* icon_;
	QLabel* label_;
};

class YaChatContactStatus : public OverlayWidget<QFrame, YaChatContactStatusExtra>
{
	Q_OBJECT
public:
	YaChatContactStatus(QWidget *parent);

	YaChatContactStatusExtra* realWidget() const { return extra(); }

	// reimplemented
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

private:
	// reimplemented
	QRect extraGeometry() const;
	bool extraShouldBeVisible() const;
};

#endif
