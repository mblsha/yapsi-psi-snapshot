/*
 * yatoastercentral.h - global toaster-specific helpers
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

#ifndef YATOASTERCENTRAL_H
#define YATOASTERCENTRAL_H

#include <QObject>

#include <QHash>
#include <QList>

#include "xmpp_jid.h"

class QTimer;

class YaToasterCentral : public QObject
{
	Q_OBJECT
public:
	YaToasterCentral(QObject* parent);
	~YaToasterCentral();

	enum Type {
		MoodChange
	};

	bool showToaster(Type type, const XMPP::Jid& jid, const QString& text);

	bool moodNotificationsDisabled(const XMPP::Jid& jid) const;
	void setMoodNotificationsDisabled(const XMPP::Jid& jid, bool disabled);

private slots:
	void cleanup();

public:
	struct Toaster {
		Toaster(Type _type, QString _text)
			: type(_type), text(_text)
		{}

		Type type;
		QString text;
	};

	QTimer* cleanupTimer_;
	QHash<QString, QList<Toaster> > toasters_;
	QHash<QString, bool> moodNotificationsDisabled_;

	QString jidToString(const XMPP::Jid& jid) const;
	void loadMoodNotificationsDisabled();
	void saveMoodNotificationsDisabled();
};

#endif
