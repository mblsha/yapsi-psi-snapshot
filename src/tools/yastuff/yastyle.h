/*
 * yastyle.h - custom style where some widgets are native
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

#ifndef YASTYLE_H
#define YASTYLE_H

#include <QPlastiqueStyle>
#include <QHash>
#include <QFont>

class QStyleOptionMenuItem;

class YaStyle : public QPlastiqueStyle
{
public:
	YaStyle(QStyle* defaultStyle);
	~YaStyle();

	static void makeMeNativeLooking(QWidget* widget);
	static QStyle* defaultStyle();
	static QPalette defaultPalette();
	static QPalette menuPalette();
	static QPalette useNativeTextSelectionColors(const QPalette& pal);

	// reimplemented
	int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const;
	void drawItemText(QPainter* painter, const QRect& rectangle, int alignment, const QPalette& palette, bool enabled, const QString& text, QPalette::ColorRole textRole) const;
	void drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const;
	void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const;
	void drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* p, const QWidget* widget) const;
	QSize sizeFromContents(ContentsType ct, const QStyleOption* opt, const QSize& contentsSize, const QWidget* w) const;
	int pixelMetric(PixelMetric metric, const QStyleOption* option , const QWidget* widget) const;
	QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const;
	SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, const QPoint& pos, const QWidget* widget) const;
	QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt, SubControl sc, const QWidget* widget) const;
	void polish(QWidget* widget);
	void unpolish(QWidget* widget);

private:
	QHash<QByteArray, QPalette> paletteHash_;
	QPalette defaultPalette_;
	QPalette menuPalette_;
	QFont menuFont_;
	static QStyle* defaultStyle_;

	QPalette defaultPaletteFor(QWidget* w) const;
	void processStyleOptionMenuItem(QStyleOptionMenuItem* option, const QWidget* w) const;
};

#endif
