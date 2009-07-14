/*
 * yaflashingscrollbar.cpp
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

#include "yaflashingscrollbar.h"

#include <QApplication>
#include <QPlastiqueStyle>
#include <QStyleOptionSlider>
#include <QTimer>
#include <QPointer>

//----------------------------------------------------------------------------
// YaFlashingScrollBarStyle
//----------------------------------------------------------------------------

class YaFlashingScrollBarStyle : public QPlastiqueStyle
{
public:
	YaFlashingScrollBarStyle(QStyle* defaultStyle, YaFlashingScrollBar* scrollBar)
		: defaultStyle_(defaultStyle)
		, scrollBar_(scrollBar)
	{}

	static void initStyleOption(QStyleOptionSlider* option, const YaFlashingScrollBar* scrollBar)
	{
		if (!scrollBar->underMouse()) {
			bool flashUp = scrollBar->flashUp() && scrollBar->flashing();
			bool flashDown = scrollBar->flashDown() && !scrollBar->flashing();

			if (flashUp) {
				option->activeSubControls |= QStyle::SC_ScrollBarSubLine;
			}
			if (flashDown) {
				option->activeSubControls |= QStyle::SC_ScrollBarAddLine;
			}
			if (flashUp || flashDown) {
// #ifndef Q_WS_MAC
				option->state |= QStyle::State_MouseOver;
// #else
// 				option->state |= QStyle::State_Sunken;
// #endif
				option->state |= QStyle::State_Active | QStyle::State_Enabled;
			}
		}
	}

	// reimplemented
	void polish(QWidget* widget)
	{
		defaultStyle()->polish(widget);
	}

	void unpolish(QWidget* widget)
	{
		defaultStyle()->unpolish(widget);
	}

	void drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		defaultStyle()->drawControl(ce, opt, p, w);
	}

	void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		defaultStyle()->drawPrimitive(pe, opt, p, w);
	}

	void drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* p, const QWidget* widget) const
	{
		if (cc == QStyle::CC_ScrollBar) {
			const QStyleOptionSlider *qsos = qstyleoption_cast<const QStyleOptionSlider*>(opt);
			if (qsos && !scrollBar_.isNull()) {
				QStyleOptionSlider o(*qsos);
				initStyleOption(&o, scrollBar_);

				defaultStyle()->drawComplexControl(cc, &o, p, widget);
				return;
			}
		}
		defaultStyle()->drawComplexControl(cc, opt, p, widget);
	}

	QSize sizeFromContents(ContentsType ct, const QStyleOption* opt, const QSize& contentsSize, const QWidget* w) const
	{
		return defaultStyle()->sizeFromContents(ct, opt, contentsSize, w);
	}

	SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, const QPoint& pos, const QWidget* widget) const
	{
		return defaultStyle()->hitTestComplexControl(cc, opt, pos, widget);
	}

	QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt, SubControl sc, const QWidget* widget) const
	{
		return defaultStyle()->subControlRect(cc, opt, sc, widget);
	}

	int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
	{
		return defaultStyle()->styleHint(hint, option, widget, returnData);
	}

	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
	{
		return QPlastiqueStyle::pixelMetric(metric, option, widget);
		// this one gets us into forever recursion on Qt 4.3.3.
		// return defaultStyle()->pixelMetric(metric, option, widget);
	}

	QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
	{
		return defaultStyle()->subElementRect(element, option, widget);
	}

private:
	QStyle* defaultStyle_;
	QPointer<YaFlashingScrollBar> scrollBar_;
	QStyle* defaultStyle() const { return defaultStyle_; }
};

//----------------------------------------------------------------------------
// YaFlashingScrollBar
//----------------------------------------------------------------------------

YaFlashingScrollBar::YaFlashingScrollBar(QWidget* parent)
	: QScrollBar(parent)
	, flashing_(false)
	, flashUp_(false)
	, flashDown_(false)
{
	YaFlashingScrollBarStyle* style = new YaFlashingScrollBarStyle(qApp->style(), this);
	style->setParent(this);
	setStyle(style);

	flashTimer_ = new QTimer(this);
	flashTimer_->setInterval(500);
	flashTimer_->setSingleShot(false);
	connect(flashTimer_, SIGNAL(timeout()), SLOT(flash()));

	updateFlashing();
}

YaFlashingScrollBar::~YaFlashingScrollBar()
{
}

void YaFlashingScrollBar::flash()
{
	flashing_ = !flashing_;
	repaint();
}

bool YaFlashingScrollBar::flashing() const
{
	return flashing_;
}

bool YaFlashingScrollBar::flashUp() const
{
	return flashUp_;
}

void YaFlashingScrollBar::setFlashUp(bool flashUp)
{
	if (flashUp_ != flashUp) {
		flashUp_ = flashUp;
		updateFlashing();
	}
}

bool YaFlashingScrollBar::flashDown() const
{
	return flashDown_;
}

void YaFlashingScrollBar::setFlashDown(bool flashDown)
{
	if (flashDown_ != flashDown) {
		flashDown_ = flashDown;
		updateFlashing();
	}
}

void YaFlashingScrollBar::updateFlashing()
{
	bool active = flashUp_ || flashDown_;
	if (active != flashTimer_->isActive()) {
		if (active)
			flashTimer_->start();
		else
			flashTimer_->stop();
	}
}

/**
 * For this function to work, it requires a Qt patch in order to make
 * QScrollBar::initStyleOption() virtual.
 */
void YaFlashingScrollBar::initStyleOption(QStyleOptionSlider* option) const
{
	QScrollBar::initStyleOption(option);
	YaFlashingScrollBarStyle::initStyleOption(option, this);
}
