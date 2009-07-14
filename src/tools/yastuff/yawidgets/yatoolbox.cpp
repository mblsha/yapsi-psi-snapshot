/*
 * yatoolbox.cpp
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

#include "yatoolbox.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>

#include "yatoolboxanimation.h"
#include "yatoolboxpage.h"

YaToolBox::YaToolBox(QWidget *parent)
	: BaseAnimatedStackedWidget(parent)
{
	normalPage_ = new QWidget();
	layout_ = new QVBoxLayout(normalPage_);
	layout_->setMargin(0);
	layout_->setSpacing(0);
}

YaToolBox::~YaToolBox()
{
}

QLayout* YaToolBox::layout() const
{
	return layout_;
}

void YaToolBox::addPage(YaToolBoxPageButton* button, QWidget* widget)
{
	YaToolBoxPage* page = new YaToolBoxPage(normalPage_, button, widget);
	connect(button, SIGNAL(setCurrentPage()), SLOT(setCurrentPage()));
	connect(button, SIGNAL(toggleCurrentPage()), SLOT(toggleCurrentPage()));
	page->setCurrentPage(pages_.isEmpty());
	pages_.append(page);

	layout_->addWidget(page);
}

void YaToolBox::setCurrentPage()
{
	YaToolBoxPageButton* button = static_cast<YaToolBoxPageButton*>(sender());
	setCurrentPage(button);
}

void YaToolBox::toggleCurrentPage()
{
	YaToolBoxPageButton* toActivate = 0;
	foreach(YaToolBoxPage* page, pages_) {
		if (!page->isCurrentPage()) {
			toActivate = page->button();
			break;
		}
	}

	if (toActivate)
		setCurrentPage(toActivate);
}

void YaToolBox::setCurrentPage(YaToolBoxPageButton* button)
{
	if (button->page()->isCurrentPage())
		return;

	setCurrentWidget(button);
}

int YaToolBox::count() const
{
	return pages_.count();
}

YaToolBoxPageButton* YaToolBox::button(int index) const
{
	return pages_[index]->button();
}

int YaToolBox::currentIndex() const
{
	for (int i = 0; i < count(); ++i)
		if (button(i)->isCurrentPage())
			return i;

	return -1;
}

QWidget* YaToolBox::normalPage() const
{
	return normalPage_;
}

BaseAnimatedStackedWidgetAnimation* YaToolBox::createAnimationPage() const
{
	return new YaToolBoxAnimation();
}

void YaToolBox::setCurrentState(QWidget* widget)
{
	Q_UNUSED(widget);
	static_cast<YaToolBoxAnimation*>(animationPage())->setCurrentState(this, pages_);
}

void YaToolBox::setNewState()
{
	static_cast<YaToolBoxAnimation*>(animationPage())->setNewState(this, pages_);
}

void YaToolBox::setCurrentWidget_internal(QWidget* widget)
{
	YaToolBoxPageButton* button = static_cast<YaToolBoxPageButton*>(widget);
	foreach(YaToolBoxPage* page, pages_) {
		page->setCurrentPage(button->page() == page);
	}
}

void YaToolBox::activateNormalPageLayout()
{
	foreach(YaToolBoxPage* page, pages_) {
		page->layout()->invalidate();
		page->layout()->activate();
	}
	layout_->invalidate();
	layout_->activate();
}
