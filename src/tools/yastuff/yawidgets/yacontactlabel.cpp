/*
 * yacontactlabel.cpp - contact's name widget
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

#include "yacontactlabel.h"

#include <QTimer>
#include <QPainter>
#include <QPen>
#include <QBrush>

#include "psiaccount.h"
#include "psitooltip.h"
#include "textutil.h"
#include "vcardfactory.h"
#include "yacommon.h"
#include "yavisualutil.h"

YaContactLabel::YaContactLabel(QWidget* parent)
	: YaLabel(parent)
	, profile_(0)
{
	setEditable(false);
}

QString YaContactLabel::text() const
{
	if (!profile_) {
		return Ya::ellipsis();
	}
	return Ya::contactName(profile_->name(), profile_->jid().full());
}

void YaContactLabel::setProfile(const YaProfile* profile)
{
	if (profile_ != profile) {
		profile_ = (YaProfile*)profile;
		if (profile_)
			connect(profile_, SIGNAL(nameChanged()), SLOT(update()));
		update();
	}
}

const YaProfile* YaContactLabel::profile() const
{
	return profile_;
}

void YaContactLabel::paintEvent(QPaintEvent* e)
{
	if (!backgroundColor_.isValid()) {
		YaLabel::paintEvent(e);
		return;
	}

	QPainter p(this);
	// p.setRenderHint(QPainter::TextAntialiasing, true);
	QFont f = p.font();
	// f.setStyleStrategy(QFont::PreferAntialias);
	p.setFont(f);
	p.setPen(textColor());
	p.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, text());
	if (fontMetrics().width(text()) > rect().width()) {
		Ya::VisualUtil::drawTextFadeOut(&p, rect(), backgroundColor_, 15);
	}
}

QColor YaContactLabel::backgroundColor() const
{
	return backgroundColor_;
}

void YaContactLabel::setBackgroundColor(QColor backgroundColor)
{
	backgroundColor_ = backgroundColor;
	update();
}
