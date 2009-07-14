/*
 * yalastmailinformermodel.h
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

#ifndef YALASTMAILINFORMERMODEL_H
#define YALASTMAILINFORMERMODEL_H

#include <QStandardItemModel>

class PsiContactList;
class YaLastMail;

class YaLastMailInformerModel : public QStandardItemModel
{
	Q_OBJECT
public:
	YaLastMailInformerModel(QObject* parent);
	~YaLastMailInformerModel();

	void setContactList(PsiContactList* contactList);

	QString getIncomingMailUrl() const;
	QString getMailUrlFor(const QModelIndex&) const;

	enum {
		FromRole    = Qt::UserRole + 0,
		SubjectRole = Qt::UserRole + 1,
		DateRole    = Qt::UserRole + 2,
		MessageID   = Qt::UserRole + 3,
	};

private slots:
	void accountCountChanged();
	void lastMailNotify(const YaLastMail&);

private:
	PsiContactList* contactList_;
};

#endif
