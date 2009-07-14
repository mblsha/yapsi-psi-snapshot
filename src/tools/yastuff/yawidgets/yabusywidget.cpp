/*
 * yabusywidget.h - cool animating widget
 * Copyright (C) 2007  Michail Pishchagin
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

#include "yabusywidget.h"

#include <QTimer>
#include <QPainter>
#include <QPixmap>

#include <QCoreApplication>
#include <QDir>

static const int fps            = 30;
static const int offsetSpeed    = 2;
static const QString framesPath = ":/images/busy/";

//----------------------------------------------------------------------------
// YaBusyWidgetStorage
//----------------------------------------------------------------------------

class YaBusyWidgetStorage : public QObject
{
public:
	static YaBusyWidgetStorage* instance()
	{
		if (!instance_)
			instance_ = new YaBusyWidgetStorage();
		return instance_;
	}

	int numFrames() const
	{
		Q_ASSERT(frames_.count() > 0);
		return frames_.count();
	}

	QSize frameSize() const
	{
		return frames_.first().size();
	}

	const QPixmap& frame(int num, int height)
	{
		const QPixmap& pix = frames_.at(num);
		if (pix.height() == height)
			return pix;
		tempPixmap_ = pix.scaledToHeight(height);
		return tempPixmap_;
	}

private:
	YaBusyWidgetStorage()
		: QObject(QCoreApplication::instance())
	{
		QDir dir(framesPath);
		foreach(QString file, dir.entryList(QDir::Files)) {
			QPixmap pix(dir.filePath(file));
			if (!pix.isNull()) {
				frames_.resize(frames_.count() + 1);
				frames_.last() = pix;
			}
		}
	}

	QVector<QPixmap> frames_;
	QPixmap tempPixmap_;

	static YaBusyWidgetStorage* instance_;
};

YaBusyWidgetStorage* YaBusyWidgetStorage::instance_ = 0;

//----------------------------------------------------------------------------
// YaBusyWidget
//----------------------------------------------------------------------------

YaBusyWidget::YaBusyWidget(QWidget* parent)
	: QWidget(parent)
	, state_(Normal)
	, frameNum_(0)
	, offset_(0)
	, timer_(0)
{
	timer_ = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), SLOT(animate()));

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	YaBusyWidgetStorage::instance();
}

YaBusyWidget::~YaBusyWidget()
{
}

QSize YaBusyWidget::minimumSizeHint() const
{
	return YaBusyWidgetStorage::instance()->frameSize();
}

QSize YaBusyWidget::sizeHint() const
{
	QSize sh = minimumSizeHint();
	sh.setWidth(88);
	return sh;
}

bool YaBusyWidget::isActive() const
{
	return state_ == Activating
	       || state_ == Activated;
}

void YaBusyWidget::setActive(bool a)
{
	if (a)
		start();
	else
		stop();
}

void YaBusyWidget::start()
{
	if (isActive())
		return;

	state_ = Activating;
	timer_->start(1000 / fps);
	emit stateChanged();
}

void YaBusyWidget::stop()
{
	if (!isActive())
		return;

	state_ = Deactivating;
	emit stateChanged();
}

void YaBusyWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.drawTiledPixmap(rect(), framePixmap(), QPoint(offset_, 0));
}

void YaBusyWidget::resizeEvent(QResizeEvent *)
{
	update();
}

const QPixmap& YaBusyWidget::framePixmap() const
{
	return YaBusyWidgetStorage::instance()->frame(frameNum_, height());
}

void YaBusyWidget::animate()
{
	switch (state_) {
	case Normal:
		break;
	case Activating:
		if (++frameNum_ >= YaBusyWidgetStorage::instance()->numFrames() - 1) {
			frameNum_ = YaBusyWidgetStorage::instance()->numFrames() - 1;
			state_ = Activated;
			emit stateChanged();
		}
		update();
		break;
	case Activated:
		offset_ += offsetSpeed;
		if (offset_ >= framePixmap().width())
			offset_ -= framePixmap().width();
		update();
		break;
	case Deactivating:
		if (--frameNum_ <= 0) {
			frameNum_ = 0;
			state_ = Normal;
			emit stateChanged();
			timer_->stop();
		}
		update();
		break;
	}
}
