/*
 * yaloginpage.h
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

#ifndef YALOGINPAGE_H
#define YALOGINPAGE_H

#include <QWidget>

#include "ui_yaloginpage.h"

class PsiContactList;
class PsiAccount;

class YaLoginPage : public QWidget
{
	Q_OBJECT
public:
	YaLoginPage();
	~YaLoginPage();

	bool shouldBeVisible();

	void setContactList(PsiContactList* contactList);

signals:
	void updateVisibility();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

private slots:
	void connectionError(const QString&);
	void accountCountChanged();
	void updatedActivity();

	void updateFocus();
	void textChanged();

	void signIn();
	void stop();
	void registerAccount();

private:
	PsiContactList* contactList_;
	bool shouldBeVisible_;
	bool disabled_;
	Ui::YaLoginPage ui_;
	QList<PsiAccount*> knownAccounts_;

	QString login() const;
	QString password() const;

	PsiAccount* getLoginAccount() const;
	PsiAccount* getAccount() const;
	void setShouldBeVisible(bool shouldBeVisible);
	bool signInEnabled() const;
};

#endif
