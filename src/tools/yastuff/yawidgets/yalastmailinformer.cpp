/*
 * yalastmailinformer.cpp
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

#include "yalastmailinformer.h"

#include <QPainter>

#include "psicon.h"
#include "yavisualutil.h"
#include "yalastmailinformermodel.h"
#include "desktoputil.h"

YaLastMailInformer::YaLastMailInformer(QObject* parent)
	: YaInformer(QModelIndex(), parent)
{
	setActivateShortcut("appwide.activate-lastmail-informer");

	model_ = new YaLastMailInformerModel(this);

	widget_ = new QFrame(0);
	widget_->setFrameShape(QFrame::NoFrame);
	widget_->installEventFilter(this);

	ui_.setupUi(widget_);
	ui_.treeView->setMinimumSize(30, 30);
	connect(ui_.treeView, SIGNAL(clicked(const QModelIndex&)), SLOT(itemClicked(const QModelIndex&)));

	connect(ui_.readAllButton, SIGNAL(clicked()), SLOT(readAll()));

	ui_.treeView->setModel(model_);
}

YaLastMailInformer::~YaLastMailInformer()
{
	if (!widget_.isNull())
		delete widget_;
}

void YaLastMailInformer::setController(PsiCon* psi)
{
	psi_ = psi;
	Q_ASSERT(psi_);

	model_->setContactList(psi_->contactList());
	connect(model_, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(itemCountChanged()));
	connect(model_, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(itemCountChanged()));
}

QWidget* YaLastMailInformer::widget()
{
	return widget_;
}

bool YaLastMailInformer::isPersistent() const
{
	return true;
}

QString YaLastMailInformer::jid() const
{
	return "lastmail.ya.ru";
}

QIcon YaLastMailInformer::icon() const
{
	return Ya::VisualUtil::rosterStatusPixmap(XMPP::Status::Offline);
}

bool YaLastMailInformer::isOnline() const
{
	return true;
}

QString YaLastMailInformer::value() const
{
	return QString::number(model_->rowCount());
}

bool YaLastMailInformer::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == widget_ && e->type() == QEvent::Paint) {
		QPainter p(widget_);
		p.fillRect(widget_->rect(), backgroundBrush());
		return true;
	}

	return YaInformer::eventFilter(obj, e);
}

void YaLastMailInformer::itemCountChanged()
{
	updateButton();
}

void YaLastMailInformer::itemClicked(const QModelIndex& index)
{
	DesktopUtil::openYaUrl(model_->getMailUrlFor(index));
	model_->removeRow(index.row(), index.parent());
}

void YaLastMailInformer::readAll()
{
	DesktopUtil::openYaUrl(model_->getIncomingMailUrl());
	model_->clear();
}
