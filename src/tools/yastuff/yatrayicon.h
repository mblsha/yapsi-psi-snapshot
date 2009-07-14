/*
 * yatrayicon.h - custom tray icon
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

#ifndef YATRAYICON_H
#define YATRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QPointer>
#include <QIcon>

class QMenu;
class QIcon;

class PsiCon;
class YaTrayIconDestroyer;
class QTimer;

class YaTrayIcon : public QSystemTrayIcon
{
	Q_OBJECT
private:
	YaTrayIcon();
	~YaTrayIcon();
	void setPsi(PsiCon* psi);

public:
	static YaTrayIcon* instance(PsiCon* psi);

	void setHaveConnectingAccounts(bool haveConnectingAccounts);

signals:
	void clicked();
	void doubleClicked();

public slots:
	void updateIcon();

private slots:
	void tray_activated(QSystemTrayIcon::ActivationReason);
	void update();

private:
	QPointer<PsiCon> psi_;
	static YaTrayIcon* instance_;
	bool haveConnectingAccounts_;
	QTimer* updateTimer_;
	QIcon icon_;

	friend class YaTrayIconDestroyer;
};

#endif
