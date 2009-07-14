/*
 * smoothscrollbar.h - a QScrollBar with smooth scrolling
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

#ifndef SMOOTHSCROLLBAR_H
#define SMOOTHSCROLLBAR_H

#include <QScrollBar>

class QAbstractScrollArea;
class QTimeLine;

class SmoothScrollBar : public QScrollBar
{
	Q_OBJECT
public:
	SmoothScrollBar(QWidget* parent);

	enum Mode {
		Normal = 0,
		PreferScrollToBottom = 1
	};

	static void install(QAbstractScrollArea* scrollArea, Mode mode = Normal);

	bool preferScrollToBottom() const;
	void setPreferScrollToBottom(bool enable);

	bool enableSmoothScrolling() const;
	void setEnableSmoothScrolling(bool enable);

	bool rangeUpdatesEnabled() const;
	void setRangeUpdatesEnabled(bool rangeUpdatesEnabled);

public slots:
	// reimplemented
	void setValue(int value);
	void setRange(int min, int max);

	void setValueImmediately(int value);

protected:
#ifdef YAPSI
	// reimplemented
	void mouseMoveEvent(QMouseEvent* event);

	void updateCursor();
#endif
	void internalSetValue(int value);

private slots:
	void timeLineFrameChanged(int value);

private:
	QTimeLine* timeLine_;
	bool preferScrollToBottom_;
	bool enableSmoothScrolling_;
	bool rangeUpdatesEnabled_;
	int savedMin_;
	int savedMax_;

	inline int boundValue(int val) const
	{
		return qMax(minimum(), qMin(maximum(), val));
	}
};

#endif
