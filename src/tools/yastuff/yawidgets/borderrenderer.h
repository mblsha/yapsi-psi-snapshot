/*
 * borderrenderer.h - poor man's CSS3 border-image
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

#ifndef BORDERRENDERER_H
#define BORDERRENDERER_H

#include <QHash>
#include <QPixmap>

class QPainter;

class BorderRenderer;

// TODO: Think of renaming to something like NSDrawNinePartImage
class BorderRectCalculator
{
public:
	BorderRectCalculator(BorderRenderer* renderer)
		: renderer_(renderer)
	{}

	virtual ~BorderRectCalculator() {}

	enum PixmapLocation {
		Unknown = -1,

		TopLeft = 0,
		TopCenter,
		TopRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		BottomLeft,
		BottomCenter,
		BottomRight,

		Arrow
	};

	virtual QRect pixmapRect(PixmapLocation pixmap, const QRect& rect) const;

protected:
	bool isTop(PixmapLocation pixmap) const;
	bool isMiddle(PixmapLocation pixmap) const;
	bool isBottom(PixmapLocation pixmap) const;
	bool isLeft(PixmapLocation pixmap) const;
	bool isCenter(PixmapLocation pixmap) const;
	bool isRight(PixmapLocation pixmap) const;

private:
	BorderRenderer* renderer_;
};

class BorderRenderer
{
public:
	~BorderRenderer();

	static BorderRenderer* rendererFor(const QString& path);

	void draw(QPainter* painter, const QRect& rect) const;
	void setBorderRectCalculator(BorderRectCalculator* calculator);

	QSize arrowSize() const;
	QRect innerRect(QRect outerRect) const;

private:
	BorderRenderer(const QString& path);

	bool haveArrow() const;
	BorderRectCalculator::PixmapLocation pixmapLocation(const QString& fileName) const;
	QHash<BorderRectCalculator::PixmapLocation, QPixmap> pixmaps_;
	// QHash<QRect, QHash<BorderRectCalculator::PixmapLocation, QRect> > rectCache_;
	BorderRectCalculator* rectCalculator_;
	friend class BorderRectCalculator;
};

#endif
