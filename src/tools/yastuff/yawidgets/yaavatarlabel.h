/*
 * yaavatarlabel.h - avatar widget
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

#ifndef YAAVATARLABEL_H
#define YAAVATARLABEL_H

#include <QFrame>
#include <QPointer>

#include "xmpp_status.h"
#include "xmpp_vcard.h"

class YaProfile;
class QPaintEvent;

class YaAvatarLabel : public QFrame
{
	Q_OBJECT
public:
	YaAvatarLabel(QWidget* parent);
	virtual ~YaAvatarLabel();

	void setStatus(XMPP::Status::Type status);

	// reimplemented
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

protected slots:
	void avatarChanged();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void mousePressEvent(QMouseEvent*);

	virtual QIcon avatar() const = 0;
	virtual void paintAvatar(QPainter* painter);
	virtual XMPP::VCard::Gender gender() const;

private:
	XMPP::Status::Type status_;
};

#endif
