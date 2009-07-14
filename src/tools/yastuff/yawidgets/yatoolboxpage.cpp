/*
 * yatoolboxpage.cpp
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

#include "yatoolboxpage.h"

#include <QVBoxLayout>
#include <QStackedWidget>

//----------------------------------------------------------------------------
// YaToolBoxPageButton
//----------------------------------------------------------------------------

YaToolBoxPageButton::YaToolBoxPageButton()
	: QFrame()
	, isCurrentPage_(false)
{
}

YaToolBoxPage* YaToolBoxPageButton::page() const
{
	return static_cast<YaToolBoxPage*>(parentWidget());
}

bool YaToolBoxPageButton::isCurrentPage() const
{
	return isCurrentPage_;
}

void YaToolBoxPageButton::setIsCurrentPage(bool isCurrentPage)
{
	isCurrentPage_ = isCurrentPage;
}

void YaToolBoxPageButton::updateMask()
{
}

//----------------------------------------------------------------------------
// YaToolBoxPage
//----------------------------------------------------------------------------

YaToolBoxPage::YaToolBoxPage(QWidget* parent, YaToolBoxPageButton* button, QWidget* widget)
	: QFrame(parent)
	, isCurrentPage_(false)
	, button_(button)
	, widget_(widget)
{
	setFrameStyle(NoFrame);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	button->setParent(this);
	widget->setParent(this);

	widget->setMinimumSize(1, 1);
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	widget->hide();

	layout->addWidget(button_);
	layout->addWidget(widget_);
}

bool YaToolBoxPage::isCurrentPage() const
{
	// return widget_->isVisible();
	// we can't simply use widget_->isVisible() because if widget is visible
	// and we do this in quick succession, we'll get not what we wanted:
	//
	//   setCurrentPage(true)
	//   widget_->isVisible() is still false, since we haven't returned to the
	//                        eventloop still
	return isCurrentPage_;
}

void YaToolBoxPage::setCurrentPage(bool current)
{
	isCurrentPage_ = current;;
	widget_->setVisible(current);
	button_->setIsCurrentPage(current);
}

QSize YaToolBoxPage::minimumSizeHint() const
{
	QSize sh = button_->minimumSizeHint();
	QSize ms = button_->maximumSize();
	return QSize(qMin(sh.width(), ms.width()), qMin(sh.height(), ms.height()));
}

QSize YaToolBoxPage::sizeHint() const
{
	return minimumSizeHint();
}

void YaToolBoxPage::resizeEvent(QResizeEvent* e)
{
	QFrame::resizeEvent(e);
	button_->updateMask();
}

void invokeResized(QWidget* widget)
{
	if (widget->metaObject()->indexOfSlot("resized()") != -1) {
		QMetaObject::invokeMethod(widget, "resized", Qt::DirectConnection);
	}
}

void YaToolBoxPage::resizeAndUpdateLayout(QSize size)
{
	resize(size);
	layout()->invalidate();
	layout()->activate();

	// work-around scrollbars showing at an incorrect place
	invokeResized(widget_);
	QStackedWidget* stack = dynamic_cast<QStackedWidget*>(widget_);
	if (stack && stack->currentWidget()) {
		stack->layout()->invalidate();
		stack->layout()->activate();
		invokeResized(stack->currentWidget());
	}

	button_->updateMask();
}
