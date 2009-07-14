/*
 * trimmablemultilinelabel.cpp
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

#include "trimmablemultilinelabel.h"

#include <QAbstractTextDocumentLayout>
#include <QCoreApplication>
#include <QPainter>
#include <QTextEdit>
#include <QTextDocument>
#include <QTextBlock>
#include <QScrollBar>

#include "yavisualutil.h"

class UnscrollableScrollBar : public QScrollBar
{
public:
	UnscrollableScrollBar(QWidget* parent)
		: QScrollBar(parent)
	{}

	/**
	 * Requires setValue() slot to be declared virtual in qabstractslider.h
	 */
	// reimplemented
	void setValue(int) {
		QScrollBar::setValue(0);
	}
};

TrimmableMultilineLabel::TrimmableMultilineLabel(QWidget* parent)
	: QFrame(parent)
	, text_(QString())
	, minNumLines_(1)
	, maxNumLines_(1)
	, textEdit_(0)
	, drawTextFadeOut_(false)
{
	textEdit_ = new QTextEdit(this);
	textEdit_->setVerticalScrollBar(new UnscrollableScrollBar(textEdit_));
	textEdit_->setReadOnly(true);
	textEdit_->setFrameShape(QFrame::NoFrame);
	textEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	textEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	textEdit_->installEventFilter(this);
	textEdit_->viewport()->installEventFilter(this);

	// we don't want no steenking background
	QPalette palette = textEdit_->palette();
	palette.setBrush(QPalette::Base, QBrush());
	textEdit_->setPalette(palette);
}

TrimmableMultilineLabel::~TrimmableMultilineLabel()
{
}

void TrimmableMultilineLabel::doSetFont(const QFont& font)
{
	textEdit_->setFont(font);
}

QFontMetrics TrimmableMultilineLabel::actualFontMetrics() const
{
	return textEdit_->fontMetrics();
}

QString TrimmableMultilineLabel::text() const
{
	return text_;
}

void TrimmableMultilineLabel::setText(const QString& text)
{
	text_ = text;
	textEdit_->setHtml(text_);
	relayout();
}

int TrimmableMultilineLabel::maxNumLines() const
{
	return maxNumLines_;
}

void TrimmableMultilineLabel::setMaxNumLines(int maxNumLines)
{
	maxNumLines_ = maxNumLines;
	relayout();
}

int TrimmableMultilineLabel::minNumLines() const
{
	return minNumLines_;
}

void TrimmableMultilineLabel::setMinNumLines(int minNumLines)
{
	minNumLines_ = minNumLines;
	relayout();
}

QSize TrimmableMultilineLabel::sizeHint() const
{
	return sizeHint_;
}

QSize TrimmableMultilineLabel::minimumSizeHint() const
{
	QSize sh;
	sh.setWidth(16);
	sh.setHeight(actualFontMetrics().height() * minNumLines_);
	return sh;
}

void TrimmableMultilineLabel::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);
	relayout();
}

void TrimmableMultilineLabel::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);
}

void TrimmableMultilineLabel::relayout()
{
	setUpdatesEnabled(false);
	QTextFrameFormat frameFormat = textEdit_->document()->rootFrame()->frameFormat();
	frameFormat.setLeftMargin(0);
	frameFormat.setRightMargin(0);
	frameFormat.setTopMargin(0);
	frameFormat.setBottomMargin(0);
	textEdit_->document()->rootFrame()->setFrameFormat(frameFormat);

	textEdit_->document()->setTextWidth(width());
	QRect textEditRect(rect());

	int lineHeight = actualFontMetrics().height();
	int maxHeight = (lineHeight * maxNumLines_);
	int minHeight = (lineHeight * minNumLines_) - 1;

	int maxPos = -1;
	int minY   = -1;

	for (int y = maxHeight; y >= minHeight; y--) {
		QPoint pos(width(), y);
		int documentPos = textEdit_->document()->documentLayout()->hitTest(pos, Qt::FuzzyHit);
		if (maxPos < documentPos) {
			maxPos = documentPos;
		}
		if (maxPos == documentPos) {
			minY = y;
		}
	}

	minY = qMin(lineHeight * maxNumLines_, minY + lineHeight);
	minY = qMin(minY, (int)textEdit_->document()->size().height());
	sizeHint_ = QSize(16, minY);

	int numLinesVisible = height() / lineHeight;
	textEditRect.setHeight(numLinesVisible * lineHeight);
	textEdit_->setWordWrapMode(numLinesVisible == 1 ?
	                           QTextOption::NoWrap :
	                           QTextOption::WrapAtWordBoundaryOrAnywhere);
	textEdit_->setGeometry(textEditRect);
	setUpdatesEnabled(true);
	updateGeometry();
}

Qt::TextInteractionFlags TrimmableMultilineLabel::textInteractionFlags() const
{
	return textEdit_->textInteractionFlags();
}

void TrimmableMultilineLabel::setTextInteractionFlags(Qt::TextInteractionFlags textInteractionFlags)
{
	if (textInteractionFlags == Qt::NoTextInteraction)
		textEdit_->viewport()->setCursor(cursor());
	else
		textEdit_->viewport()->unsetCursor();

	textEdit_->setTextInteractionFlags(textInteractionFlags);
}

void TrimmableMultilineLabel::setDrawTextFadeOut(bool drawTextFadeOut, QColor backgroundColor)
{
	drawTextFadeOut_ = drawTextFadeOut;
	backgroundColor_ = backgroundColor;
	update();
}

bool TrimmableMultilineLabel::eventFilter(QObject* obj, QEvent* e)
{
	static bool filtering = false;
	if (filtering)
		return QFrame::eventFilter(obj, e);

	filtering = true;

	if (obj == textEdit_->viewport() && e->type() == QEvent::Paint) {
		QPaintEvent pe(textEdit_->viewport()->rect());
		QCoreApplication::sendEvent(textEdit_->viewport(), &pe);

		if (drawTextFadeOut_) {
			QPainter p(textEdit_->viewport());
			QRect rect(textEdit_->viewport()->rect());
			Ya::VisualUtil::drawTextFadeOut(&p, rect, backgroundColor_, 15);
		}

		filtering = false;
		return true;
	}

	filtering = false;
	return QFrame::eventFilter(obj, e);
}
