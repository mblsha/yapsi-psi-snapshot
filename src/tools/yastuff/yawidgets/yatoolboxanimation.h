/*
 * yatoolboxanimation.h
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

#ifndef YATOOLBOXANIMATION_H
#define YATOOLBOXANIMATION_H

#include "baseanimatedstackedwidget.h"

class YaToolBox;
class YaToolBoxPage;

class YaToolBoxAnimation : public BaseAnimatedStackedWidgetAnimation
{
	Q_OBJECT
public:
	YaToolBoxAnimation();
	~YaToolBoxAnimation();
	void setCurrentState(YaToolBox* toolBox, const QList<YaToolBoxPage*>& pages);
	void setNewState(YaToolBox* toolBox, const QList<YaToolBoxPage*>& pages);

protected:
	// reimplemented
	virtual void animationPaint(QPainter* p);

private:
	struct PageState {
		QPixmap pixmap;
		bool selected;
		YaToolBoxPage* page;
		int oldY;
		int newY;
	};
	QList<PageState*> states_;

	QSize fullSize_;
};

#endif
