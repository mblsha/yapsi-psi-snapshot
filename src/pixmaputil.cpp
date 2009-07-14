#include "pixmaputil.h"

#include <QBitmap>
#include <QImage>
#include <QPainter>

QPixmap PixmapUtil::createTransparentPixmap(QSize size)
{
	return PixmapUtil::createTransparentPixmap(size.width(), size.height());
}

QPixmap PixmapUtil::createTransparentPixmap(int width, int height)
{
	// Now there's third alternative:
	// QPainter p(&pixmap);
	// p.fillRect(pixmap.rect(), Qt::transparent);
#if 1
	QPixmap pix(width, height);
	QBitmap mask(width, height);
	pix.fill();
	mask.clear();
	pix.setMask(mask);
#else
	QImage img(width, height, QImage::Format_ARGB32);
	img.fill(0x00000000);
	QPixmap pix = QPixmap::fromImage(img);
#endif
	return pix;
}

QPixmap PixmapUtil::rotatedPixmap(int rotation, const QPixmap& sourcePixmap)
{
	QPixmap result = PixmapUtil::createTransparentPixmap(sourcePixmap.width(), sourcePixmap.height());
	QPainter p(&result);

	QPoint offset(sourcePixmap.width() / 2, sourcePixmap.height() / 2);
	p.translate(offset);
	QMatrix matrix = p.worldMatrix();
	if (rotation) {
		matrix.rotate(rotation);
		p.setWorldMatrix(matrix);

		p.setRenderHint(QPainter::Antialiasing);
	}
	p.drawPixmap(-offset, sourcePixmap);

	return result;
}
