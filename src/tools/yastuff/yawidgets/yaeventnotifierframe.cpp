/*
 * yaeventnotifierframe.cpp
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

#include "yaeventnotifierframe.h"

#include <QPainter>

#include "yaprofile.h"
#include "psievent.h"
#include "psioptions.h"
#include "yacommon.h"
#include "yacontactlabel.h"
#include "yavisualutil.h"
#include "psiaccount.h"
#include "globaleventqueue.h"

YaEventNotifierFrame::YaEventNotifierFrame(QWidget* parent)
	: OverlayWidget<QFrame, YaEventNotifierFrameExtra>(parent,
		new YaEventNotifierFrameExtra(0))
	, profile_(0)
	, tosterMode_(false)
	, id_(-1)
{
	extra()->setParent(this);
	connect(extra(), SIGNAL(clicked()), SIGNAL(closeClicked()));

	setFrameShape(QFrame::StyledPanel);

	ui_.setupUi(this);
	ui_.eventNotifierCount->hide();

	connect(ui_.skipButton, SIGNAL(clicked()), SIGNAL(skip()));
	connect(ui_.readButton, SIGNAL(clicked()), SIGNAL(read()));
	connect(ui_.declineButton, SIGNAL(clicked()), SLOT(decline()));
	connect(ui_.acceptButton, SIGNAL(clicked()), SLOT(accept()));
	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));

	foreach(QWidget* w, findChildren<QWidget*>()) {
		w->installEventFilter(this);
	}

	setBackground();
}

/**
 * Overriding the paint-nothing paintEvent in OverlayWidget.
 */
void YaEventNotifierFrame::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.fillRect(rect(), backgroundBrush());
}

/**
 * Eat the mouse clicks outside of buttons when buttons are visible.
 */
void YaEventNotifierFrame::mousePressEvent(QMouseEvent* e)
{
	if (ui_.buttonsFrame->isVisible())
		return;
	OverlayWidget<QFrame, YaEventNotifierFrameExtra>::mousePressEvent(e);
}

void YaEventNotifierFrame::optionChanged(const QString& opt)
{
	Q_UNUSED(opt);
}

QRect YaEventNotifierFrame::extraGeometry() const
{
	// extra()->hide();
	const QSize sh = extra()->sizeHint();
	return QRect(globalRect().x() + width() - sh.width() - 4,
	             globalRect().y() + 6,
	             sh.width(), sh.height());
}

void YaEventNotifierFrame::setEventCounter(int skipped, int total)
{
	bool hide = !skipped && !total;
	ui_.eventNotifierCount->setText(
		QString::fromUtf8("%1 of %2")
			.arg(skipped)
			.arg(total));
	if (hide)
		ui_.eventNotifierCount->setText(QString());
}

int YaEventNotifierFrame::id() const
{
	return id_;
}

void YaEventNotifierFrame::setEvent(int id, PsiEvent* event)
{
	id_ = id;
	updateProfile(event);
	updateButtons(event);
}

void YaEventNotifierFrame::setTosterMode(bool enable)
{
	tosterMode_ = enable;

	// if we won't do this, single clicks won't be passed to the YaPopupNotificator.
	ui_.eventNotifierMessage->setTextInteractionFlags(
	    tosterMode_ ? Qt::NoTextInteraction : Qt::LinksAccessibleByMouse);
}

void YaEventNotifierFrame::updateProfile(PsiEvent* event)
{
	if (profile_) {
		delete profile_;
		profile_ = 0;
	}

	QString message;
	if (event) {
		if (event->account())
			profile_ = YaProfile::create(event->account(), event->from());
		message = event->description();

		if (event->type() == PsiEvent::Message && tosterMode_) {
			message = Ya::messageNotifierText(message);
		}
	}

	ui_.eventNotifierUserpic->setProfile(profile_);
	ui_.eventNotifierContactName->setProfile(profile_);
	ui_.eventNotifierContactName->setIsEventNotifierLabel(true);
	ui_.eventNotifierMessage->setMessage(message);
}

// TODO: grab event clicks if we're currently on auth-subscription event

void YaEventNotifierFrame::updateButtons(PsiEvent* event)
{
	setUpdatesEnabled(false);
	bool showButtons = false;
	if (event) {
		setBackground(Ya::isYaInformer(event) ? YaEventNotifierFrame::Informer : YaEventNotifierFrame::Notifier);

		int index = Ya::isSubscriptionRequest(event) ? 1 : 0;
		QList<QStackedWidget*> stacks;
		stacks << ui_.leftStack << ui_.rightStack;
		foreach(QStackedWidget* stack, stacks)
			stack->setCurrentIndex(index);
		// ui_.skipButton->hide();

		showButtons = !tosterMode_ || Ya::isSubscriptionRequest(event);
	}

	ui_.buttonsFrame->setVisible(showButtons);
	ui_.spacerFrame->setVisible(!showButtons); // to make notification text to be central vertical aligned
	setUpdatesEnabled(true);
}

QBrush YaEventNotifierFrame::backgroundBrush() const
{
	return backgroundBrush_;
}

void YaEventNotifierFrame::setBackground(YaEventNotifierFrame::Background background)
{
	backgroundBrush_ = (background == Informer) ? Ya::VisualUtil::highlightBackgroundColor() : Ya::VisualUtil::downlightColor();
	update();

	QString informer;
	if (background == Informer)
		informer = "_informer";

	// QPalette pal = palette();
	// pal.setBrush(QPalette::Window, backgroundBrush_);
	// setPalette(pal);
#ifndef YAPSI_NO_STYLESHEETS
	setStyleSheet(QString(
	"YaEventNotifierFrame {"
	"border-image: url(:/images/eventnotifier%1.png) 4px 5px 4px 5px;"
	"}"
	).arg(informer));

	extra()->setStyleSheet(QString(
	"border-style: none;"
	"border-image: none;"
	"border: 0px;"
	"background-image: url(:/images/eventnotifier_close%1.png);"
	).arg(informer));
#endif
}

static void doAuth(PsiAccount* account, const XMPP::Jid& jid) { account->dj_auth(jid); }
static void doDeny(PsiAccount* account, const XMPP::Jid& jid) { account->dj_deny(jid); }

static void processAuthEvent(PsiEvent* event, void (*func)(PsiAccount*, const XMPP::Jid&))
{
	if (!event)
		return;
	Q_ASSERT(event->type() == PsiEvent::Auth);
	QPointer<PsiEvent> e = event;
	func(event->account(), event->jid());
	if (e) {
			e->account()->eventQueue()->dequeue(e);
			delete e;
	}
}

void YaEventNotifierFrame::decline()
{
	processAuthEvent(currentEvent(), doDeny);
}

void YaEventNotifierFrame::accept()
{
	processAuthEvent(currentEvent(), doAuth);
}

PsiEvent* YaEventNotifierFrame::currentEvent() const
{
	if (!GlobalEventQueue::instance()->ids().contains(id_))
		return 0;

	return GlobalEventQueue::instance()->peek(id_);
}

XMPP::Jid YaEventNotifierFrame::currentJid() const
{
	if (profile_)
		return profile_->jid();
	return XMPP::Jid();
}

bool YaEventNotifierFrame::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::MouseButtonPress) {
		QMouseEvent* me = static_cast<QMouseEvent*>(e);
		if (me->button() == Qt::LeftButton &&
		    !ui_.buttonsFrame->isVisible() &&
		    obj != extra())
		{
			emit clicked();
			return true;
		}
	}

	return OverlayWidget<QFrame, YaEventNotifierFrameExtra>::eventFilter(obj, e);
}
