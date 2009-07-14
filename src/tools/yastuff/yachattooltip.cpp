/*
 * yachattooltip.cpp - custom contact tooltip for chat dialog
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

#include "yachattooltip.h"
#include "yachattiplabel.h"

#include "yarostertiplabel.h"

//----------------------------------------------------------------------------
// YaChatToolTipPosition
//----------------------------------------------------------------------------

QPoint YaChatToolTipPosition::calculateTipPosition(const QWidget* _label, const QSize& size) const
{
	Q_UNUSED(size);
	const YaChatTipLabel* label = dynamic_cast<const YaChatTipLabel*>(_label);
	QRect screen = screenRect();

	QPoint p = globalRect.topLeft();
	if (p.x() + label->width() > screen.x() + screen.width())
		p.rx() = screen.right() - label->width();
	if (p.x() < screen.x())
		p.rx() = screen.x();
	if (p.y() + label->height() > screen.y() + screen.height())
		p.ry() = screen.bottom() - label->height();
	if (p.y() < screen.y())
		p.ry() = screen.y();

	return p;
}

//----------------------------------------------------------------------------
// YaChatToolTip
//----------------------------------------------------------------------------

YaChatToolTip *YaChatToolTip::instance_ = 0;

YaChatToolTip* YaChatToolTip::instance()
{
	if (!instance_)
		instance_ = new YaChatToolTip();
	return instance_;
}

ToolTipPosition* YaChatToolTip::createTipPosition(const QPoint& cursorPos, const QWidget* parentWidget)
{
	Q_UNUSED(cursorPos);
	return new YaChatToolTipPosition(lastRect_, parentWidget);
}

PsiTipLabel* YaChatToolTip::createTipLabel(const QString& text, QWidget* parent)
{
	YaChatTipLabel* label = new YaChatTipLabel(parent);
	YaRosterToolTip::instance()->registerTipLabel(label);
	label->init(text);
	updateLabel(label, text);
	updateTipLabelProperties(label);
	return label;
}

void YaChatToolTip::updateTipLabelProperties(PsiTipLabel* label)
{
// #ifdef Q_WS_WIN
	YaRosterTipLabel* tipLabel = dynamic_cast<YaRosterTipLabel*>(label);
	if (tipLabel) {
		tipLabel->setLeaveTimerPermanentlyDisabled(true);
	}
// #endif
}

bool YaChatToolTip::isVisible() const
{
	return PsiToolTip::isVisible() &&
	       dynamic_cast<YaChatTipLabel*>(PsiTipLabel::instance());
}
