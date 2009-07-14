/*
 * yacontactavatarlabel.h - contact's avatar widget
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

#ifndef YACONTACTAVATARLABEL_H
#define YACONTACTAVATARLABEL_H

#include <QFrame>

#include "yaavatarlabel.h"

class YaProfile;
class QPaintEvent;

class YaContactAvatarLabel : public YaAvatarLabel
{
	Q_OBJECT
public:
	YaContactAvatarLabel(QWidget* parent);
	virtual ~YaContactAvatarLabel();

public slots:
	virtual void setProfile(YaProfile*);

protected:
	virtual QIcon avatar() const;
	YaProfile* profile() const;

	// reimplemented
	virtual XMPP::VCard::Gender gender() const;

private:
	QPointer<YaProfile> profile_;
};

#endif
