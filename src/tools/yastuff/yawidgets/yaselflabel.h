/*
 * yaselflabel.h
 * Copyright (C) 2008  Yandex LLC (Alexei Matiouchkine)
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

#ifndef YASELFLABEL_H
#define YASELFLABEL_H

#include "m11editablelabel.h"

#include <QList>

#include "yalabel.h"
#include "yaprofile.h"

class PsiContactList;

using namespace Ya;

class YaSelfLabel : virtual public YaLabel
{
	Q_OBJECT
public:
	YaSelfLabel(QWidget* parent);

public:
	// reimplemented
	virtual QString text() const;

public:
	virtual void setContactList(const PsiContactList* contactList);
	virtual const PsiContactList* contactList() const;
	const QList<PsiAccount*>& accounts() const;

protected slots:
	// void nameChanged(const QString &);

private:
	PsiContactList* contactList_;
};

#endif
