/*
 * yachatcontactinfo.cpp
 * Copyright (C) 2009  Yandex LLC (Michail Pishchagin)
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

#include "yachatcontactinfo.h"

#include <QDir>
#include <QPainter>

#include "yavisualutil.h"
#include "yawindow.h"
#include "yawindowtheme.h"

static QPixmap yaChatContactInfoBackground()
{
	static QPixmap pix;
	if (pix.isNull()) {
		pix = QPixmap(":images/chat/profile_background.png");
	}
	Q_ASSERT(!pix.isNull());
	return pix;
}

//----------------------------------------------------------------------------
// YaChatContactInfoExtra
//----------------------------------------------------------------------------

QList<QPixmap> YaChatContactInfoExtra::frames_;

YaChatContactInfoExtra::YaChatContactInfoExtra(QWidget *parent)
	: QAbstractButton(parent)
	, currentFrame_(0)
	, mode_(YaChatContactInfoExtra::Compact)
{
	setAttribute(Qt::WA_Hover, true);
	setCursor(Qt::PointingHandCursor);

	ensureFrames();
	Q_ASSERT(!frames_.isEmpty());

	animationTimer_ = new QTimer(this);
	animationTimer_->setSingleShot(false);
	animationTimer_->setInterval(20);
	connect(animationTimer_, SIGNAL(timeout()), SLOT(animate()));
}

QSize YaChatContactInfoExtra::sizeHint() const
{
	if (mode_ == Compact) {
		return frames_.first().size();
	}

	QFont font = this->font();
	font.setPixelSize(12);
	QFontMetrics fm(font);

	QSize sh = frames_.first().size();
	sh.setWidth(sh.width() + fm.width(text()) + 5 + 3 + 5);
	sh.setHeight(yaChatContactInfoBackground().height());
	return sh;
}

void YaChatContactInfoExtra::ensureFrames()
{
	if (!frames_.isEmpty())
		return;

	QDir dir(":images/chat/info_button");
	foreach(QString file, dir.entryList()) {
		frames_ << QPixmap(dir.absoluteFilePath(file));
	}
}

void YaChatContactInfoExtra::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	// p.fillRect(rect(), Qt::blue);

	int x = 0;
	int y = (height() - frames_[currentFrame_].height()) / 2;

	if (mode() == Button) {
		YaWindow* yaWindow = dynamic_cast<YaWindow*>(window());
		Q_ASSERT(yaWindow);

		QPixmap profileBackground = yaWindow->theme().theme().profileBackground();

		p.setOpacity(0.6 + (0.4 * float(currentFrame_) / float(frames_.count() - 1)));
		QRect r(rect());

		QRect backgroundRect(r);
		p.drawPixmap(backgroundRect.topLeft(), profileBackground);
		backgroundRect.setLeft(backgroundRect.left() + profileBackground.width());
		p.fillRect(backgroundRect, yaWindow->theme().theme().profileBackgroundColor());

		QFont font = p.font();
		font.setPixelSize(12);
		p.setFont(font);

		p.setPen(yaWindow->theme().theme().profileTextColor());

		r.setLeft(frames_.first().width() + 5);
		int flags = Qt::AlignVCenter | Qt::AlignLeft;
		p.drawText(r, flags, text());
		// Ya::VisualUtil::drawTextDashedUnderline(&p, text(), r.adjusted(2, 0, 2, 0), flags);

		x += 5;
		y += 1;
	}

	p.setOpacity(1.0);
	p.drawPixmap(x, y, frames_[currentFrame_]);
}

void YaChatContactInfoExtra::enterEvent(QEvent* event)
{
	QAbstractButton::enterEvent(event);
	animationTimer_->start();
}

void YaChatContactInfoExtra::leaveEvent(QEvent* event)
{
	QAbstractButton::leaveEvent(event);
	animationTimer_->start();
}

void YaChatContactInfoExtra::contextMenuEvent(QContextMenuEvent* event)
{
	Q_UNUSED(event);
	emit alternateClicked();
}

void YaChatContactInfoExtra::animate()
{
	int val = currentFrame_;
	if (underMouse())
		++val;
	else
		--val;
	int newVal = qMax(0, qMin(val, frames_.count() - 1));
	if (newVal != currentFrame_) {
		currentFrame_ = newVal;
		update();
	}
	else {
		animationTimer_->stop();
	}
}

YaChatContactInfoExtra::Mode YaChatContactInfoExtra::mode() const
{
	return mode_;
}

void YaChatContactInfoExtra::setMode(YaChatContactInfoExtra::Mode mode)
{
	mode_ = mode;
	setSizePolicy(mode_ == Compact ? QSizePolicy::Fixed : QSizePolicy::Maximum,
	              mode_ == Compact ? QSizePolicy::Fixed : QSizePolicy::Expanding);
	setText(mode_ == Compact ? QString() : tr("Profile"));
	updateGeometry();
}

//----------------------------------------------------------------------------
// YaChatContactInfo
//----------------------------------------------------------------------------

YaChatContactInfo::YaChatContactInfo(QWidget *parent)
	: OverlayWidget<QFrame, YaChatContactInfoExtra>(parent,
		new YaChatContactInfoExtra(parent->window()->objectName() == "MainWindow" ? parent->window() : parent->parentWidget()))
{
	window()->installEventFilter(this);
	connect(extra(), SIGNAL(clicked()), SIGNAL(clicked()));
	connect(extra(), SIGNAL(alternateClicked()), SIGNAL(alternateClicked()));
}

QRect YaChatContactInfo::extraGeometry() const
{
	extra()->setVisible(true);

	int ox = 0;
	int oy = 0;

	if (window()->objectName() == "MainWindow") {
		ox = 6;
		oy = 13;
	}

	QSize sh = extra()->sizeHint();
	QRect result(globalRect().x() - 19 + ox,
	             globalRect().y() - sh.height() + 6 + oy,
	             sh.width(),
	             sh.height());

	return result;
}

QSize YaChatContactInfo::sizeHint() const
{
	return minimumSizeHint();
}

QSize YaChatContactInfo::minimumSizeHint() const
{
	return QSize(1, 1);
}

void YaChatContactInfo::paintEvent(QPaintEvent*)
{
	// QPainter p(this);
	// p.fillRect(rect(), Qt::red);
}

bool YaChatContactInfo::eventFilter(QObject* obj, QEvent* e)
{
	if ((e->type() == QEvent::Move || e->type() == QEvent::Resize) && obj == window()) {
		moveExtra();
	}

	return OverlayWidget<QFrame, YaChatContactInfoExtra>::eventFilter(obj, e);
}
