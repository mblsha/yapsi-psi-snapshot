/*
 * yastatusbar.cpp
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

#include "yastatusbar.h"

#include <QAbstractItemModel>
#include <QTimer>
#include <QSizeGrip>
#include <QStackedWidget>

#ifndef WIDGET_PLUGIN
#include "psicon.h"
#include "psiaccount.h"
#endif
#include "psicontactlist.h"
#include "yainformer.h"
#include "yainformerbutton.h"
#include "yastatusbarlayout.h"
#include "psioptions.h"

static const QString showStatusBarOptionPath = "options.ya.main-window.status-bar.show";

YaStatusBar::YaStatusBar(QWidget* parent)
	: QFrame(parent)
	, controller_(0)
	, model_(0)
	, stack_(0)
	, topLayout_(0)
	, layout_(0)
	, sizeGrip_(0)
	, busyWidget_(0)
	, actualStatusBar_(0)
	, layoutDirty_(false)
	, shouldBeVisible_(false)
{
	busyWidget_ = new YaBusyWidget(this);
	busyWidget_->hide();

	topLayout_ = new QVBoxLayout(this);
	topLayout_->setMargin(0);
	topLayout_->setSpacing(0);

	stack_ = new QStackedWidget(this);
	stack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	topLayout_->addWidget(stack_);
	stack_->hide();

	actualStatusBar_ = new QWidget(this);
	topLayout_->addWidget(actualStatusBar_);

	sizeGrip_ = new QSizeGrip(actualStatusBar_);

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
}

YaStatusBar::~YaStatusBar()
{
}

void YaStatusBar::setModel(QAbstractItemModel* model)
{
	if (model == model_)
		return;

	if (model_) {
		disconnect(model_, SIGNAL(destroyed()),
		           this, SLOT(modelDestroyed()));
		disconnect(model_, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
		           this, SLOT(dataChanged(QModelIndex, QModelIndex)));
		disconnect(model_, SIGNAL(rowsInserted(QModelIndex, int, int)),
		           this, SLOT(rowsInserted(QModelIndex, int, int)));
		disconnect(model_, SIGNAL(rowsRemoved(QModelIndex, int, int)),
		           this, SLOT(rowsRemoved(QModelIndex, int, int)));

		disconnect(model_, SIGNAL(modelReset()), this, SLOT(reset()));
		// disconnect(model_, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
	}

	model_ = model;

	if (model_) {
		connect(model_, SIGNAL(destroyed()),
		        this, SLOT(modelDestroyed()));
		connect(model_, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
		        this, SLOT(dataChanged(QModelIndex, QModelIndex)));
		connect(model_, SIGNAL(rowsInserted(QModelIndex, int, int)),
		        this, SLOT(rowsInserted(QModelIndex, int, int)));
		connect(model_, SIGNAL(rowsRemoved(QModelIndex, int, int)),
		        this, SLOT(rowsRemoved(QModelIndex, int, int)));

		connect(model_, SIGNAL(modelReset()), this, SLOT(reset()));
		// connect(model_, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
	}

	reset();
}

QSize YaStatusBar::sizeHint() const
{
	return QFrame::sizeHint();
	// return minimumSizeHint();
}

QSize YaStatusBar::minimumSizeHint() const
{
	return QFrame::minimumSizeHint();
	// return QSize(32, 32);
}

void YaStatusBar::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);
}

void YaStatusBar::modelDestroyed()
{
	model_ = 0;
	reset();
}

void YaStatusBar::dataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
	Q_UNUSED(topLeft);
	Q_UNUSED(bottomRight);
}

void YaStatusBar::rowsInserted(QModelIndex, int, int)
{
	reset();
}

void YaStatusBar::rowsRemoved(QModelIndex, int, int)
{
	reset();
}

void YaStatusBar::reset()
{
	if (layoutDirty_)
		return;

	setUpdatesEnabled(false);

	if (layout_)
		delete layout_;
	layout_ = new YaStatusBarLayout(actualStatusBar_);
	layout_->setMargin(2);
	layout_->setSpacing(1);

	layoutDirty_ = true;
	QTimer::singleShot(0, this, SLOT(doItemsLayout()));
}

void YaStatusBar::removeInvalidInformers()
{
	QList<YaInformer*> toDelete;
	foreach(YaInformer* informer, informers_) {
		if (!informer->isValid()) {
			toDelete << informer;
		}
	}

	foreach(YaInformer* informer, toDelete) {
		informers_.removeAll(informer);
		delete informer;
	}
}

void YaStatusBar::addInformer(YaInformer* informer)
{
	addInformerHelper(informer);
	persistentInformers_ << informer;
	if (informer->shortcutWidget())
		topLayout_->addWidget(informer->shortcutWidget());
	reset();
}

void YaStatusBar::addInformerHelper(YaInformer* informer)
{
	stack_->addWidget(informer->widget());

	connect(informer, SIGNAL(activated()), SLOT(informerActivated()));
	connect(informer, SIGNAL(deactivated()), SLOT(informerDeactivated()));
}

void YaStatusBar::addInformerButton(YaInformer* informer)
{
	Q_ASSERT(layout_);
	YaInformerButton* button = informer->button();
	button->setParent(this);
	button->raise();
	layout_->addWidget(button);
	button->show();
}

void YaStatusBar::doItemsLayout()
{
	setUpdatesEnabled(true);

	if (!model_ || !layoutDirty_)
		return;

	layoutDirty_ = false;
	layout_->addWidget(sizeGrip_);

	foreach(YaInformer* informer, persistentInformers_) {
		addInformerButton(informer);
	}

	removeInvalidInformers();
	for (int row = 0; row < model_->rowCount(QModelIndex()); ++row) {
		QModelIndex index = model_->index(row, 0, QModelIndex());

		YaInformer* handler = 0;
		foreach(YaInformer* informer, allInformers()) {
			if (informer->handlesIndex(index)) {
				handler = informer;
				break;
			}
		}

		if (!handler) {
			handler = YaInformer::create(index, this);
			addInformerHelper(handler);
			informers_ << handler;
		}

		addInformerButton(handler);
	}

	// layout_->addStretch();
}

void YaStatusBar::resizeEvent(QResizeEvent* e)
{
	QFrame::resizeEvent(e);

	updateBusyWidget();
}

void YaStatusBar::setController(PsiCon* controller)
{
#ifndef WIDGET_PLUGIN
	controller_ = controller;
	if (controller_) {
		connect(controller_->contactList(), SIGNAL(accountActivityChanged()), SLOT(updateBusyWidget()));
	}
#endif
}

void YaStatusBar::updateBusyWidget()
{
	if (layout_)
		busyWidget_->setGeometry(layout_->contentsRect().adjusted(-2, -2, 2, 4));

	bool active = false;
#ifndef WIDGET_PLUGIN
	if (controller_) {
		active = controller_->contactList()->haveConnectingAccounts();
	}
#endif
	busyWidget_->setVisible(active);
	busyWidget_->setActive(active);
	update();
}

QList<YaInformer*> YaStatusBar::allInformers() const
{
	QList<YaInformer*> result;
	result += persistentInformers_;
	result += informers_;
	return result;
}


void YaStatusBar::informerActivated()
{
	bool updatesEnabled = window()->updatesEnabled();
	window()->setUpdatesEnabled(false);

	YaInformer* informer = static_cast<YaInformer*>(sender());
	foreach(YaInformer* i, allInformers()) {
		i->button()->setChecked(i == informer);
	}

	stack_->setMaximumSize(stack_->maximumSize().width(), informer->desiredHeight());
	stack_->show();
	stack_->setCurrentWidget(informer->widget());

	window()->layout()->activate();
	window()->setUpdatesEnabled(updatesEnabled);
}

void YaStatusBar::informerDeactivated()
{
	bool updatesEnabled = window()->updatesEnabled();
	window()->setUpdatesEnabled(false);

	YaInformer* informer = static_cast<YaInformer*>(sender());
	informer->button()->setChecked(false);

	bool checked = false;
	foreach(YaInformer* i, allInformers()) {
		if (i->button()->isChecked()) {
			checked = true;
			break;
		}
	}

	if (!checked) {
		stack_->hide();
		updateGeometry();
		window()->layout()->activate();
	}

	window()->setUpdatesEnabled(updatesEnabled);
}

void YaStatusBar::setShouldBeVisible(bool shouldBeVisible)
{
	shouldBeVisible_ = shouldBeVisible;
	updateVisibility();
}

void YaStatusBar::updateVisibility()
{
	actualStatusBar_->setVisible(shouldBeVisible_ &&
	                             PsiOptions::instance()->getOption(showStatusBarOptionPath).toBool());
}

void YaStatusBar::optionChanged(const QString& option)
{
	if (option == showStatusBarOptionPath)
		updateVisibility();
}
