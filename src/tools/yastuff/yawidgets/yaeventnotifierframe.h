/*
 * yaeventnotifierframe.h
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

#ifndef YAEVENTNOTIFIERFRAME_H
#define YAEVENTNOTIFIERFRAME_H

#include <QFrame>
#include <QPushButton>
#include <QPointer>

#include "overlaywidget.h"
#include "xmpp_jid.h"

class YaEventNotifierFrame;
class PsiEvent;
class YaProfile;

#include "ui_yaeventnotifier.h"

// TODO: maybe replace this with YaCloseButton
class YaEventNotifierFrameExtra : public QPushButton
{
	Q_OBJECT
public:
	YaEventNotifierFrameExtra(QWidget* parent)
		: QPushButton(parent)
	{
		setObjectName("eventNotifierCloseButton");
		setFocusPolicy(Qt::NoFocus);
	}

	// reimplemented
	QSize sizeHint() const
	{
		return QSize(11, 11);
	}
};

class YaEventNotifierFrame : public OverlayWidget<QFrame, YaEventNotifierFrameExtra>
{
	Q_OBJECT
public:
	YaEventNotifierFrame(QWidget* parent);

	PsiEvent* currentEvent() const;
	XMPP::Jid currentJid() const;

	QBrush backgroundBrush() const;

	int id() const;

	void setEventCounter(int skipped, int total);
	void setEvent(int id, PsiEvent* event);
	void setTosterMode(bool enable);

signals:
	void skip();
	void read();
	void closeClicked();
	void clicked();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void mousePressEvent(QMouseEvent*);
	bool eventFilter(QObject* obj, QEvent* e);

private slots:
	void decline();
	void accept();

public slots:
	void optionChanged(const QString&);

private:
	Ui::EventNotifierFrame ui_;
	QPointer<YaProfile> profile_;
	QBrush backgroundBrush_;
	bool tosterMode_;
	int id_;

	// reimplemented
	QRect extraGeometry() const;

	enum Background {
		Notifier = 0,
		Informer
	};

	void setBackground(Background background = YaEventNotifierFrame::Informer);
	void updateProfile(PsiEvent* event);
	void updateButtons(PsiEvent* event);
};

#endif
