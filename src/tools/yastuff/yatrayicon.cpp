/*
 * yatrayicon.cpp - custom tray icon
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

#include "yatrayicon.h"

#include <QCoreApplication>
#include <QPainter>
#include <QPointer>
#include <QTimer>

#include "globaleventqueue.h"
#include "psicon.h"
#include "psievent.h"
#include "psiiconset.h"
#include "yacommon.h"
#include "yavisualutil.h"

class YaTrayIconDestroyer : public QObject
{
	Q_OBJECT
public:
	YaTrayIconDestroyer(YaTrayIcon* trayIcon)
		: QObject(QCoreApplication::instance())
		, trayIcon_(trayIcon)
	{
		connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(terminating()));
	}

private slots:
	void terminating()
	{
		if (!trayIcon_.isNull())
			delete trayIcon_;
	}

private:
	QPointer<YaTrayIcon> trayIcon_;
};

YaTrayIcon::YaTrayIcon()
	: QSystemTrayIcon(QCoreApplication::instance())
	, psi_(0)
	, haveConnectingAccounts_(false)
{
	new YaTrayIconDestroyer(this);
	connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(tray_activated(QSystemTrayIcon::ActivationReason)));
	connect(GlobalEventQueue::instance(), SIGNAL(queueChanged()), SLOT(update()));

	icon_ = QIcon(":iconsets/system/default/logo_16.png");

	updateTimer_ = new QTimer(this);
	updateTimer_->setSingleShot(false);
	// we have to periodically update our icon so the icon would
	// not be considered inactive and hidden by Windows
	updateTimer_->setInterval(1000);
	connect(updateTimer_, SIGNAL(timeout()), SLOT(updateIcon()));
#ifdef Q_WS_WIN
	updateTimer_->start();
#endif

#ifndef Q_WS_MAC
	setToolTip(tr("Ya.Online"));
#endif
}

YaTrayIcon::~YaTrayIcon()
{
}

void YaTrayIcon::setPsi(PsiCon* psi)
{
	psi_ = psi;
	if (psi_) {
		connect(psi_, SIGNAL(accountActivityChanged()), SLOT(updateIcon()));
	}
}

YaTrayIcon* YaTrayIcon::instance(PsiCon* psi)
{
	if (!instance_) {
		instance_ = new YaTrayIcon();
	}
	instance_->setPsi(psi);
	return instance_;
}

void YaTrayIcon::tray_activated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
		emit clicked(); // Qt::LeftButton
	else if (reason == QSystemTrayIcon::MiddleClick)
		emit clicked(); // Qt::MidButton
	else if (reason == QSystemTrayIcon::DoubleClick)
		emit doubleClicked();
}

void YaTrayIcon::updateIcon()
{
#if 0
	XMPP::Status::Type status = haveConnectingAccounts_ ? XMPP::Status::Reconnecting :
	                            !psi_.isNull() ? psi_->currentStatusType() : XMPP::Status::Offline;

	QPixmap px = PsiIconset::instance()->status(status).pixmap();
	QPainter p(&px);

	if (!GlobalEventQueue::instance()->ids().isEmpty()) {
		QRect ICON_RECT = QRect(QPoint(0, 0), px.size());
		int RADIUS = qMin(ICON_RECT.width(), ICON_RECT.height()) / 2;
		QRect bounds = Ya::circle_bounds(QPointF(), RADIUS, 0);
		QRadialGradient gr(RADIUS, RADIUS, RADIUS, 3 * RADIUS / 5, 3 * RADIUS / 5);
		gr.setColorAt(0.0, QColor(255, 255, 255, 191));
		gr.setColorAt(0.2, QColor(255, 255, 127, 191));
		gr.setColorAt(0.9, QColor(150, 150, 200, 63));
		gr.setColorAt(0.95, QColor(0, 0, 0, 127));
		gr.setColorAt(1, QColor(0, 0, 0, 0));
		p.setRenderHint(QPainter::Antialiasing);
		p.setBrush(gr);
		p.setPen(Qt::NoPen);
		p.drawEllipse(0, 0, bounds.width(), bounds.height());

		// TODO: Use QMatrix to make sure all text fits on screen
		p.setPen(Qt::black);
		p.setFont(QFont("Monospace", 8, QFont::Bold));
		p.drawText(ICON_RECT, Qt::AlignCenter, QString::number(GlobalEventQueue::instance()->ids().size()));
	}
#endif
	setIcon(icon_);
}

void YaTrayIcon::update()
{
	updateIcon();
}

void YaTrayIcon::setHaveConnectingAccounts(bool haveConnectingAccounts)
{
	haveConnectingAccounts_ = haveConnectingAccounts;
	updateIcon();
}

YaTrayIcon* YaTrayIcon::instance_ = 0;

#include "yatrayicon.moc"
