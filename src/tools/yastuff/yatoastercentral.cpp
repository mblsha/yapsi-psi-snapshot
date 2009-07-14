/*
 * yatoastercentral.cpp - global toaster-specific helpers
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

#include "yatoastercentral.h"

#include <QTimer>

#include "psioptions.h"

static const QString disableNotificationsForContactsOptionPath = "options.ya.popups.moods.disable-notifications-for-contacts";

YaToasterCentral::YaToasterCentral(QObject* parent)
	: QObject(parent)
	, cleanupTimer_(0)
{
	cleanupTimer_ = new QTimer(this);
	connect(cleanupTimer_, SIGNAL(timeout()), SLOT(cleanup()));
	cleanupTimer_->setSingleShot(true);
	cleanupTimer_->setInterval(10000);

	loadMoodNotificationsDisabled();
}

YaToasterCentral::~YaToasterCentral()
{
}

void YaToasterCentral::loadMoodNotificationsDisabled()
{
	moodNotificationsDisabled_.clear();

	foreach(QVariant item, PsiOptions::instance()->getOption(disableNotificationsForContactsOptionPath).toList()) {
		XMPP::Jid jid(item.toString());
		moodNotificationsDisabled_[jid.bare()] = true;
	}
}

void YaToasterCentral::saveMoodNotificationsDisabled()
{
	QList<QVariant> list;
	QHashIterator<QString, bool> it(moodNotificationsDisabled_);
	while (it.hasNext()) {
		it.next();
		if (it.value() == true) {
			list << QVariant(it.key());
		}
	}
	PsiOptions::instance()->setOption(disableNotificationsForContactsOptionPath, list);
}

bool YaToasterCentral::moodNotificationsDisabled(const XMPP::Jid& jid) const
{
	return moodNotificationsDisabled_.contains(jid.bare()) &&
	       moodNotificationsDisabled_[jid.bare()] == true;
}

void YaToasterCentral::setMoodNotificationsDisabled(const XMPP::Jid& jid, bool disabled)
{
	moodNotificationsDisabled_[jid.bare()] = disabled;
	saveMoodNotificationsDisabled();
}

void YaToasterCentral::cleanup()
{
	toasters_.clear();
}

// TODO: think of a better name. It should imply a question as in 'show toaster?'
bool YaToasterCentral::showToaster(YaToasterCentral::Type type, const XMPP::Jid& jid, const QString& text)
{
	bool result = true;
	if (toasters_.contains(jidToString(jid))) {
		foreach(Toaster t, toasters_[jidToString(jid)]) {
			if (t.type == type && t.text == text) {
				result = false;
				break;
			}
		}
	}

	if (result) {
		if (toasters_.contains(jidToString(jid))) {
			toasters_[jidToString(jid)] << Toaster(type, text);
		}
		else {
			QList<Toaster> list;
			list << Toaster(type, text);
			toasters_[jidToString(jid)] = list;
		}
	}

	cleanupTimer_->start();
	return result;
}

QString YaToasterCentral::jidToString(const XMPP::Jid& jid) const
{
	return jid.full();
}
