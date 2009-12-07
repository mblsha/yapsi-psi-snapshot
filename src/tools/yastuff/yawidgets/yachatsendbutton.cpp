/*
 * yachatsendbutton.cpp - custom send button for chat dialog
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

#include "yachatsendbutton.h"

#include <QPainter>
#include <QAction>

#include "tabbablewidget.h"
#include "yawindowtheme.h"
#include "shortcutmanager.h"
#include "psioptions.h"

typedef enum YaChatSendButton_Background {
	YCSB_Disabled = 0,
	YCSB_Idle,
	YCSB_Hover,
	YCSB_Pressed
};

static const int LEFT_MARGIN = 13;
static const int RIGHT_MARGIN = 9;

static QPixmap buttonBackground(YaChatSendButton_Background type)
{
	QPixmap pix[4];

	if (type == YCSB_Idle) {
		if (pix[0].isNull()) {
			pix[0] = QPixmap(":images/chat/send_button/send.png");
		}
		Q_ASSERT(!pix[0].isNull());
		return pix[0];
	}

	if (type == YCSB_Hover) {
		if (pix[1].isNull()) {
			pix[1] = QPixmap(":images/chat/send_button/send_hover.png");
		}
		Q_ASSERT(!pix[1].isNull());
		return pix[1];
	}

	if (type == YCSB_Pressed) {
		if (pix[2].isNull()) {
			pix[2] = QPixmap(":images/chat/send_button/send_pressed.png");
		}
		Q_ASSERT(!pix[2].isNull());
		return pix[2];
	}

	if (type == YCSB_Disabled) {
		if (pix[3].isNull()) {
			pix[3] = QPixmap(":images/chat/send_button/send_disabled.png");
		}
		Q_ASSERT(!pix[3].isNull());
		return pix[3];
	}

	Q_ASSERT(false);
	return QPixmap();
}

//----------------------------------------------------------------------------
// YaChatSendButton
//----------------------------------------------------------------------------

YaChatSendButton::YaChatSendButton(QWidget* parent)
	: QToolButton(parent)
{
	setAttribute(Qt::WA_Hover, true);

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));

	setCursor(Qt::PointingHandCursor);
}

YaChatSendButton::~YaChatSendButton()
{
}

QString YaChatSendButton::buttonText() const
{
	return tr("Send");
}

void YaChatSendButton::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	YaChatSendButton_Background type = YCSB_Disabled;
	if (underMouse() && isEnabled())
		type = YCSB_Hover;
	if (isDown())
		type = YCSB_Pressed;

	QPixmap pix = buttonBackground(type);
	p.drawPixmap(0, 0, pix);

	QImage img = pix.toImage();
	QRect tileRect(rect());
	tileRect.setLeft(pix.width());
	p.drawTiledPixmap(tileRect, QPixmap::fromImage( img.copy(img.width()-1, 0, 1, img.height()) ));

	p.setPen(type != YCSB_Disabled ? Qt::white : QColor("#BEBEBE"));
	int topMargin = isDown() ? 1 : 0;
	QRect textRect(rect().adjusted(LEFT_MARGIN, topMargin, 0, topMargin));
	p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, buttonText());
}

void YaChatSendButton::setAction(QAction* action)
{
	setDefaultAction(action);
	connect(action, SIGNAL(changed()), SLOT(actionChanged()));
	actionChanged();
}

void YaChatSendButton::actionChanged()
{
	if (!defaultAction())
		return;

	setToolTip(tr("%1 (%2)")
	           .arg(defaultAction()->text())
	           .arg(ShortcutManager::instance()->shortcut("chat.send").toString(QKeySequence::NativeText)));
}

void YaChatSendButton::optionChanged(const QString& option)
{
	if (option == "options.shortcuts.chat.send") {
		actionChanged();
	}
}

void YaChatSendButton::updatePosition()
{
}

QSize YaChatSendButton::minimumSizeHint() const
{
	QSize sh = buttonBackground(YCSB_Idle).size();
	sh.setWidth(LEFT_MARGIN + fontMetrics().width(buttonText()) + RIGHT_MARGIN);
	return sh;
}

QSize YaChatSendButton::sizeHint() const
{
	return minimumSizeHint();
}
