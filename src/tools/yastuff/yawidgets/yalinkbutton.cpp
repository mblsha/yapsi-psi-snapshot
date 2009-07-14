/*
 * yalinkbutton.cpp - hyperlink-styled button
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

#include "yalinkbutton.h"

#include <QEvent>
#include <QPainter>

#include "yavisualutil.h"

YaLinkButton::YaLinkButton(QWidget* parent)
	: QPushButton(parent)
	, alignment_(Qt::AlignHCenter)
	, encloseInBrackets_(false)
{
	setPaintOptions();
}

QSize YaLinkButton::sizeHint() const
{
	return minimumSizeHint();
}

QSize YaLinkButton::minimumSizeHint() const
{
	QString text = this->text();
	if (encloseInBrackets_) {
		text = "(" + text + ")";
	}
	return QSize(fontMetrics().width(text), fontMetrics().height());
}

void YaLinkButton::setPaintOptions()
{
	setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
	QPalette palette = this->palette();
	palette.setColor(
		QPalette::ButtonText,
		isEnabled() ? Ya::VisualUtil::linkButtonColor() : QColor(0xDD, 0xCC, 0xBB)
	);
	setPalette(palette);

	QFont font = Ya::VisualUtil::normalFont();
	font.setUnderline(isEnabled());

	// avoid recursion
	if (font != this->font()) {
		setFont(font);
		updateGeometry();
	}
}

void YaLinkButton::changeEvent(QEvent* e)
{
	QPushButton::changeEvent(e);

	if (e->type() == QEvent::EnabledChange) {
		setPaintOptions();
	}
	else if (e->type() == QEvent::FontChange) {
		setPaintOptions();
	}
}

void YaLinkButton::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	QRect r = rect();

	QFont enclosureFont = font();
	enclosureFont.setUnderline(false);

	if (encloseInBrackets_) {
		// other alignments are not supported for now
		Q_ASSERT(alignment_ == Qt::AlignLeft);

		p.save();
		p.setFont(enclosureFont);
		p.setPen(Qt::black);

		p.drawText(r, alignment_ | Qt::AlignVCenter, "(");
		r.setLeft(r.left() + fontMetrics().width("("));

		p.restore();
	}

	p.drawText(r,
	           alignment_ | Qt::AlignVCenter,
	           text());
	r.setLeft(r.left() + fontMetrics().width(text()));

	if (encloseInBrackets_) {
		p.setFont(enclosureFont);
		p.setPen(Qt::black);
		p.drawText(r, alignment_ | Qt::AlignVCenter, ")");
	}
}

Qt::Alignment YaLinkButton::alignment() const
{
	return alignment_;
}

void YaLinkButton::setAlignment(Qt::Alignment alignment)
{
	alignment_ = alignment;
}

bool YaLinkButton::encloseInBrackets() const
{
	return encloseInBrackets_;
}

void YaLinkButton::setEncloseInBrackets(bool enclose)
{
	encloseInBrackets_ = enclose;
	update();
	updateGeometry();
}
