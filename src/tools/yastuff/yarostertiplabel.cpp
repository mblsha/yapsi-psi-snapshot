/*
 * yarostertiplabel.cpp - widget that shows contact tooltips
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

#include "yarostertiplabel.h"

#include <QEvent>
#include <QDesktopServices>
#include <QTimer>
#include <QPainter>
#include <QTextDocument> // for Qt::escape()
#include <QKeyEvent>
#include <QClipboard>
#include <QDesktopWidget>
#include <QTextFrameFormat>
#include <QMenu>

#include "yaprofile.h"
#include "psiaccount.h"
#include "psicontact.h"
#include "psiiconset.h"
#include "xmpp_vcard.h"
#include "xmpp_tasks.h"
#include "vcardfactory.h"
#include "userlist.h"
#include "psioptions.h"
#include "yacommon.h"
#include "yacontactlabel.h"
#include "yavisualutil.h"
#include "lastactivitytask.h"
#include "yarostertooltip.h"
#include "yaprivacymanager.h"
#include "psiselfcontact.h"

//----------------------------------------------------------------------------
// YaRosterTipLabelLeaveTimer
//----------------------------------------------------------------------------

YaRosterTipLabelLeaveTimer::YaRosterTipLabelLeaveTimer(QWidget* parent)
	: QObject(parent)
	, parent_(parent)
	, listView_(0)
	, enabled_(true)
	, permanentlyDisabled_(false)
{
}

QWidget* YaRosterTipLabelLeaveTimer::listView() const
{
	return listView_;
}

bool YaRosterTipLabelLeaveTimer::isEnabledWithoutConsideringPermanentlyDisabledState() const
{
	return enabled_;
}

bool YaRosterTipLabelLeaveTimer::isEnabled() const
{
	return enabled_ && !permanentlyDisabled_;
}

void YaRosterTipLabelLeaveTimer::setEnabled(bool enabled)
{
	enabled_ = enabled;
}

bool YaRosterTipLabelLeaveTimer::permanentlyDisabled() const
{
	return permanentlyDisabled_;
}

void YaRosterTipLabelLeaveTimer::setPermanentlyDisabled(bool permanentlyDisabled)
{
	permanentlyDisabled_ = permanentlyDisabled;
}

void YaRosterTipLabelLeaveTimer::setListView(QWidget* listView)
{
	listView_ = listView;
	if (listView_ && listView_->underMouse()) {
		stopLeaveTimer();
	}
}

bool YaRosterTipLabelLeaveTimer::processEvent(QObject* obj, QEvent* e, bool* result)
{
	Q_ASSERT(result);
	*result = false;

	QList<QObject*> leaveObjects;
	leaveObjects << parent_;
	if (listView_)
		leaveObjects << listView_;

	QWidget* widget = 0;
	if (obj->isWidgetType())
		widget = static_cast<QWidget*>(obj);

	bool childWidget = widget && widget->parentWidget() && (widget->parentWidget()->window() == parent_);

	switch (e->type()) {
	case QEvent::ContextMenu:
		if (childWidget) {
			// we don't want context menus in QTextEdits as it's currently (Qt 4.2.3) crash prone
			*result = true;
			return true;
		}
		break;
	case QEvent::FocusIn:
		if (leaveObjects.contains(obj) || childWidget)
			stopLeaveTimer();
		return true;
	case QEvent::FocusOut:
		if (leaveObjects.contains(obj))
			startLeaveTimer();
		return true;
	case QEvent::WindowActivate:
		if (leaveObjects.contains(obj) || childWidget)
			stopLeaveTimer();
		return true;
	case QEvent::WindowDeactivate:
		if (leaveObjects.contains(obj))
			startLeaveTimer();
		return true;
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
		if (widget == parent_ || childWidget) {
			return true;
		}
		break;
	case QEvent::Leave:
		if (leaveObjects.contains(obj)) {
			startLeaveTimer();
		}
		return true;
	case QEvent::Enter:
		if (leaveObjects.contains(obj) || childWidget) {
			stopLeaveTimer();
		}
		return true;
	default:
		;
	}

	return false;
}

void YaRosterTipLabelLeaveTimer::startLeaveTimer()
{
	leaveTimer_.start(500, this);
}

void YaRosterTipLabelLeaveTimer::stopLeaveTimer()
{
	leaveTimer_.stop();
}

void YaRosterTipLabelLeaveTimer::timerEvent(QTimerEvent* e)
{
	if (e->timerId() == leaveTimer_.timerId()) {
		if (parent_->windowFlags() & Qt::Window) {
			if (qApp->activeWindow() != parent_->window()) {
				setPermanentlyDisabled(false);
			}
		}
		if (isEnabled()) {
			emit hideTip();
		}
		return;
	}

	QObject::timerEvent(e);
}

//----------------------------------------------------------------------------
// YaRosterTipLabel
//----------------------------------------------------------------------------

YaRosterTipLabel::YaRosterTipLabel(QWidget* parent, bool isHelperTipLabel)
	: PsiTipLabel(parent, isHelperTipLabel)
	, tipPosition_(0)
	, contact_(0)
	, contactSelection_(0)
	, profile_(0)
	, initialized_(false)
	, deleteTimer_(0)
	, updateGeometryTimer_(0)
	, leaveTimer_(new YaRosterTipLabelLeaveTimer(this))
	, copyJidAction_(0)
	, composeEmailAction_(0)
	, statusTextInvisible_(false)
	, statusTextEscape_(false)
{
	connect(leaveTimer_, SIGNAL(hideTip()), SLOT(hideTip()));
	installEventFilter(this);

	deleteTimer_ = new QTimer(this);
	deleteTimer_->setSingleShot(true);
	deleteTimer_->setInterval(1000);
	connect(deleteTimer_, SIGNAL(timeout()), SLOT(hideTip()));

	updateGeometryTimer_ = new QTimer(this);
	updateGeometryTimer_->setSingleShot(true);
	updateGeometryTimer_->setInterval(0);
	connect(updateGeometryTimer_, SIGNAL(timeout()), SLOT(applyNewGeometry()));

	copyJidAction_ = new QAction(tr("Copy"), this);
	connect(copyJidAction_, SIGNAL(triggered()), SLOT(copyJid()));
	composeEmailAction_ = new QAction(tr("Compose E-mail..."), this);
	connect(composeEmailAction_, SIGNAL(triggered()), SLOT(composeEmail()));

	jidMenu_ = new QMenu(this);
	// menu.setWindowFlags(Qt::Popup | Qt::WindowStaysOnTopHint);
	jidMenu_->setWindowFlags(Qt::ToolTip | Qt::WindowStaysOnTopHint);
	jidMenu_->addAction(copyJidAction_);
	// jidMenu_->addAction(composeEmailAction_);
}

YaRosterTipLabel::~YaRosterTipLabel()
{
	setContactSelection(0);
	setTipPosition(0);
	delete profile_;
}

void YaRosterTipLabel::setTipPosition(ToolTipPosition* tipPosition)
{
	if (tipPosition_)
		delete tipPosition_;
	tipPosition_ = tipPosition;
}

void YaRosterTipLabel::setContact(PsiContact* contact)
{
	if (contact_ == contact)
		return;

	contact_ = contact;
	if (profile_)
		delete profile_;
	profile_ = YaProfile::create(contact->account(), contact->jid());
	invalidateData();
}

void YaRosterTipLabel::setContactSelection(QMimeData* contactSelection)
{
	if (contactSelection_)
		delete contactSelection_;
	contactSelection_ = contactSelection;
}

void YaRosterTipLabel::setListView(QWidget* listView)
{
	leaveTimer_->setListView(listView);
}

bool YaRosterTipLabel::selfContact() const
{
	return contact_ && dynamic_cast<PsiSelfContact*>(contact_);
}

void YaRosterTipLabel::invalidateData()
{
	deleteTimer_->stop();

	if (!initialized_)
		return;

	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	Q_ASSERT(contact_);
	Q_ASSERT(profile_);

	YaPrivacyManager* privacyManager = dynamic_cast<YaPrivacyManager*>(profile_->account()->privacyManager());
	Q_ASSERT(privacyManager);
	disconnect(privacyManager, 0, this, 0);
	connect(privacyManager, SIGNAL(availabilityChanged()), SLOT(updateActionButtons()));
	connect(privacyManager, SIGNAL(listChanged(const QStringList&)), SLOT(updateActionButtons()));

	disconnect(profile_->account(), 0, this, 0);
	connect(profile_->account(), SIGNAL(updateContact(const Jid&)), SLOT(updateContact(const Jid&)));
	connect(profile_->account(), SIGNAL(updatedActivity()), SLOT(updateActionButtons()));
	connect(contact_, SIGNAL(updated()), SLOT(updateActionButtons()));
	updateActionButtons();

	ui_.historyButton->setEnabled(contact_->historyAvailable());

	ui_.chatButton->setVisible(!selfContact());
	ui_.emailButton->setVisible(!selfContact());
	// ui_.historyButton->setVisible(!selfContact());
	ui_.renameButton->setVisible(!selfContact());
	ui_.renameButton->setEnabled(contact_->isEditable());

	ui_.contactName->setFixedHeight(15);
	ui_.contactName->setBackgroundColor(backgroundColor());
	ui_.contactName->setProfile(profile_);
	ui_.contactUserpic->setProfile(profile_);
	ui_.jidIconLabel->setPixmap(QPixmap(":/images/jabber.png"));
	ui_.jidLabel->setText(contact_->jid().full());
	ui_.jidLabel->setTextInteractionFlags(Qt::NoTextInteraction);
	ui_.infoLabel->setBackgroundColor(backgroundColor());

	setStatusText(contact_->status().status());
	ui_.profileButton->setEnabled(contact_->isYaJid() || contact_->isYandexTeamJid());
	ui_.photosButton->setEnabled(contact_->isYaJid());

	if (!lastActivityTask_.isNull()) {
		delete lastActivityTask_;
	}

	if (!selfContact() &&
	    !isHelperTipLabel() &&
	    profile_->account()->isAvailable() &&
	    contact_->status().type() == XMPP::Status::Offline) {
		setLastAvailable(QDateTime::currentDateTime(), true);
		LastActivityTask* jtLast = new LastActivityTask(profile_->jid().bare(), profile_->account()->client()->rootTask());
		Q_ASSERT(lastActivityTask_.isNull());
		lastActivityTask_ = jtLast;
		connect(jtLast, SIGNAL(finished()), SLOT(lastAvailableFinished()));
		jtLast->go(true);
	}

	const VCard *vcard = VCardFactory::instance()->vcard(profile_->jid());
	if (vcard) {
		setVCard(*vcard);
	}
	else {
		setVCard(XMPP::VCard());
	}

	{
		if (!vcardTask_.isNull())
			disconnect(vcardTask_, 0, this, SLOT(vcardFinished()));
		vcardTask_ = 0;
		if (!isHelperTipLabel() && profile_->account()->isAvailable()) {
			XMPP::JT_VCard* jtVCard = VCardFactory::instance()->getVCard(profile_->jid(),
			                          profile_->account()->client()->rootTask(),
			                          this, SLOT(vcardFinished()), true);
			vcardTask_ = jtVCard;
		}
	}

	dataInvalidated();

	setUpdatesEnabled(updatesEnabled);
	updateSize();
}

void YaRosterTipLabel::dataInvalidated()
{
}

void YaRosterTipLabel::initUi()
{
	ui_.setupUi(this);
	ui_.closeButton->setType(YaWindowExtraButton::Close);
	connect(ui_.closeButton, SIGNAL(clicked()), SLOT(closeClicked()));

	connect(ui_.chatButton, SIGNAL(clicked()), SLOT(openChat()));
	connect(ui_.emailButton, SIGNAL(clicked()), SLOT(composeEmail()));
	connect(ui_.profileButton, SIGNAL(clicked()), SLOT(openYaProfile()));
	connect(ui_.historyButton, SIGNAL(clicked()), SLOT(openHistory()));
	connect(ui_.photosButton, SIGNAL(clicked()), SLOT(openPhotos()));
	connect(ui_.addButton, SIGNAL(clicked()), SLOT(addContact()));
	connect(ui_.authButton, SIGNAL(clicked()), SLOT(authContact()));
	connect(ui_.renameButton, SIGNAL(clicked()), SLOT(renameContact()));
	connect(ui_.deleteButton, SIGNAL(clicked()), SLOT(deleteContact()));
	connect(ui_.blockButton, SIGNAL(clicked()), SLOT(blockContact()));
	ui_.deleteButton->setButtonStyle(YaPushButton::ButtonStyle_Destructive);
	ui_.blockButton->setButtonStyle(YaPushButton::ButtonStyle_Destructive);

	ui_.statusTypeDescriptionLabel->setFrameShape(QFrame::NoFrame);
	ui_.statusTypeDescriptionLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui_.statusTypeDescriptionLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QPalette pal = ui_.statusTypeDescriptionLabel->palette();
	pal.setColor(QPalette::Background, backgroundColor());
	pal.setColor(QPalette::Base, backgroundColor());
	ui_.statusTypeDescriptionLabel->setPalette(pal);

	YaPushButton::initAllButtons(this);
	initialized_ = true;
}

void YaRosterTipLabel::updateActionButtons()
{
	ui_.addButton->setVisible(contact_->addAvailable());
	ui_.deleteButton->setVisible(contact_->removeAvailable());
	ui_.authButton->setVisible(contact_->authAvailable());
	ui_.blockButton->setVisible(contact_->blockAvailable());
	YaPrivacyManager* privacyManager = dynamic_cast<YaPrivacyManager*>(contact_->account()->privacyManager());
	Q_ASSERT(privacyManager);
	ui_.blockButton->setEnabled(privacyManager->isAvailable());
	ui_.blockButton->setText(contact_->isBlocked() ?
	                         tr("Unblock") : tr("Block"));
	updateSize();
}

void YaRosterTipLabel::addContact()
{
	closeClicked();
	emit addContact(contact_->jid(), contact_->account(), QStringList(), QString());
}

void YaRosterTipLabel::authContact()
{
	closeClicked();
	contact_->rerequestAuthorizationFrom();
}

void YaRosterTipLabel::renameContact()
{
	deleteTimer_->start();
	emit renameContact(contact_, contactSelection_);
}

void YaRosterTipLabel::deleteContact()
{
	deleteTimer_->start();
	emit removeContact(contact_, contactSelection_);
}

void YaRosterTipLabel::blockContact()
{
	closeClicked();
	contact_->toggleBlockedState();
}

int YaRosterTipLabel::firstButtonTop() const
{
	QPoint globalPos = ui_.frame_2->mapToGlobal(ui_.frame_2->rect().topLeft());
	return mapFromGlobal(globalPos).y();
}

void YaRosterTipLabel::setStatusText(QString text, bool invisible, bool escape)
{
	statusText_ = text;
	statusTextInvisible_ = invisible;
	statusTextEscape_ = escape;
	QString tmp = statusText_;
	if (escape) {
		tmp = Ya::limitText(tmp, 160);
		tmp = Qt::escape(tmp);
	}

	QString theText = Ya::VisualUtil::spanFor(!invisible ? Qt::black : backgroundColor(),
	                  tmp);

	PsiIcon* icon = PsiIconset::instance()->statusPtr(contact_->status().type());
	QString status;
	if (icon)
		status += QString("<icon name=\"%1\">&nbsp;").arg(icon->name());
	QString statusDesc = Ya::statusDescription(contact_->status().type(), contact_->gender());
	if (!text.isEmpty()) {
		if (contact_->status().type() != XMPP::Status::Offline)
			statusDesc += ":";
		statusDesc += " ";
	}
	status += Ya::VisualUtil::spanFor(Ya::VisualUtil::rosterToolTipStatusColor(contact_->status().type()),
	          statusDesc);
	QString statusTypeDescriptionText;
	fullStatusTextHtmlSansStatus_ = QString();
	if (!text.isEmpty()) {
		statusTypeDescriptionText = QString("%1%2")
		                                        .arg(status)
		                                        .arg(theText);
		fullStatusTextHtmlSansStatus_ = theText;
	}
	else {
		statusTypeDescriptionText = QString("%1")
		                                        .arg(status);
	}
	fullStatusTextHtml_ = statusTypeDescriptionText;
	ui_.statusTypeDescriptionLabel->clear();
	ui_.statusTypeDescriptionLabel->appendText(statusTypeDescriptionText);

	QTextFrameFormat frameFormat = ui_.statusTypeDescriptionLabel->document()->rootFrame()->frameFormat();
	frameFormat.setLeftMargin(0);
	frameFormat.setRightMargin(0);
	frameFormat.setTopMargin(0);
	frameFormat.setBottomMargin(0);
	ui_.statusTypeDescriptionLabel->document()->rootFrame()->setFrameFormat(frameFormat);
}

QFont YaRosterTipLabel::labelFont() const
{
	return Ya::VisualUtil::normalFont();
}

void YaRosterTipLabel::updateFont()
{
	QFont f = labelFont();
	ui_.statusTypeDescriptionLabel->setFont(f);
	ui_.jidLabel->setFont(f);
	ui_.infoLabel->setFont(f);
}

void YaRosterTipLabel::updateSize(bool force)
{
	if (!updatesEnabled() && !force)
		return;

	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	updateFont();

	if (!isHelperTipLabel()) {
		YaRosterTipLabel* helper = new YaRosterTipLabel(0, true);
		helper->setUpdatesEnabled(false);
		helper->init(theText());
		helper->setContact(contact_);
		helper->setStatusText(fullStatusTextHtmlSansStatus_, false, false);
		helper->setUpdatesEnabled(true);
		helper->updateSize();

		newGeometry_ = helper->newGeometry_;
		setProperty("firstButtonTop", QVariant(helper->firstButtonTop()));
		if (tipPosition_) {
			newGeometry_.moveTo(tipPosition_->calculateTipPosition(this, newGeometry_.size()));
		}

		ui_.statusTypeDescriptionLabel->setFixedSize(helper->ui_.statusTypeDescriptionLabel->maximumSize());
		ui_.statusTypeDescriptionLabel->verticalScrollBar()->setValue(0);

#ifdef Q_WS_WIN
		setFixedSize(newGeometry_.size());
#endif
		setGeometry(newGeometry_);
		setFixedSize(newGeometry_.size());

#ifdef Q_WS_WIN
		// unfortunately when window is hidden, sometimes it can't resize
		// to the size we want. work-around here is to move it away from
		// screen area, temporarily show it, hide back and set the geometry
		// we want once more
		if (!isVisible()) {
			QRect geom = newGeometry_;
			geom.moveTo(0, Ya::VisualUtil::belowScreenGeometry());
			setGeometry(geom);
			show();
			hide();

			setGeometry(newGeometry_);
			setFixedSize(newGeometry_.size());
		}
#endif

		foreach(QLayout* layout, findChildren<QLayout*>()) {
			layout->invalidate();
			layout->activate();
		}

		delete helper;
	}
	else {
		setFixedWidth(265);
#ifdef Q_WS_MAC
		// on Tiger font on buttons is too large
		setFixedWidth(295);
#endif
		move(0, Ya::VisualUtil::belowScreenGeometry() + 100);
		show();

		int leftMargin, topMargin, rightMargin, bottomMargin;
		layout()->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

		QLabel label(0);
		label.setFont(labelFont());
		label.setTextFormat(Qt::RichText);
		label.setText(QString("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") + fullStatusTextHtml_);

		// label.move(0, Ya::VisualUtil::belowScreenGeometry() + 100);
		// label.move(10, 10);
		// label.show();
		// label.setFont(labelFont());

		label.setWordWrap(true);
		label.setMaximumWidth(maximumWidth() - (leftMargin + rightMargin));
		label.resize(QSize(label.maximumWidth(), label.heightForWidth(label.maximumWidth())));
		ui_.statusTypeDescriptionLabel->setFixedSize(
		    QSize(label.maximumWidth(),
		          label.heightForWidth(label.maximumWidth())
#ifdef Q_WS_WIN
		          + 3
#endif
		    ));

		layout()->invalidate();
		layout()->activate();

		QSize newSize(minimumSizeHint());
		newSize.setWidth(minimumWidth());
		resize(newSize);

		// label.setUpdatesEnabled(true);
		// label.repaint();
		// qApp->processEvents();
		// Sleep(1000);

		// setUpdatesEnabled(true);
		// repaint();
		// setUpdatesEnabled(true);
		// qApp->processEvents();
		// // QThread::currentThread()->sleep(1);
		// Sleep(1000);

		newGeometry_ = QRect(pos(), size());
	}

	setUpdatesEnabled(updatesEnabled);
}

void YaRosterTipLabel::applyNewGeometry()
{
	updateGeometryTimer_->stop();
	setGeometry(newGeometry_);
}

void YaRosterTipLabel::vcardFinished()
{
	XMPP::JT_VCard* jtVCard = static_cast<XMPP::JT_VCard*>(sender());
	if (jtVCard->success()) {
		setVCard(jtVCard->vcard());
	}
}

void YaRosterTipLabel::setLastAvailable(const QDateTime& dateTime, bool invisible)
{
	QString msg = useMaleMsg() ?
	              tr("Last available on", "contact is male") :
	              tr("Last available on", "contact is female");
	QString str = QString("<br>%1<br>%2")
	              .arg(msg)
	              .arg(Ya::DateFormatter::instance()->longDateTime(dateTime));
	setStatusText(str, invisible, false);
}

void YaRosterTipLabel::lastAvailableFinished()
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);
	LastActivityTask *j = static_cast<LastActivityTask*>(sender());
	if (j->success()) {
		setLastAvailable(j->time(), false);
	}
	else {
		setStatusText(QString(), true, false);
	}
	setUpdatesEnabled(updatesEnabled);
	updateSize();
}

void YaRosterTipLabel::setVCard(XMPP::VCard vcard)
{
	if (vcard.gender() != XMPP::VCard::UnknownGender)
		ui_.genderLabel->setPixmap(Ya::genderPixmap(vcard.gender()));
	else
		ui_.genderLabel->setPixmap(QPixmap());

	Q_ASSERT(profile_);
	bool stubTextWasUsed;
	QString text = Ya::VisualUtil::contactAgeLocation(profile_->account(), profile_->jid(), &stubTextWasUsed);
	QColor color = Qt::black;
	if (stubTextWasUsed) {
		color = Qt::gray;
	}

	ui_.infoLabel->setText(text);

	QPalette pal = ui_.infoLabel->palette();
	pal.setColor(QPalette::Text, color);
	pal.setColor(QPalette::WindowText, color);
	ui_.infoLabel->setPalette(pal);

	setStatusText(statusText_, statusTextInvisible_, statusTextEscape_);

	updateSize();
}

QSize YaRosterTipLabel::sizeHint() const
{
	return QFrame::sizeHint();
}

static bool isWidgetOrOneOfTheParents(QWidget* widget, QWidget* compareWith)
{
	if (widget == compareWith)
		return true;

	if (widget && widget->parentWidget())
		return isWidgetOrOneOfTheParents(widget->parentWidget(), compareWith);

	return false;
}

QColor YaRosterTipLabel::backgroundColor() const
{
	return Ya::VisualUtil::highlightBackgroundColor();
}

void YaRosterTipLabel::hideMenuAndTip()
{
	jidMenu_->hide();
	hideTip();
	QTimer::singleShot(0, this, SLOT(hideTip()));
}

bool YaRosterTipLabel::eventFilter(QObject* obj, QEvent* e)
{
	if (leaveTimer_ && !leaveTimer_->isEnabledWithoutConsideringPermanentlyDisabledState()) {
		switch (e->type()) {
		case QEvent::ApplicationDeactivate:
			hideMenuAndTip();
			return false;
		case QEvent::MouseButtonPress: {
			if (obj == jidMenu_) {
				return false;
			}

			QWidget* w = dynamic_cast<QWidget*>(obj);
			QList<QWidget*> widgets;
			widgets << this;
			widgets << findChildren<QWidget*>();
			if (widgets.contains(w)) {
				leaveTimer_->setEnabled(true);
				jidMenu_->hide();
				return false;
			}
		}
		case QEvent::KeyPress:
			hideMenuAndTip();
		default:
			;
		}
		return false;
	}

	switch (e->type()) {
	case QEvent::MouseButtonPress: {
		QMouseEvent* me = static_cast<QMouseEvent*>(e);
		QRect r(ui_.jidIconLabel->mapToGlobal(ui_.jidIconLabel->rect().topLeft()), ui_.jidLabel->mapToGlobal(ui_.jidLabel->rect().bottomRight()));

		if (leaveTimer_ && leaveTimer_->isEnabledWithoutConsideringPermanentlyDisabledState() &&
		    r.contains(me->globalPos()))
		{
			leaveTimer_->setEnabled(false);

			qApp->installEventFilter(this);
			// jidMenu_->exec(me->globalPos());
			jidMenu_->exec(ui_.jidLabel->mapToGlobal(ui_.jidLabel->rect().bottomLeft()) + QPoint(0, 3));
			qApp->removeEventFilter(this);

			if (!leaveTimer_.isNull())
				leaveTimer_->setEnabled(true);
			return true;
		}
	}
	default:
		;
	}

	bool result;
	if (leaveTimer_->processEvent(obj, e, &result))
		return result;
	return PsiTipLabel::eventFilter(obj, e);
}

void YaRosterTipLabel::paintEvent(QPaintEvent*)
{
	Q_ASSERT(updatesEnabled());
	QPainter p(this);
	p.fillRect(rect(), backgroundColor());
}

void YaRosterTipLabel::enterEvent(QEvent*)
{
	emit toolTipEntered(contact_, contactSelection_);

	// we don't want our tooltip hidden as result of enterEvent,
	// so we don't call parent's enterEvent
}

void YaRosterTipLabel::startHideTimer()
{
	// we don't want to be hidden so we do nothing
}

void YaRosterTipLabel::copyJid()
{
	jidMenu_->hide();
	closeClicked();
	QTimer::singleShot(0, this, SLOT(closeClicked()));
	if (!contact_)
		return;
	QApplication::clipboard()->setText(contact_->jid().full());
}

void YaRosterTipLabel::resizeEvent(QResizeEvent*)
{
	int corner = 6;
	setMask(Ya::VisualUtil::roundedMask(size(), corner, Ya::VisualUtil::AllBorders));
}

void YaRosterTipLabel::openChat()
{
	closeClicked();
	contact_->openChat();
}

void YaRosterTipLabel::composeEmail()
{
	jidMenu_->hide();
	closeClicked();
	QTimer::singleShot(0, this, SLOT(closeClicked()));
	contact_->yaEmail();
}

void YaRosterTipLabel::openYaProfile()
{
	closeClicked();
	contact_->yaProfile();
}

void YaRosterTipLabel::openHistory()
{
	closeClicked();
	if (!selfContact()) {
		contact_->history();
	}
	else {
		Ya::showHistory(0, XMPP::Jid());
	}
}

void YaRosterTipLabel::openPhotos()
{
	closeClicked();
	contact_->yaPhotos();
}

void YaRosterTipLabel::hideTip()
{
	if (!leaveTimer_->isEnabled())
		return;

	PsiTipLabel::hideTip();
	emit toolTipHidden(contact_, contactSelection_);
}

void YaRosterTipLabel::closeClicked()
{
	leaveTimer_->setPermanentlyDisabled(false);
	hideTip();
}

void YaRosterTipLabel::updateContact(const XMPP::Jid& jid)
{
	if (jid == profile_->jid()) {
		updateActionButtons();
	}
}

void YaRosterTipLabel::setText(const QString& _text)
{
	QRegExp rx(QString("^%1").arg(YaRosterToolTip::forceTipLabelUpdateString()));
	QString text(_text);
	text.replace(rx, "");
	PsiTipLabel::setText(text);
}

void YaRosterTipLabel::showEvent(QShowEvent* event)
{
	PsiTipLabel::showEvent(event);
}

bool YaRosterTipLabel::leaveTimerPermanentlyDisabled() const
{
	return leaveTimer_->permanentlyDisabled();
}

void YaRosterTipLabel::setLeaveTimerPermanentlyDisabled(bool permanentlyDisabled)
{
	if (leaveTimer_->permanentlyDisabled() != permanentlyDisabled) {
		leaveTimer_->setPermanentlyDisabled(permanentlyDisabled);
		setWindowFlags(permanentlyDisabled ? Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint : Qt::ToolTip);
#ifdef Q_WS_WIN
		if (permanentlyDisabled) {
			SetWindowLongA(winId(), GWL_EXSTYLE, GetWindowLongA(winId(), GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
		}
		else {
			SetWindowLongA(winId(), GWL_EXSTYLE, GetWindowLongA(winId(), GWL_EXSTYLE) & ~WS_EX_TOOLWINDOW);
		}
#endif
	}
}

bool YaRosterTipLabel::useMaleMsg() const
{
	return contact_ ? Ya::useMaleMessage(contact_->gender()) : true;
}
