/*
 * yatoolboxanimation.cpp 
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

#include "yatoolboxanimation.h"

#include <QLayout>
#include <QPainter>
#include <QTimeLine>

#include "yatoolbox.h"
#include "yatoolboxpage.h"

YaToolBoxAnimation::YaToolBoxAnimation()
	: BaseAnimatedStackedWidgetAnimation()
{
}

YaToolBoxAnimation::~YaToolBoxAnimation()
{
	qDeleteAll(states_);
	states_.clear();
}

void YaToolBoxAnimation::setCurrentState(YaToolBox* toolBox, const QList<YaToolBoxPage*>& pages)
{
	qDeleteAll(states_);
	states_.clear();

	fullSize_ = QSize();
	int animationSize = 0;
	int y = toolBox->layout()->contentsRect().top();
	foreach(YaToolBoxPage* page, pages) {
		PageState* state = new PageState;
		state->page     = page;
		state->selected = page->isCurrentPage();
		state->oldY     = y;
		states_.append(state);

		y += page->minimumSizeHint().height();
		if (page->isCurrentPage()) {
			fullSize_ = page->size();
			animationSize = fullSize_.height() - page->minimumSizeHint().height();
			y += animationSize;
		}
	}

	animationTimeLine()->setFrameRange(0, animationSize);
}

void YaToolBoxAnimation::setNewState(YaToolBox* toolBox, const QList<YaToolBoxPage*>& pages)
{
	Q_UNUSED(pages);
#ifdef ENABLE_YATOOLBOX_ANIMATION
	if (fullSize_.isNull())
		return;

	int y = toolBox->layout()->contentsRect().top();
	foreach(PageState* state, states_) {
		YaToolBoxPage* page = state->page;
		state->newY = y;

		bool isCurrent = page->isCurrentPage();
		if (isCurrent)
			y += fullSize_.height();
		else
			y += page->minimumSizeHint().height();

		page->setCurrentPage(true);
		page->resizeAndUpdateLayout(fullSize_);

		QPixmap pix(page->size());
		page->render(&pix);
		state->pixmap = pix;

		page->setCurrentPage(isCurrent);
	}
#endif
}

void YaToolBoxAnimation::animationPaint(QPainter* p)
{
	foreach(PageState* state, states_) {
		int y = state->oldY;
		if (state->newY > state->oldY)
			y += animationTimeLine()->currentFrame();
		else if (state->newY < state->oldY)
			y -= animationTimeLine()->currentFrame();
		p->drawPixmap(0, y, state->pixmap);
	}
}
