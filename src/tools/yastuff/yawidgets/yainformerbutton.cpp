/*
 * yainformerbutton.cpp
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

#include "yainformerbutton.h"

#include <QPainter>
#include <QFont>
#include <QSizeGrip>

#include "psitooltip.h"
#include "xmpp_jid.h"
#include "xmpp_vcard.h"
#include "vcardfactory.h"
#include "desktoputil.h"
#include "yainformer.h"
#include "yavisualutil.h"

static const int buttonSize = 22;
static const int margin = 2;
static const float INFORMER_BUTTON_CHECKED_OPACITY = 0.75;

YaInformerButton::YaInformerButton(YaInformer* informer)
	: QAbstractButton()
	, informer_(informer)
{
	hide();

	setFocusPolicy(Qt::NoFocus);
	setCheckable(true);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	PsiToolTip::install(this);
	setAttribute(Qt::WA_Hover, true);
	setCursor(Qt::PointingHandCursor);

	// QFont font("Tahoma", 6);
	// font.setBold(true);
	// setFont(font);

	connect(this, SIGNAL(clicked()), SLOT(clicked()));

	dataChanged();
}

YaInformerButton::~YaInformerButton()
{
}

bool YaInformerButton::isPersistent() const
{
	return informer_->isPersistent();
}

void YaInformerButton::dataChanged()
{
	// QString statusText = index_.data(ContactListModel::StatusTextRole).toString();
	// QString toolTip = QString("<div style='white-space:pre'><b>%1</b></div>")
	//                   .arg(index_.data(Qt::DisplayRole).toString());
	// if (!statusText.isEmpty())
	// 	toolTip += QString("<br>") + statusText;
	// // setToolTip(toolTip);

	update();
	updateGeometry();
}

QSize YaInformerButton::sizeHint() const
{
	return minimumSizeHint();
}

QSize YaInformerButton::minimumSizeHint() const
{
	QSize sh(buttonSize, buttonSize);
	// int valueWidth = fontMetrics().width(value());
	// sh.setWidth(sh.width() + margin + valueWidth);
	return sh;
}

QIcon YaInformerButton::informerIcon() const
{
	QIcon result = informer_->icon();
	if (!result.isNull())
		return result;

	return Ya::VisualUtil::noAvatarPixmap(XMPP::VCard::UnknownGender);
}

bool YaInformerButton::informerOnline() const
{
	return informer_->isOnline();
}

QString YaInformerButton::value() const
{
	return informer_->value();
}

void YaInformerButton::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	p.save();

	QRect iconRect(0, 0, buttonSize, buttonSize);

	if (isChecked()) {
		p.fillRect(rect(), informer_->backgroundBrush());
		p.setOpacity(INFORMER_BUTTON_CHECKED_OPACITY);
		iconRect.translate(0, -1);
	}

	informerIcon().paint(&p, iconRect, Qt::AlignCenter,
	                     testAttribute(Qt::WA_UnderMouse) && isEnabled() ?
	                     QIcon::Selected :
	                     (informerOnline() ? QIcon::Normal : QIcon::Disabled),
	                     isDown() ? QIcon::On : QIcon::Off);

	p.restore();

	if (!value().isEmpty()) {
		// QRect textRect(rect());
		// textRect.setLeft(iconRect.right() + margin);
		QRect textRect(iconRect);
		textRect.translate(0, fontMetrics().descent());
		p.drawText(textRect, Qt::AlignBottom | Qt::AlignRight, fontMetrics().elidedText(value(), Qt::ElideRight, textRect.width()));
	}
	// p.drawRect(rect());
}

QString YaInformerButton::jid() const
{
	return informer_->jid();
}

void YaInformerButton::clicked()
{
	// XMPP::Jid j(jid());
	// if (!j.isValid())
	// 	return;
	//
	// const XMPP::VCard* vcard = VCardFactory::instance()->vcard(j);
	// if (!vcard)
	// 	return;
	//
	// DesktopUtil::openUrl(vcard->url());
}
