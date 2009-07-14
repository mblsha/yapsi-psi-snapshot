/*
 * customtimeline.h - a generalization of QTimeLine that could animate different types
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

#ifndef CUSTOMTIMELINE_H
#define CUSTOMTIMELINE_H

#include <QTimeLine>

class GTimeLine : public QObject
{
	Q_OBJECT
public:
	GTimeLine(int duration, QObject *parent);
	~GTimeLine();

	QTimeLine::CurveShape curveShape() const;
	void setCurveShape(QTimeLine::CurveShape curveShape);

	QTimeLine::Direction direction() const;
	void setDirection(QTimeLine::Direction direction);

	int loopCount() const;
	void setLoopCount(int loopCount);

	int updateInterval() const;
	void setUpdateInterval(int updateInterval);

	int duration() const;
	void setDuration(int duration);

	int currentTime() const;
	void setCurrentTime(int msec);

	QTimeLine::State state() const;

public slots:
	void resume();
	void setPaused(bool paused);
	void start();
	void stop();
	void toggleDirection();

signals:
	void frameChanged();
	void finished();

private slots:
	void timeLineFinished();

protected:
	QTimeLine* timeLine() const { return timeLine_; }

private:
	QTimeLine* timeLine_;
};

template <typename Value>
Value __frameForTime(QTimeLine* timeLine, QTimeLine::Direction direction, Value start, Value end, int msec)
{
	Q_UNUSED(direction);
	return start + Value((end - start) * timeLine->valueForTime(msec));
}

template <class Value>
class CustomTimeLine : public GTimeLine
{
public:
	CustomTimeLine(int duration, QObject *parent = 0)
		: GTimeLine(duration, parent)
	{}

	Value currentFrame() const
	{
		return frameForTime(currentTime());
	}

	Value startFrame() const
	{
		return startFrame_;
	}

	void setStartFrame(Value value)
	{
		startFrame_ = value;
		timeLine()->setStartFrame(0);
	}

	Value endFrame() const
	{
		return endFrame_;
	}

	void setEndFrame(Value value)
	{
		endFrame_ = value;
		timeLine()->setEndFrame(1000000);
	}

	void setFrameRange(Value start, Value end)
	{
		setStartFrame(start);
		setEndFrame(end);
	}

	Value frameForTime(int msec) const
	{
		return __frameForTime(timeLine(), direction(), startFrame_, endFrame_, msec);
	}

private:
	Value startFrame_;
	Value endFrame_;
};

#endif
