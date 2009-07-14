/*
 * borderrenderer.cpp - poor man's CSS3 border-image
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

#include "borderrenderer.h"

#include <QCoreApplication>
#include <QDir>
#include <QPainter>

//----------------------------------------------------------------------------
// BorderRendererFactory
//----------------------------------------------------------------------------

class BorderRendererFactory : public QObject
{
	Q_OBJECT
public:
	static BorderRendererFactory* instance()
	{
		if (!instance_)
			instance_ = new BorderRendererFactory();
		return instance_;
	}

	BorderRenderer* rendererFor(const QString& path)
	{
		if (!borderRenderers_.contains(path))
			return 0;
		return borderRenderers_[path];
	}

	void addRendererFor(const QString& path, BorderRenderer* renderer)
	{
		Q_ASSERT(!borderRenderers_.contains(path));
		borderRenderers_[path] = renderer;
	}

private:
	BorderRendererFactory()
		: QObject(QCoreApplication::instance())
	{}

	~BorderRendererFactory()
	{
		qDeleteAll(borderRenderers_);
		borderRenderers_.clear();
	}

	static BorderRendererFactory* instance_;
	QHash<QString, BorderRenderer*> borderRenderers_;
};

BorderRendererFactory* BorderRendererFactory::instance_ = 0;

//----------------------------------------------------------------------------
// BorderRenderer
//----------------------------------------------------------------------------

BorderRenderer* BorderRenderer::rendererFor(const QString& path)
{
	BorderRenderer* renderer = BorderRendererFactory::instance()->rendererFor(path);
	if (!renderer) {
		renderer = new BorderRenderer(path);
		BorderRendererFactory::instance()->addRendererFor(path, renderer);
	}
	return renderer;
}

void BorderRenderer::draw(QPainter* painter, const QRect& rect) const
{
	QHashIterator<BorderRectCalculator::PixmapLocation, QPixmap> it(pixmaps_);
	while (it.hasNext()) {
		it.next();

		// if (it.key() == BorderRectCalculator::Arrow)
		// 	continue;

		painter->drawPixmap(rectCalculator_->pixmapRect(it.key(), rect), it.value());
	}

	// if (pixmaps_.contains(BorderRectCalculator::Arrow)) {
	// 	painter->drawPixmap(rectCalculator_->pixmapRect(BorderRectCalculator::Arrow, rect),
	// 	                    pixmaps_[BorderRectCalculator::Arrow]);
	// }
}

BorderRenderer::BorderRenderer(const QString& path)
	: rectCalculator_(new BorderRectCalculator(this))
{
	QDir dir(path);
	foreach(QString file, dir.entryList(QDir::Files)) {
		QPixmap pix(dir.filePath(file));
		if (!pix.isNull())
			pixmaps_[pixmapLocation(file)] = pix;
	}
}

BorderRenderer::~BorderRenderer()
{
	delete rectCalculator_;
}

QSize BorderRenderer::arrowSize() const
{
	if (haveArrow())
		return pixmaps_.value(BorderRectCalculator::Arrow).size();
	return QSize();
}

QRect BorderRenderer::innerRect(QRect outerRect) const
{
	QRect result;
	result.setLeft(rectCalculator_->pixmapRect(BorderRectCalculator::MiddleLeft, outerRect).right());
	result.setTop(rectCalculator_->pixmapRect(BorderRectCalculator::TopCenter, outerRect).bottom());
	result.setRight(rectCalculator_->pixmapRect(BorderRectCalculator::MiddleRight, outerRect).left());
	result.setBottom(rectCalculator_->pixmapRect(BorderRectCalculator::BottomCenter, outerRect).top());
	return result;
}

bool BorderRenderer::haveArrow() const
{
	return pixmaps_.contains(BorderRectCalculator::Arrow);
}

BorderRectCalculator::PixmapLocation BorderRenderer::pixmapLocation(const QString& fileName) const
{
	QHash<QString, BorderRectCalculator::PixmapLocation> locations;
	locations["arrow"]         = BorderRectCalculator::Arrow;
	locations["top_left"]      = BorderRectCalculator::TopLeft;
	locations["top_center"]    = BorderRectCalculator::TopCenter;
	locations["top_right"]     = BorderRectCalculator::TopRight;
	locations["middle_left"]   = BorderRectCalculator::MiddleLeft;
	locations["middle_center"] = BorderRectCalculator::MiddleCenter;
	locations["middle_right"]  = BorderRectCalculator::MiddleRight;
	locations["bottom_left"]   = BorderRectCalculator::BottomLeft;
	locations["bottom_center"] = BorderRectCalculator::BottomCenter;
	locations["bottom_right"]  = BorderRectCalculator::BottomRight;
	QString name = fileName.split(".")[0];
	if (!locations.contains(name)) {
		qWarning("BottomRight::pixmapLocation(%s) failed.", fileName.toLatin1().data());
		return BorderRectCalculator::Unknown;
	}
	return locations.value(name);
}

void BorderRenderer::setBorderRectCalculator(BorderRectCalculator* calculator)
{
	if (rectCalculator_)
		delete rectCalculator_;
	rectCalculator_ = calculator;
}

//----------------------------------------------------------------------------
// BorderRectCalculator
//----------------------------------------------------------------------------

QRect BorderRectCalculator::pixmapRect(BorderRectCalculator::PixmapLocation pixmap, const QRect& rect) const
{
	if (pixmap == BorderRectCalculator::Arrow) {
		QPixmap arrow = renderer_->pixmaps_.value(pixmap);
		return QRect(QPoint(rect.left(), rect.top() + (rect.height() - arrow.height()) / 2), arrow.size());
	}

	int x = 0, y = 0, w = 0, h = 0;
	if (isLeft(pixmap)) {
		if (renderer_->haveArrow())
			x = pixmapRect(BorderRectCalculator::Arrow, rect).right();
		else
			x = rect.left();
		w = renderer_->pixmaps_.value(pixmap).width();
	}
	else if (isRight(pixmap)) {
		w = renderer_->pixmaps_.value(pixmap).width();
		x = rect.right() - w;
	}
	else if (isCenter(pixmap)) {
		x = pixmapRect(TopLeft, rect).right() + 1;
		w = pixmapRect(TopRight, rect).left() - x;
	}

	if (isTop(pixmap)) {
		y = rect.top();
		h = renderer_->pixmaps_.value(pixmap).height();
	}
	else if (isBottom(pixmap)) {
		h = renderer_->pixmaps_.value(pixmap).height();
		y = rect.bottom() - h;
	}
	else if (isMiddle(pixmap)) {
		y = pixmapRect(TopLeft, rect).bottom() + 1;
		h = pixmapRect(BottomLeft, rect).top() - y;
	}

	return QRect(x, y, w, h);
}

bool BorderRectCalculator::isTop(BorderRectCalculator::PixmapLocation pixmap) const
{
	switch (pixmap) {
	case TopLeft:
	case TopCenter:
	case TopRight:
		return true;
	default:
		return false;
	}
}

bool BorderRectCalculator::isMiddle(BorderRectCalculator::PixmapLocation pixmap) const
{
	switch (pixmap) {
	case MiddleLeft:
	case MiddleCenter:
	case MiddleRight:
		return true;
	default:
		return false;
	}
}

bool BorderRectCalculator::isBottom(BorderRectCalculator::PixmapLocation pixmap) const
{
	switch (pixmap) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
		return true;
	default:
		return false;
	}
}

bool BorderRectCalculator::isLeft(BorderRectCalculator::PixmapLocation pixmap) const
{
	switch (pixmap) {
	case TopLeft:
	case MiddleLeft:
	case BottomLeft:
		return true;
	default:
		return false;
	}
}

bool BorderRectCalculator::isCenter(BorderRectCalculator::PixmapLocation pixmap) const
{
	switch (pixmap) {
	case TopCenter:
	case MiddleCenter:
	case BottomCenter:
		return true;
	default:
		return false;
	}
}

bool BorderRectCalculator::isRight(BorderRectCalculator::PixmapLocation pixmap) const
{
	switch (pixmap) {
	case TopRight:
	case MiddleRight:
	case BottomRight:
		return true;
	default:
		return false;
	}
}

#include "borderrenderer.moc"
