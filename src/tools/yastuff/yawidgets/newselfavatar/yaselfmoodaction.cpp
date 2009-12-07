/*
 * yaselfmoodaction.cpp - mood widget buttons
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

#include "yaselfmoodaction.h"

#include <QAction>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QCheckBox>

#include "yaselfmood.h"
#include "yarotation.h"
#include "yavisualutil.h"

static const int pixmapSize = 10;

//----------------------------------------------------------------------------
// YaSelfMoodButton
//----------------------------------------------------------------------------

YaSelfMoodButton::YaSelfMoodButton(QWidget* parent)
	: QAbstractButton(parent)
	, action_(0)
	, helperWidget_(0)
	, rotation_(new YaRotation(this))
	, drawSelection_(true)
	, drawEmpty_(true)
{
	// setAttribute(Qt::WA_OpaquePaintEvent, true);

	connect(rotation_, SIGNAL(rotationChanged()), SLOT(update()));

#ifndef YAPSI_NO_STYLESHEETS
	setStyleSheet(
	"font-family: \"Arial\";"
	"font-size: 10px;"
	// "font-style: italic;"
	"font-weight: bold;"
	);
#endif

	connect(this, SIGNAL(clicked()), SLOT(clicked()));
	setAttribute(Qt::WA_Hover, true);
	hide();
}

YaSelfMoodButton::~YaSelfMoodButton()
{
	delete helperWidget_;
}

bool YaSelfMoodButton::currentAction() const
{
	return action_ && action_->property("current").toBool();
}

QAction* YaSelfMoodButton::action() const
{
	return action_;
}

bool YaSelfMoodButton::isEmpty() const
{
	return currentAction() && action_->text().isEmpty();
}

bool YaSelfMoodButton::isSeparator() const
{
	return action_ && action_->isSeparator();
}

QString YaSelfMoodButton::text() const
{
	if (!action_)
		return QString();
	if (action_->text().isEmpty() && !action_->property("emptyText").toString().isEmpty())
		return action_->property("emptyText").toString();
	if (isEmpty()) {
		// return tr("Choose a mood");
		return "nn";
	}
	return action_->text().simplified();
}

QSize YaSelfMoodButton::sizeHint() const
{
	if (!action_)
		return QSize();

	int width = pixmapSize + YaSelfMoodExtra::margin() + fontMetrics().width(text()) + YaSelfMoodExtra::margin() + 2;
	if (isSeparator())
		return QSize(width, YaSelfMoodExtra::margin());
	return QSize(width, qMax(pixmapSize, fontMetrics().height()));
}

QPixmap YaSelfMoodButton::currentPixmap() const
{
	if (action_) {
		QPixmap statusPixmap = action_->icon().pixmap(pixmapSize, pixmapSize);
		if (!statusPixmap.isNull()) {
			return rotation_->rotatedPixmap(statusPixmap);
		}
	}

	return QPixmap();
}

void YaSelfMoodButton::paintEvent(QPaintEvent*)
{
	if (!action_)
		return;

	QPainter p(this);
	// p.fillRect(rect(), Qt::red);
	if (action_->property("boldFont").toBool()) {
		QFont font = p.font();
		font.setBold(true);
		p.setFont(font);
	}

	if (isSeparator()) {
		p.setPen(Qt::DotLine);
		QRect r(rect());
		int y = r.center().y();
		p.drawLine(r.left(), y, r.right(), y);
		return;
	}

	if (drawSelection_ && testAttribute(Qt::WA_UnderMouse)) {
		p.fillRect(rect(), palette().brush(QPalette::Normal, QPalette::Highlight));
	}

	QPixmap statusPixmap = currentPixmap();
	if (!statusPixmap.isNull()) {
		// p.drawPixmap(pixmapRect_.topLeft(), statusPixmap);
	}
	else if (action_->isCheckable()) {
		if (!helperWidget_)
			helperWidget_ = new QCheckBox(0);

		QStyleOptionButton option;
		option.state |= QStyle::State_Enabled;
		if (action_->isChecked())
			option.state |= QStyle::State_On;

		option.rect = pixmapRect_;
		style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, &p, helperWidget_);
		// style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, &p);
	}

	bool doDraw = !isEmpty() || (isEmpty() && drawEmpty_);
	if (!doDraw)
		return;

	if (isEmpty()) {
		// p.setPen(Qt::gray);
		return;
	}
	// p.drawText(textRect_.adjusted(0, -1, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, p.fontMetrics().elidedText(text(), Qt::ElideRight, textRect_.width()));
	QRect rect = textRect_;
#ifdef Q_WS_MAC
	rect.adjust(0, -2, 0, -2);
#else
	rect.adjust(0, -1, 0, -1);
#endif
	p.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, text());

	if (p.fontMetrics().width(text()) > textRect_.width()) {
		Ya::VisualUtil::drawTextFadeOut(&p, textRect_, Ya::VisualUtil::blueBackgroundColor());
	}
}

void YaSelfMoodButton::resizeEvent(QResizeEvent* event)
{
	QAbstractButton::resizeEvent(event);
	updateRects();
}

void YaSelfMoodButton::updateRects()
{
	pixmapRect_ = rect();
	pixmapRect_.setTop(pixmapRect_.top() + (pixmapRect_.height() - pixmapSize) / 2);
	pixmapRect_.setWidth(pixmapSize);
	pixmapRect_.setHeight(pixmapSize);

	textRect_ = rect();
	textRect_.setLeft(pixmapRect_.right() + YaSelfMoodExtra::margin());
	textRect_.moveTop(textRect_.top() + 2);
}

const QRect& YaSelfMoodButton::pixmapRect() const
{
	return pixmapRect_;
}

const QRect& YaSelfMoodButton::textRect() const
{
	return textRect_;
}

void YaSelfMoodButton::setDrawSelection(bool draw)
{
	drawSelection_ = draw;
	update();
}

void YaSelfMoodButton::setDrawEmpty(bool draw)
{
	drawEmpty_ = draw;
	update();
}

void YaSelfMoodButton::actionEvent(QActionEvent* event)
{
	QAbstractButton::actionEvent(event);
	if (!action_ && event->action()) {
		action_ = event->action();
		connect(action_, SIGNAL(changed()), SLOT(actionChanged()));
		actionChanged();
	}
	update();
}

bool YaSelfMoodButton::actionVisible() const
{
	return action_ && action_->isVisible();
}

void YaSelfMoodButton::actionChanged()
{
	// qWarning("isVisible %d", action_->isVisible());
}

void YaSelfMoodButton::clicked()
{
	if (action_)
		action_->trigger();
}

void YaSelfMoodButton::enableRotation(bool enable)
{
	// rotation_->setEnabled(enable);
}

//----------------------------------------------------------------------------
// YaSelfMoodActionGroup
//----------------------------------------------------------------------------

YaSelfMoodActionGroup::YaSelfMoodActionGroup(QObject* parent)
	: QObject(parent)
{
}

YaSelfMoodActionGroup::~YaSelfMoodActionGroup()
{
	qDeleteAll(actions_);
	actions_.clear();
}

QList<QAction*> YaSelfMoodActionGroup::actions() const
{
	return actions_;
}

void YaSelfMoodActionGroup::addAction(QAction* action)
{
	// Q_ASSERT(!actions_.contains(action));
	actions_.append(action);
	emit actionsChanged();
}

void YaSelfMoodActionGroup::removeAction(QAction* action)
{
	// Q_ASSERT(actions_.contains(action));
	actions_.removeAll(action);
	emit actionsChanged();
}

void YaSelfMoodActionGroup::clear()
{
	qDeleteAll(actions_);
	actions_.clear();
	emit actionsChanged();
}
