/*
 * fadingmultilinelabel.h - multiline label with fading
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

#ifndef FADINGMULTILINELABEL_H
#define FADINGMULTILINELABEL_H

#include <QWidget>
#include <QTextOption>

class QTextLayout;

class FadingMultilineLabel : public QWidget
{
	Q_OBJECT
public:
	FadingMultilineLabel(QWidget* parent);
	~FadingMultilineLabel();

	const QString& text() const;
	void setText(const QString& text);

	const QColor& backgroundColor() const;
	void setBackgroundColor(const QColor& backgroundColor);

	// reimplemented
	QSize sizeHint() const;

protected:
	// reimplemented
	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);

private:
	void positionLayouts();

private:
	QString text_;
	QTextLayout* textLayout_;
	QTextOption textOption_;
	QList<QRect> fadeOuts_;
	QColor backgroundColor_;
	QSize sizeHint_;
};

#endif
