/*
 * yajidlabel.cpp - clickable dash-underlined JID label widget
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

#include "yajidlabel.h"

#include <QPainter>

#include "yavisualutil.h"

YaJidLabel::YaJidLabel(QWidget* parent)
	: QLabel(parent)
{
}

void YaJidLabel::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	drawBackground(&p);
	p.drawText(textRect(), alignment(), text());
	Ya::VisualUtil::drawTextDashedUnderline(&p, text(), textRect(), alignment());

	if (fontMetrics().width(text()) > textRect().width()) {
		Ya::VisualUtil::drawTextFadeOut(&p, textRect(), Ya::VisualUtil::highlightBackgroundColor(), 15);
	}
}

QRect YaJidLabel::textRect() const
{
	return rect();
}

Qt::Alignment YaJidLabel::alignment() const
{
	return Qt::AlignLeft | Qt::AlignVCenter;
}

void YaJidLabel::drawBackground(QPainter* painter)
{
	Q_UNUSED(painter);
}
