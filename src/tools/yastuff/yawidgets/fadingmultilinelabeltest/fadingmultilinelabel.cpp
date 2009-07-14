/*
 * fadingmultilinelabel.cpp - multiline label with fading
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

#include "fadingmultilinelabel.h"

#include <QPainter>
#include <QTextLayout>

#include "yavisualutil.h"

FadingMultilineLabel::FadingMultilineLabel(QWidget* parent)
	: QWidget(parent)
	, textLayout_(0)
{
	textOption_ = QTextOption(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
	textOption_.setWrapMode(QTextOption::WordWrap);
}

FadingMultilineLabel::~FadingMultilineLabel()
{
	delete textLayout_;
}

const QString& FadingMultilineLabel::text() const
{
	return text_;
}

void FadingMultilineLabel::setText(const QString& text)
{
	text_ = text;
	positionLayouts();
}

void FadingMultilineLabel::positionLayouts()
{
	QFontMetricsF fm = fontMetrics();
	qreal lineHeight = fm.height();

	delete textLayout_;
	textLayout_ = 0;

	textLayout_ = new QTextLayout(text_, font());
	textLayout_->setTextOption(textOption_);
	// textLayout_->setAdditionalFormats(formats[p]);
	textLayout_->beginLayout();

	qreal x = 0.0;
	qreal y = 0.0;
	qreal ymax = 0.0;

	fadeOuts_.clear();
	QTextLine line = textLayout_->createLine();

	while (line.isValid()) {
		line.setPosition(QPointF(x, y));
		line.setLineWidth(width());

		if (line.naturalTextWidth() > width()) {
			fadeOuts_ << QRect(x, y, width(), lineHeight);
		}

		y += line.height();
		line = textLayout_->createLine();
	}

	textLayout_->endLayout();

	sizeHint_ = QSize(width(), y);
	updateGeometry();
}

void FadingMultilineLabel::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.fillRect(rect(), backgroundColor());

	if (textLayout_) {
		textLayout_->draw(&p, QPointF(0, 0));

		foreach(QRect fadeOut, fadeOuts_) {
			Ya::VisualUtil::drawTextFadeOut(&p, fadeOut, backgroundColor_);
			// p.fillRect(fadeOut, Qt::red);
		}
	}
}

void FadingMultilineLabel::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	positionLayouts();
	update();
}

const QColor& FadingMultilineLabel::backgroundColor() const
{
	return backgroundColor_;
}

void FadingMultilineLabel::setBackgroundColor(const QColor& backgroundColor)
{
	backgroundColor_ = backgroundColor;
	update();
}

QSize FadingMultilineLabel::sizeHint() const
{
	QWidget::sizeHint();
	return sizeHint_;
}
