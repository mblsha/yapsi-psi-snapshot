#ifndef PIXMAPUTIL_H
#define PIXMAPUTIL_H

class QPixmap;
class QSize;

namespace PixmapUtil
{
	QPixmap createTransparentPixmap(QSize size);
	QPixmap createTransparentPixmap(int width, int height);
	QPixmap rotatedPixmap(int rotation, const QPixmap& sourcePixmap);
};

#endif
