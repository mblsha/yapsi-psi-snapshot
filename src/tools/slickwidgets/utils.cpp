/*
 * utils.cpp
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

#include "utils.h"

#include <QBrush>
#include <QLinearGradient>

namespace Ya {

	Utils *Utils::utils = 0;

	static const QColor BASE_COLOR = QColor(0xFF, 0xFF, 0xFF);
	static const QColor LIGHT_COLOR = QColor(0xFD, 0xEB, 0x9F);
	static const QColor DEFAULT_COLOR = QColor(0xFF, 0xFA, 0xE7);

/*/
	static const QPalette PALETTE = QPalette(
		QBrush(QColor(0x00, 0x00, 0x00)), // window text
		QBrush(QColor(0xF0, 0xEC, 0xE8)), // button
		QBrush(QColor(0xFF, 0x00, 0x00)), // light -- the color to temporarily highlight stuff
		QBrush(QColor(0x00, 0xFF, 0x00)), // dark
		QBrush(QColor(0xE8, 0xE8, 0xE0)), // mid
		QBrush(QColor(0x00, 0x00, 0x00)), // text
		QBrush(QColor(0xC0, 0xA0, 0x80)), // bright_text
		QBrush(QColor(0x00, 0x00, 0xFF)), // base -- the base color (it has alternate variant, kinda shadowed)
		QBrush(QColor(0xB0, 0xA8, 0xA0))  // window
	);
*/
	static const int TEXT_MARGIN = 8;

	Utils& Utils::instance()
	{
		if (!utils) {
			utils = new Utils(QCoreApplication::instance());
		}
		return *utils;
	}

	Utils::Utils(QObject *parent)
		: QObject(parent)
	{
		palette_ = QPalette(
			QBrush(QColor(0x00, 0x00, 0x00)), // window text
			QBrush(DEFAULT_COLOR), // button
			QBrush(LIGHT_COLOR), // light
			QBrush(QColor(0xC0, 0xC0, 0x90)), // dark
			QBrush(QColor(0xE8, 0xE8, 0xE0)), // mid
			QBrush(QColor(0x00, 0x00, 0x00)), // text
			QBrush(QColor(0xC0, 0xA0, 0x80)), // bright_text
			QBrush(BASE_COLOR), // base
			QBrush(DEFAULT_COLOR)  // window
		);

		palette_.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);

		palette_.setColor(QPalette::Highlight, LIGHT_COLOR);
		palette_.setColor(QPalette::HighlightedText, Qt::black);
	}

	QPalette Utils::palette() const
	{
		return palette_;
	}

	int Utils::textMargin() const
	{
		return TEXT_MARGIN;
	}

	QColor Utils::baseColor() const
	{
		return BASE_COLOR;
	}

	QColor Utils::lightColor() const
	{
		return LIGHT_COLOR;
	}

	QBrush Utils::gradientTopToBottom(const QRect& rect) const
	{
		QLinearGradient linearGrad(
			QPointF(rect.width() / 2, rect.top()), QPointF(rect.width() / 2, rect.bottom())
		);
		linearGrad.setColorAt(0, palette().light());
		linearGrad.setColorAt(1, palette().base());
		return QBrush(linearGrad);
	}

}
