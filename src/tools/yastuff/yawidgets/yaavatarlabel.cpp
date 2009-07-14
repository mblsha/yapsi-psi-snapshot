/*
 * yaavatarlabel.cpp - avatar widget
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

#include "yaavatarlabel.h"

#include <QIcon>
#include <QPainter>
#include <QMouseEvent>

#include "yaprofile.h"
#include "psitooltip.h"
#include "yavisualutil.h"

YaAvatarLabel::YaAvatarLabel(QWidget* parent)
	: QFrame(parent)
	, status_(XMPP::Status::Online)
{
	setFrameShape(QFrame::NoFrame);
}

YaAvatarLabel::~YaAvatarLabel()
{}

QSize YaAvatarLabel::minimumSizeHint() const
{
	return minimumSize();
}

QSize YaAvatarLabel::sizeHint() const
{
	if (avatar().isNull())
		return maximumSize();

	return avatar().actualSize(maximumSize());
}

void YaAvatarLabel::avatarChanged()
{
	update();
}

void YaAvatarLabel::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	paintAvatar(&painter);

	drawFrame(&painter);
}

XMPP::VCard::Gender YaAvatarLabel::gender() const
{
	return XMPP::VCard::UnknownGender;
}

void YaAvatarLabel::paintAvatar(QPainter* painter)
{
	Ya::VisualUtil::drawAvatar(painter, rect(), status_, gender(), avatar());
}

void YaAvatarLabel::setStatus(XMPP::Status::Type status)
{
	status_ = status;
	avatarChanged();
}

void YaAvatarLabel::mousePressEvent(QMouseEvent* e)
{
	e->accept();
}
