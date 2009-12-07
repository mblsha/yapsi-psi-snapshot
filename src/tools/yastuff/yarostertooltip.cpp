/*
 * yarostertooltip.cpp - per-contact tooltips
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

#include "yarostertooltip.h"

#include "yarostertiplabel.h"
#include "psicontact.h"
#include "psiaccount.h"

//----------------------------------------------------------------------------
// YaRosterToolTipPosition
//----------------------------------------------------------------------------

QPoint YaRosterToolTipPosition::initialPoint(const QWidget* label) const
{
	const YaRosterTipLabel* tipLabel = dynamic_cast<const YaRosterTipLabel*>(label);
	Q_ASSERT(tipLabel);
	return globalRect.topRight() - QPoint(0, tipLabel->property("firstButtonTop").toInt());
}

QPoint YaRosterToolTipPosition::calculateTipPosition(const QWidget* label, const QSize& size) const
{
	QRect screen = screenRect();

	QPoint p = initialPoint(label);
	p.rx() = globalRect.left() - size.width();
	if (p.x() < screen.x())
		p.rx() = globalRect.right();
	if (p.x() + size.width() > screen.right())
		p.rx() = screen.right() - size.width();
	if (p.y() + size.height() > screen.bottom())
		p.ry() = screen.bottom() - size.height();
	if (p.y() < screen.y())
		p.ry() = screen.y();

	return p;
}

//----------------------------------------------------------------------------
// YaRosterToolTip
//----------------------------------------------------------------------------

QRect YaRosterToolTip::currentRect() const
{
	if (isVisible())
		return lastRect_;
	return QRect();
}

void YaRosterToolTip::hide()
{
	doShowText(QPoint(), QString(), 0);

	lastContact_ = 0;
	lastWidget_ = 0;
	contactSelection_ = 0;
}

QString YaRosterToolTip::forceTipLabelUpdateString()
{
	return "YaForceTipLabelUpdateString@";
}

void YaRosterToolTip::showText(QRect rect, PsiContact* contact, const QWidget* w, QMimeData* contactSelection)
{
	Q_ASSERT(!rect.isNull());
	Q_ASSERT(contact);
	if (!contact)
		return;

	lastRect_ = rect;
	Q_ASSERT(!lastContact_);
	lastContact_ = contact;
	Q_ASSERT(!lastWidget_);
	lastWidget_ = (QWidget*)w;
	Q_ASSERT(!contactSelection_);
	contactSelection_ = contactSelection;

	QString text = contact ? contact->account()->id() + ":" + contact->jid().full() : QString();
	doShowText(rect.topLeft(), text, w);

	lastContact_ = 0;
	lastWidget_ = 0;
	delete contactSelection_;
	contactSelection_ = 0;
}

ToolTipPosition* YaRosterToolTip::createTipPosition(const QPoint&, const QWidget* parentWidget)
{
	return new YaRosterToolTipPosition(lastRect_, parentWidget);
}

PsiTipLabel* YaRosterToolTip::createTipLabel(const QString& text, QWidget* parent)
{
	YaRosterTipLabel* label = new YaRosterTipLabel(parent);
	registerTipLabel(label);
	label->init(text);
	updateLabel(label, text);
	return label;
}

void YaRosterToolTip::registerTipLabel(YaRosterTipLabel* tipLabel)
{
	connect(tipLabel, SIGNAL(removeContact(PsiContact*, QMimeData*)), SIGNAL(removeContact(PsiContact*, QMimeData*)));
	connect(tipLabel, SIGNAL(renameContact(PsiContact*, QMimeData*)), SIGNAL(renameContact(PsiContact*, QMimeData*)));
	connect(tipLabel, SIGNAL(blockContact(PsiContact*, QMimeData*)), SIGNAL(blockContact(PsiContact*, QMimeData*)));
	connect(tipLabel, SIGNAL(unblockContact(PsiContact*, QMimeData*)), SIGNAL(unblockContact(PsiContact*, QMimeData*)));
	connect(tipLabel, SIGNAL(addContact(const XMPP::Jid&, PsiAccount*, const QStringList&, const QString&)), SIGNAL(addContact(const XMPP::Jid&, PsiAccount*, const QStringList&, const QString&)));
	connect(tipLabel, SIGNAL(toolTipEntered(PsiContact*, QMimeData*)), SIGNAL(toolTipEntered(PsiContact*, QMimeData*)));
	connect(tipLabel, SIGNAL(toolTipHidden(PsiContact*, QMimeData*)), SIGNAL(toolTipHidden(PsiContact*, QMimeData*)));
}

YaRosterToolTip *YaRosterToolTip::instance_ = 0;

YaRosterToolTip* YaRosterToolTip::instance()
{
	if (!instance_)
		instance_ = new YaRosterToolTip();
	return instance_;
}

YaRosterToolTip::YaRosterToolTip()
	: PsiToolTip()
	, lastContact_(0)
	, lastWidget_(0)
	, contactSelection_(0)
{
}

bool YaRosterToolTip::moveAndUpdateTipLabel(PsiTipLabel* label, const QString& text)
{
	Q_UNUSED(text);

	YaRosterTipLabel* tipLabel = dynamic_cast<YaRosterTipLabel*>(label);
	if (tipLabel) {
		if (tipLabel->leaveTimerPermanentlyDisabled()) {
			return false;
		}
	}

	return isVisible();
}

void YaRosterToolTip::updateTipLabel(PsiTipLabel* label, const QString& text, const ToolTipPosition* pos)
{
	updateTipLabelProperties(label);

	Q_UNUSED(pos);
	if (label->theText() == text)
		return;

	YaRosterTipLabel* tipLabel = dynamic_cast<YaRosterTipLabel*>(label);
	if (!tipLabel)
		return;

	updateLabel(tipLabel, text);
}

// ONLINE-2151
bool YaRosterToolTip::preventShow()
{
	if (!mouseOverRect()) {
		hide();
		// ONLINE-2261: We need to process events in order for YaContactListView
		// to update its Qt::WA_UnderMouse attribute
		QCoreApplication::instance()->processEvents();
		emit toolTipHidden(0, 0);
		return true;
	}
	return false;
}

void YaRosterToolTip::updateTipLabelProperties(PsiTipLabel* label)
{
	YaRosterTipLabel* tipLabel = dynamic_cast<YaRosterTipLabel*>(label);
	if (tipLabel) {
		tipLabel->setLeaveTimerPermanentlyDisabled(false);
	}
}

void YaRosterToolTip::updateLabel(YaRosterTipLabel* label, const QString& text)
{
#ifdef Q_WS_WIN
	bool wasVisible = label->isVisible();
	// this hack is required on Windows in order to reduce nasty in-tooltip flicker
	// caused by hiding/showing its internal widgets (while tooltip's updates
	// are explicitly disabled) Qt bug.
	label->hide();
#endif

	label->setUpdatesEnabled(false);
	label->setText(text);
	label->setListView(lastWidget_);
	label->setTipPosition(createTipPosition(QPoint(), lastWidget_));
	label->setContactSelection(contactSelection_);
	label->setContact(lastContact_);
	label->updateSize(true);
	label->setUpdatesEnabled(true);

	contactSelection_ = 0;

#ifdef Q_WS_WIN
	if (wasVisible) {
		label->show();
	}
#endif
}

bool YaRosterToolTip::isVisible() const
{
	return PsiToolTip::isVisible() &&
	       dynamic_cast<YaRosterTipLabel*>(PsiTipLabel::instance());
}

bool YaRosterToolTip::preventAnimation() const
{
	return true;
}

bool YaRosterToolTip::mouseOverRect() const
{
	if (!lastRect_.isNull()) {
		return lastRect_.contains(QCursor::pos());
	}
	return true;
}
