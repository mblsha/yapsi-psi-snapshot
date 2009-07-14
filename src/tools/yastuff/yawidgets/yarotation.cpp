/*
 * yarotation.cpp
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

#include "yarotation.h"

#include <QPainter>
#include <QTimer>

#include "pixmaputil.h"

YaRotation::YaRotation(QObject* parent)
	: QObject(parent)
	, rotationTimer_(0)
	, rotation_(0)
{
	rotationTimer_ = new QTimer(this);
	rotationTimer_->setSingleShot(false);
	rotationTimer_->setInterval(100);
	connect(rotationTimer_, SIGNAL(timeout()), SLOT(updateRotation()));
}

YaRotation::~YaRotation()
{
}

bool YaRotation::isEnabled() const
{
	return rotationTimer_->isActive();
}

void YaRotation::setEnabled(bool enable)
{
	if (enable)
		rotationTimer_->start();
	else
		rotationTimer_->stop();
	emit rotationChanged();
}

void YaRotation::updateRotation()
{
	rotation_ += 20;
	if (rotation_ >= 360)
		rotation_ = 0;
	emit rotationChanged();
}

const QPixmap& YaRotation::rotatedPixmap(const QPixmap& sourcePixmap) const
{
	if (!isEnabled() || !rotation_)
		return sourcePixmap;

	Q_ASSERT(isEnabled() && rotation_);
	pixmapCache_ = PixmapUtil::rotatedPixmap(rotation_, sourcePixmap);
	return pixmapCache_;
}
