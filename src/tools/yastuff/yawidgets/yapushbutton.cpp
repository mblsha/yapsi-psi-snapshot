/*
 * yapushbutton.cpp - custom push button
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

#include "yapushbutton.h"

#include <QStylePainter>
#include <QStyleOptionButton>
#include <QStyle>
#include <QTimer>

YaPushButton::YaPushButton(QWidget* parent)
	: QPushButton(parent)
	, buttonStyle_(ButtonStyle_Normal)
	, initialized_(false)
{
}

void YaPushButton::init()
{
	if (initialized_)
		return;

	initialized_ = true;
	setButtonStyle(buttonStyle_);

	setMinimumWidth(minimumSizeHint().width());
	setFixedHeight(minimumSizeHint().height());
}

void YaPushButton::updateButtonStyle()
{
#ifndef YAPSI_NO_STYLESHEETS
	QString style = buttonStyle_ == ButtonStyle_Destructive ? "red" : "green";

	QString styleSheet = QString(
	"QPushButton {"
	"	color: white;"
	"	border-image: url(:/images/pushbutton/%1/pushbutton.png) 12px 11px 12px 10px;"
	"	border-width: 0px 11px 0px 10px;"
	"}"
	""
	"QPushButton:disabled {"
	"	color: white;"
	"}"
	""
	"QPushButton:hover {"
	"	border-image: url(:/images/pushbutton/%1/pushbutton_hover.png) 12px 11px 12px 10px;"
	"}"
	""
	"QPushButton:pressed {"
	"	border-image: url(:/images/pushbutton/%1/pushbutton_pressed.png) 12px 11px 12px 10px;"
	"}"
	).arg(style);
	setStyleSheet(styleSheet);
#endif
}

void YaPushButton::setButtonStyle(YaPushButton::ButtonStyle buttonStyle)
{
	buttonStyle_ = buttonStyle;
	updateButtonStyle();
}

QStyleOptionButton YaPushButton::getStyleOption() const
{
	QStyleOptionButton opt;
	opt.init(this);
	opt.features = QStyleOptionButton::None;
	if (isFlat())
		opt.features |= QStyleOptionButton::Flat;
	if (menu())
		opt.features |= QStyleOptionButton::HasMenu;
	if (autoDefault() || isDefault())
		opt.features |= QStyleOptionButton::AutoDefaultButton;
	if (isDefault())
		opt.features |= QStyleOptionButton::DefaultButton;
	if (isDown() /*|| menuOpen*/)
		opt.state |= QStyle::State_Sunken;
	if (isChecked())
		opt.state |= QStyle::State_On;
	if (!isFlat() && !isDown())
		opt.state |= QStyle::State_Raised;
	opt.text = text();
	// opt.icon = icon();
	if (!icon().isNull()) {
		opt.iconSize = icon().actualSize(QSize(32, 32));
	}
	return opt;
}

QSize YaPushButton::sizeHint() const
{
	ensurePolished();
	int w = 0, h = 0;

	QStyleOptionButton opt = getStyleOption();

	QFontMetrics fm = fontMetrics();
	QSize sz = fm.size(Qt::TextShowMnemonic, opt.text);
	w += sz.width();
	h = qMax(h, sz.height());

	QSize size = style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this);

	if (!icon().isNull()) {
		int leftMargin = (size.width() - w) / 2;
		size.setWidth(size.width() + opt.iconSize.width() + 10 - leftMargin);
		size.setHeight(qMax(size.height(), opt.iconSize.height()));
	}

	size.setHeight(minimumSizeHint().height());
	return size;
}

QSize YaPushButton::minimumSizeHint() const
{
	return QSize(94, 27 + iconPopOut());
}

// ripped from QCommonStyle
static void drawBevel(QPainter* p, const QStyleOptionButton* btn, const QWidget* widget)
{
	QRect br = btn->rect;
	int dbi = widget->style()->pixelMetric(QStyle::PM_ButtonDefaultIndicator, btn, widget);
	if (btn->features & QStyleOptionButton::DefaultButton)
		widget->style()->drawPrimitive(QStyle::PE_FrameDefaultButton, btn, p, widget);
	if (btn->features & QStyleOptionButton::AutoDefaultButton)
		br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);

	QStyleOptionButton tmpBtn = *btn;
	tmpBtn.rect = br;
	widget->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &tmpBtn, p, widget);

	// if (btn->features & QStyleOptionButton::HasMenu) {
	// 	int mbi = widget->style()->pixelMetric(QStyle::PM_MenuButtonIndicator, btn, widget);
	// 	QRect ir = btn->rect;
	// 	QStyleOptionButton newBtn = *btn;
	// 	newBtn.rect = QRect(ir.right() - mbi, ir.height() - 20, mbi, ir.height() - 4);
	// 	widget->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, p, widget);
	// }
}

void YaPushButton::adjustRect(const QStyleOptionButton* btn, QRect* rect) const
{
	if (btn->state & (QStyle::State_On | QStyle::State_Sunken))
		rect->translate(0, 1);
}

// ripped from QCommonStyle
static void drawLabel(QPainter* p, const QStyleOptionButton* btn, const QWidget* widget)
{
	bool enabled = btn->state & QStyle::State_Enabled;
	QRect ir = btn->rect.adjusted(0, -2, 0, -2);
	uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
	if (!widget->style()->styleHint(QStyle::SH_UnderlineShortcut, btn, widget))
		tf |= Qt::TextHideMnemonic;

	if (!btn->iconSize.isEmpty()) {
		QIcon::Mode mode = enabled ? QIcon::Normal
		                   : QIcon::Disabled;
		if (mode == QIcon::Normal && btn->state & QStyle::State_HasFocus)
			mode = QIcon::Active;
		QIcon::State state = QIcon::Off;
		if (btn->state & QStyle::State_On)
			state = QIcon::On;
		QPixmap pixmap = btn->icon.pixmap(btn->iconSize, mode, state);
		int pixw = 25; // btn->iconSize.width();
		int pixh = btn->iconSize.height();
		//Center the icon if there is no text

		QPoint point;
		if (btn->text.isEmpty()) {
			point = QPoint(ir.x() + ir.width() / 2 - pixw / 2,
			               ir.y() + ir.height() / 2 - pixh / 2);
		}
		else {
			point = QPoint(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2);
		}
		if (btn->direction == Qt::RightToLeft)
			point.rx() += pixw;

		if ((btn->state & (QStyle::State_On | QStyle::State_Sunken)) && btn->direction == Qt::RightToLeft)
			point.rx() -= widget->style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, btn, widget) * 2;

		// p->drawPixmap(widget->style()->visualPos(btn->direction, btn->rect, point), pixmap);

		if (btn->direction == Qt::RightToLeft)
			ir.translate(-4, 0);
		else
			ir.translate(pixw + 4, 0);
		ir.setWidth(ir.width() - (pixw + 4));
		// left-align text if there is
		if (!btn->text.isEmpty())
			tf |= Qt::AlignLeft;
	}
	else {
		tf |= Qt::AlignHCenter;
	}
	// widget->style()->drawItemText(p, ir, tf, btn->palette, enabled,
	//                               btn->text, QPalette::ButtonText);
	QColor color = btn->palette.color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText);
	p->setPen(color);
	p->drawText(ir, tf, btn->text);
	// p->drawRect(ir);
}

float YaPushButton::disabledOpacity() const
{
	return 0.5;
}

int YaPushButton::iconPopOut() const
{
	return 2;
}

void YaPushButton::paintEvent(QPaintEvent*)
{
	Q_ASSERT(initialized_);

	QStyleOptionButton opt = getStyleOption();
	opt.rect.adjust(0, iconPopOut(), 0, 0);
	QStylePainter p(this);
	p.fillRect(rect(), Qt::transparent); // it's better on Windows in some weird configs

	if (!isEnabled())
		p.setOpacity(disabledOpacity());

	// p.drawControl(QStyle::CE_PushButtonBevel, opt);
	drawBevel(&p, &opt, this);

	// p.drawRect(rect().adjusted(0, 0, -1, -1));

	int topMargin = isDown() ? 1 : 0;

	adjustRect(&opt, &opt.rect);
	// p.drawControl(QStyle::CE_PushButtonLabel, opt);
	drawLabel(&p, &opt, this);

	if (!icon().isNull()) {
		QRect r(QPoint(1, topMargin), opt.iconSize);
		icon().paint(&p, r, Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
	}
}

void YaPushButton::initAllButtons(QWidget* window)
{
	foreach(YaPushButton* button, window->findChildren<YaPushButton*>()) {
		button->init();
	}
}
