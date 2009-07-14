/*
 * yalabel.cpp 
 * Copyright (C) 2008  Yandex LLC (Alexei Matiouchkine)
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

#include "yalabel.h"

#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>

#include "yawindow.h"
#include "textutil.h"
#include "vcardfactory.h"

#include "yacommon.h"
#include "yamainwin.h"
#include "tabdlg.h"

static const QList<PsiAccount*> EMPTY_LIST;

YaLabel::YaLabel(QWidget* parent)
	: M11EditableLabel(parent)
	, effectiveWidth_(-1)
	, isEventNotifierLabel_(false)
{
	setOpenExternalLinks(false);
	setEditable(false);
}

void YaLabel::paintEvent(QPaintEvent*)
{
	QPainter p(this);	

	// Deal with empty names
	QString txt = text();
	if (txt.isEmpty()) {
		txt = Ya::ellipsis();
	}

	// Deal with veryveryveryveryveryveryveryveryveryverylong names
	QRect rectNorm(rect());
	int w = rectNorm.width();
	if (effectiveWidth_ > 0) {
		w = qMin(effectiveWidth_, w);
	}

	if (w < fontMetrics().width(txt)) {
		txt = fontMetrics().elidedText(txt, Qt::ElideRight, w);
	}

	// p.setRenderHint(QPainter::TextAntialiasing, true);
	QFont f = p.font();
	f.setPixelSize(!isEventNotifierLabel_ ? 14 : 12);
	f.setBold(false);
	// f.setStyleStrategy(QFont::PreferAntialias);
	p.setFont(f);

	p.setPen(textColor());

	p.drawText(rectNorm.x(), rectNorm.y() + fontMetrics().ascent(), txt);

	// QRect firstChar = rectNorm;
	// firstChar.setWidth(fontMetrics().width(txt.at(0)));
	// 
	// p.save();
	// p.setPen(Qt::red);
	// p.drawText(rectNorm.x(), rectNorm.y() + fontMetrics().ascent(), txt.at(0));
	// p.restore();
	// 
	// if (txt.length() > 1) {
	// 	QRect theRest = rectNorm;
	// 	theRest.adjust(firstChar.width(), 0, 0, 0);
	// 	p.drawText(theRest.x(), theRest.y() + fontMetrics().ascent(), txt.mid(1));
	// }
}

QSize YaLabel::sizeHint() const
{
	QSize sh = M11EditableLabel::sizeHint();
	sh.setWidth(fontMetrics().width(text()));
	return sh;
}

QSize YaLabel::minimumSizeHint() const
{
	QSize sh = M11EditableLabel::sizeHint();
	sh.setWidth(fontMetrics().width(Ya::ellipsis()));
	return sh;
}

QString YaLabel::text() const
{
	return M11EditableLabel::text();
}

void YaLabel::setEffectiveWidth(int effectiveWidth)
{
	if (effectiveWidth_ == effectiveWidth)
		return;
	effectiveWidth_ = effectiveWidth;
	update();
}

int YaLabel::effectiveWidth() const
{
	return effectiveWidth_;
}

bool YaLabel::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == windowExtra_ && (event->type() == QEvent::Move ||
	                            event->type() == QEvent::Show ||
	                            event->type() == QEvent::Hide))
	{
		updateEffectiveWidth(windowExtra_);
	}

	return M11EditableLabel::eventFilter(obj, event);
}

void YaLabel::setWindowExtra(YaWindowExtra* extra)
{
	windowExtra_ = extra;
	Q_ASSERT(!windowExtra_.isNull());
	windowExtra_->installEventFilter(this);
	updateEffectiveWidth(windowExtra_);
}

void YaLabel::updateEffectiveWidth(YaWindowExtra* extra)
{
	if (!extra)
		return;

	if (!extra->isVisible()) {
		setEffectiveWidth(-1);
		return;
	}

	QPoint topLeft = mapToGlobal(rect().topLeft());
	QPoint topRight = extra->mapToGlobal(extra->rect().topLeft());

	if (dynamic_cast<YaMainWin*>(window())) {
		topRight = window()->mapToGlobal(window()->rect().topRight()) - QPoint(5, 0);
	}

	setEffectiveWidth(topRight.x() - topLeft.x());
}

QColor YaLabel::textColor() const
{
	if (isEventNotifierLabel_)
		return Qt::black;

	if (dynamic_cast<YaMainWin*>(window())) {
		return dynamic_cast<YaMainWin*>(window())->theme().theme().nickColor();
	}

	if (dynamic_cast<TabDlg*>(window())) {
		return dynamic_cast<TabDlg*>(window())->theme().theme().nickColor();
	}

	return Qt::black;
}

bool YaLabel::isEventNotifierLabel() const
{
	return isEventNotifierLabel_;
}

void YaLabel::setIsEventNotifierLabel(bool isEventNotifierLabel)
{
	isEventNotifierLabel_ = isEventNotifierLabel;
}
