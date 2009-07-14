/*
 * YaBusyWidget2.h - cool animating widget
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

#include "yabusywidget2.h"

#include <QTimeLine>
#include <QPainter>
#include <QPixmap>

#include <QCoreApplication>
#include <QDir>

static const QString framesPath = ":/images/busy/";

//----------------------------------------------------------------------------
// YaBusyWidget2Storage
//----------------------------------------------------------------------------

class YaBusyWidget2Storage : public QObject
{
public:
	static YaBusyWidget2Storage* instance()
	{
		if (!instance_)
			instance_ = new YaBusyWidget2Storage();
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
	YaBusyWidget2Storage()
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

	static YaBusyWidget2Storage* instance_;
};

YaBusyWidget2Storage* YaBusyWidget2Storage::instance_ = 0;

//----------------------------------------------------------------------------
// YaBusyWidget2
//----------------------------------------------------------------------------

YaBusyWidget2::YaBusyWidget2(QWidget* parent)
	: QWidget(parent)
	, state_(Normal)
	, fadeTimeLine_(0)
	, scrollTimeLine_(0)
{
	YaBusyWidget2Storage::instance();
	fadeTimeLine_ = new QTimeLine(200, this);
	fadeTimeLine_->setCurveShape(QTimeLine::LinearCurve);
	fadeTimeLine_->setFrameRange(0, YaBusyWidget2Storage::instance()->numFrames() - 1);
	fadeTimeLine_->setLoopCount(1);
	connect(fadeTimeLine_, SIGNAL(frameChanged(int)), SLOT(animate()));

	scrollTimeLine_ = new QTimeLine(250, this);
	scrollTimeLine_->setCurveShape(QTimeLine::LinearCurve);
	scrollTimeLine_->setFrameRange(0, YaBusyWidget2Storage::instance()->frameSize().width());
	scrollTimeLine_->setLoopCount(0);
	scrollTimeLine_->start();
	scrollTimeLine_->setPaused(true);
	scrollTimeLine_->setCurrentTime(0);
	connect(scrollTimeLine_, SIGNAL(frameChanged(int)), SLOT(animate()));

	fadeTimeLine_->setUpdateInterval(1000 / 30);
	scrollTimeLine_->setUpdateInterval(1000 / 30);

	// fadeTimeLine_->setDuration(fadeTimeLine_->updateInterval() * fadeTimeLine_->endFrame());
	// scrollTimeLine_->setDuration(scrollTimeLine_->updateInterval() * scrollTimeLine_->endFrame());

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}

YaBusyWidget2::~YaBusyWidget2()
{
}

QSize YaBusyWidget2::minimumSizeHint() const
{
	return YaBusyWidget2Storage::instance()->frameSize();
}

QSize YaBusyWidget2::sizeHint() const
{
	QSize sh = minimumSizeHint();
	sh.setWidth(88);
	return sh;
}

bool YaBusyWidget2::isActive() const
{
	return state_ == Activating
	       || state_ == Activated;
}

void YaBusyWidget2::setActive(bool a)
{
	if (a)
		start();
	else
		stop();
}

void YaBusyWidget2::start()
{
	if (isActive())
		return;

	state_ = Activating;
	updateTimeLines();
	emit stateChanged();
}

void YaBusyWidget2::stop()
{
	if (!isActive())
		return;

	state_ = Deactivating;
	updateTimeLines();
	emit stateChanged();
}

void YaBusyWidget2::updateTimeLines()
{
	switch (state_) {
	case Activating:
		if (scrollTimeLine_->state() != QTimeLine::Paused) {
			scrollTimeLine_->setPaused(true);
		}
		fadeTimeLine_->setDirection(QTimeLine::Forward);
		if (fadeTimeLine_->state() != QTimeLine::Running) {
			fadeTimeLine_->start();
		}
		break;
	case Activated:
		if (scrollTimeLine_->state() != QTimeLine::Running) {
			scrollTimeLine_->setPaused(false);
		}
		if (fadeTimeLine_->state() != QTimeLine::NotRunning) {
			fadeTimeLine_->stop();
		}
		break;
	case Deactivating:
		if (scrollTimeLine_->state() != QTimeLine::Paused) {
			scrollTimeLine_->setPaused(true);
		}
		fadeTimeLine_->setDirection(QTimeLine::Backward);
		if (fadeTimeLine_->state() != QTimeLine::Running) {
			fadeTimeLine_->start();
		}
		break;
	case Normal:
		;
	}
}

void YaBusyWidget2::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.drawTiledPixmap(rect(), framePixmap(), QPoint(scrollTimeLine_->currentFrame(), 0));
}

void YaBusyWidget2::resizeEvent(QResizeEvent *)
{
	update();
}

const QPixmap& YaBusyWidget2::framePixmap() const
{
	return YaBusyWidget2Storage::instance()->frame(fadeTimeLine_->currentFrame(), height());
}

void YaBusyWidget2::animate()
{
	switch (state_) {
	case Normal:
		break;
	case Activating:
		if (fadeTimeLine_->currentFrame() >= fadeTimeLine_->endFrame()) {
			state_ = Activated;
			updateTimeLines();
			emit stateChanged();
		}
		update();
		break;
	case Activated:
		update();
		break;
	case Deactivating:
		if (fadeTimeLine_->currentFrame() <= 0) {
			state_ = Normal;
			updateTimeLines();
			emit stateChanged();
		}
		update();
		break;
	}
}
