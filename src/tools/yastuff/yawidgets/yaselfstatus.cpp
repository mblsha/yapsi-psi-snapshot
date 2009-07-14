/*
 * yaselfstatus.cpp
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

#include "yaselfstatus.h"

#include <QLayout>

#include "yacommon.h"
#include "psioptions.h"

YaSelfStatus::YaSelfStatus(QWidget* parent)
	: YaRosterComboBox(parent)
{
	QList<XMPP::Status::Type> statuses;
	if (PsiOptions::instance()->getOption("options.ui.menu.status.chat").toBool())
		statuses << XMPP::Status::FFC;
	statuses << XMPP::Status::Online;
	statuses << XMPP::Status::Away;
	if (PsiOptions::instance()->getOption("options.ui.menu.status.xa").toBool())
		statuses << XMPP::Status::XA;
	statuses << XMPP::Status::DND;
	statuses << XMPP::Status::Offline;

	// FIXME: use gender-specific versions
	foreach(XMPP::Status::Type status, statuses)
		addItem(Ya::statusFullName(status), QVariant(status));

	connect(this, SIGNAL(currentIndexChanged(int)), SLOT(statusChanged()));
}

YaSelfStatus::~YaSelfStatus()
{
}

XMPP::Status::Type YaSelfStatus::status() const
{
	return static_cast<XMPP::Status::Type>(itemData(currentIndex()).toInt());
}

void YaSelfStatus::setStatus(XMPP::Status::Type type)
{
	if (status() == type)
		return;

	int index = findData(QVariant(type));
	if (index == -1) {
		qWarning("YaSelfStatus::setStatus(%d) -- status not found in combobox" , type);
		// Q_ASSERT(false);
		return;
	}
	blockSignals(true);
	setCurrentIndex(index);
	updateColor();
	blockSignals(false);
}

void YaSelfStatus::updateColor()
{
	QPalette pal = palette();
	pal.setColor(QPalette::Text, Ya::statusColor(status()));
	setPalette(pal);
}

void YaSelfStatus::statusChanged()
{
	updateColor();
	emit statusChanged(status());

	QLayout* layout = parentWidget()->layout();
	if (layout)
		layout->invalidate();
}
