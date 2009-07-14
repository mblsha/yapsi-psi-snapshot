/*
 * yawindowextrabutton.cpp
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

#include "yawindowextrabutton.h"

#include <QPainter>

#include "yawindow.h"
#include "yawindowtheme.h"
#include "yarostertiplabel.h"
#include "pixmaputil.h"
#include "psitooltip.h"


//----------------------------------------------------------------------------
// YaWindowExtraButtonBase
//----------------------------------------------------------------------------

YaWindowExtraButtonBase::YaWindowExtraButtonBase(QWidget* parent)
	: QToolButton(parent)
{
	setCursor(Qt::PointingHandCursor);
	setAttribute(Qt::WA_Hover, true);

	setFocusPolicy(Qt::NoFocus);

	PsiToolTip::install(this);

	connect(this, SIGNAL(toggled(bool)), SLOT(buttonToggled()));
	connect(this, SIGNAL(pressed()), SLOT(buttonToggled()));
	connect(this, SIGNAL(released()), SLOT(buttonToggled()));
}

void YaWindowExtraButtonBase::paintEvent(QPaintEvent* event)
{
	emit doPaint();
}

bool YaWindowExtraButtonBase::event(QEvent* event)
{
	if (event->type() == QEvent::HoverEnter ||
	    event->type() == QEvent::HoverLeave)
	{
		emit doPaint();
	}

	return QToolButton::event(event);
}

void YaWindowExtraButtonBase::buttonToggled()
{
	emit doPaint();
}

//----------------------------------------------------------------------------
// YaWindowExtraButton
//----------------------------------------------------------------------------

YaWindowExtraButton::YaWindowExtraButton(QWidget* parent)
	: YaWindowExtraButtonBase(parent)
{
	QSize size(12, 12);
	setFixedSize(size);
}

void YaWindowExtraButton::setType(YaWindowExtraButton::Type type)
{
	type_ = type;
}

void YaWindowExtraButton::paintEvent(QPaintEvent* e)
{
	YaWindow* yaWindow = dynamic_cast<YaWindow*>(window());
	if (!yaWindow) {
		QPainter p(this);
		p.drawPixmap(0, 0, currentPixmap());
		return;
	}

	YaWindowExtraButtonBase::paintEvent(e);
}

QPixmap YaWindowExtraButton::currentPixmap() const
{
	static QPixmap closeBlack;
	static QPixmap closeBlackHovered;

	YaWindow* yaWindow = dynamic_cast<YaWindow*>(window());
	if (yaWindow) {
		QPixmap pixmap;
		switch (type_) {
		case Close:
			return yaWindow->theme().theme().closeButton(underMouse());
		case Maximize:
			return yaWindow->theme().theme().maximizeButton(underMouse());
		case Minimize:
			return yaWindow->theme().theme().minimizeButton(underMouse());
		}

		Q_ASSERT(false);
	}

	if (closeBlack.isNull()) {
		closeBlackHovered = QPixmap(":images/window/buttons/black/close.png");
		closeBlack = PixmapUtil::createTransparentPixmap(closeBlackHovered.size());
		QPainter tmp(&closeBlack);
		tmp.setOpacity(0.5);
		tmp.drawPixmap(0, 0, closeBlackHovered);
	}
	return underMouse() ? closeBlackHovered : closeBlack;
}
