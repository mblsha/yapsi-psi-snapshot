/*
 * yaselfmood.cpp - expanding mood widget
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

#include "yaselfmood.h"

#include <QAction>
#include <QActionGroup>
#include <QAbstractButton>
#include <QPainter>
#include <QTimeLine>
#include <QTimer>
#include <QLineEdit>
#include <QTextEdit>
#include <QMenu>
#include <QLayout>
#include <QAbstractTextDocumentLayout>
#include <QToolButton>
#include <QScrollBar>

#include "yaselfmoodaction.h"
#include "psioptions.h"
#include "yavisualutil.h"
#include "common.h"
#include "typographyhighlighter.h"
#include "yaspellhighlighter.h"
#include "combinedsyntaxhighlighter.h"
#include "yastyle.h"
#include "yacommon.h"
#include "psirichtext.h"
#include "yaeditorcontextmenu.h"
#include "yaboldmenu.h"
#include "yaokbutton.h"
#include "yaclosebutton.h"
#include "psitooltip.h"
#include "psicontactlist.h"
#include "psiaccount.h"
#include "yawindow.h"
#include "yaremoveconfirmationmessagebox.h"

static const QString statusMessagesOptionPath = "options.ya.moods.all";
static const QString showYaruWarningOptionPath = "options.ya.moods.show-yaru-warning";

static const int numberOfPastStatuses = 5;
static const int shadowWidth = 3;
static const int shadowHeight = 7;
static const int statusButtonMargin = 6;

class YaSelfMoodStatusButton : public QToolButton
{
	Q_OBJECT
public:
	YaSelfMoodStatusButton(QWidget* parent, YaSelfMoodButton* moodButton)
		: QToolButton(parent)
		, moodButton_(moodButton)
		, status_(XMPP::Status::Offline)
		, enableRotation_(false)
	{
		setAttribute(Qt::WA_Hover, true);

		Q_ASSERT(moodButton_);
		setPopupMode(QToolButton::InstantPopup);
		setCursor(Qt::PointingHandCursor);
		setFocusPolicy(Qt::NoFocus);
	}

	void setStatus(XMPP::Status::Type status)
	{
		status_ = status;
		update();
	}

	void setContactList(PsiContactList* contactList)
	{
		contactList_ = contactList;
	}

	XMPP::Status::Type status() const
	{
#ifdef YAPSI_ACTIVEX_SERVER
		if (!contactList_.isNull() && contactList_->accountsLoaded()) {
			PsiAccount* acc = contactList_->onlineAccount();
			// PsiAccount* acc = !contactList_->enabledAccounts().isEmpty() ? contactList_->enabledAccounts().first() : 0;
			if (acc && acc->isActive() && !acc->isAvailable()) {
				return XMPP::Status::Reconnecting;
			}

			if (acc && acc->isAvailable()) {
				return acc->status().type();
			}

			return XMPP::Status::Offline;
		}
#else
		if (enableRotation_) {
			return XMPP::Status::Reconnecting;
		}
#endif
		return status_;
	}

	void enableRotation(bool enable)
	{
		enableRotation_ = enable;
		update();
	}

	QPixmap downArrowPixmap() const
	{
		// if (isDown())
		// 	return QPixmap(":/images/down-arrow/pressed.png");

		if (underMouse() || isDown())
			return QPixmap(":/images/down-arrow/hover.png");

		return QPixmap(":/images/down-arrow/normal.png");
	}

	// reimplemented
	void paintEvent(QPaintEvent*)
	{
		QPainter p(this);
		// p.drawPixmap(0, 0, moodButton_->currentPixmap());
		XMPP::Status::Type status = this->status();
		p.drawPixmap(0, 0, Ya::VisualUtil::statusPixmap(status));
		if (underMouse() || isDown()) {
			p.save();
			p.setCompositionMode(QPainter::CompositionMode_Screen);
			p.drawPixmap(0, 0, Ya::VisualUtil::statusPixmap(status));
			p.restore();
		}

		p.drawPixmap(10, 4, downArrowPixmap());

		// p.setPen(Qt::black);
		// p.drawRect(rect().adjusted(0, 0, -1, -1));
	}

	// reimplemented
	QSize sizeHint() const
	{
		return QSize(16, 11);
	}

private:
	QPointer<PsiContactList> contactList_;
	YaSelfMoodButton* moodButton_;
	YaRotation* rotation_;
	XMPP::Status::Type status_;
	bool enableRotation_;
};

//----------------------------------------------------------------------------
// YaSelfMoodBorderRectCalculator
//----------------------------------------------------------------------------

YaSelfMoodBorderRectCalculator::YaSelfMoodBorderRectCalculator(BorderRenderer* renderer)
	: BorderRectCalculator(renderer)
{
}

QRect YaSelfMoodBorderRectCalculator::pixmapRect(PixmapLocation pixmap, const QRect& rect) const
{
	if (pixmap == Arrow) {
		QRect result = BorderRectCalculator::pixmapRect(pixmap, rect);
		result.moveTo(0, 7);
		return result;
	}
	else if (isLeft(pixmap)) {
		QRect result = BorderRectCalculator::pixmapRect(pixmap, rect);
		result.translate(-4, 0);
		return result;
	}

	return BorderRectCalculator::pixmapRect(pixmap, rect);
}

//----------------------------------------------------------------------------
// YaSelfMoodExtra
//----------------------------------------------------------------------------

YaSelfMoodExtra::YaSelfMoodExtra(QWidget* parent, YaSelfMood* placeholderParent)
	: ExpandingExtraWidget(parent, placeholderParent)
	, menu_(0)
	, lineEdit_(0)
	, lineEditHovered_(false)
	, focusInTimer_(0)
	, focusOutTimer_(0)
	, currentAction_(0)
	, currentActionButton_(0)
	, statusButton_(0)
	, okButton_(0)
	, closeButton_(0)
	, changeStatusAction_(0)
	, menuPastStatuses_(0)
	, statusType_(XMPP::Status::Offline)
	, gender_(XMPP::VCard::UnknownGender)
	, editingBackground_(false)
	, showingLineEditMenu_(false)
	, state_(Normal)
	, animatingTo_(-1)
{
	menu_ = new YaBoldMenu(this);
	menu_->setTitle(tr("Mood"));

	currentAction_ = new QAction(this);
	currentAction_->setProperty("current", QVariant(true));
	currentAction_->setVisible(true);

	currentActionButton_ = new YaSelfMoodButton(this);
	currentActionButton_->addAction(currentAction_);
	currentActionButton_->setFocusPolicy(Qt::NoFocus);
	currentActionButton_->setDrawSelection(false);
	currentActionButton_->show();

	statusButton_ = new YaSelfMoodStatusButton(this, currentActionButton_);
	statusButton_->setMenu(menu_);

	lineEdit_ = new QTextEdit(this);
	lineEdit_->setAcceptRichText(false);
	connect(lineEdit_, SIGNAL(textChanged()), SLOT(lineEditTextChanged()));
	lineEdit_->setMinimumHeight(0);
	lineEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	// lineEdit_->setFrame(false);
	lineEdit_->hide();
	lineEdit_->setFocusPolicy(Qt::ClickFocus);
	lineEdit_->setUndoRedoEnabled(true);

#if 0
	CombinedSyntaxHighlighter* hl = new CombinedSyntaxHighlighter(lineEdit_);
	new TypographyHighlighter(hl, lineEdit_);
	if (PsiOptions::instance()->getOption("options.ui.spell-check.enabled").toBool()) {
		new YaSpellHighlighter(hl, lineEdit_);
	}
#endif

	createMenu();

	lineEdit_->installEventFilter(this);
	lineEdit_->viewport()->installEventFilter(this);
	currentActionButton_->installEventFilter(this);

	currentActionButton_->setMouseTracking(true);

#ifndef YAPSI_NO_STYLESHEETS
	lineEdit_->setStyleSheet(currentActionButton_->styleSheet());
	// setStyleSheet(
	// "QTextEdit {"
	// "	border-style: none;"
	// "	border-width: 0;"
	// // "	background-color: white;"
	// "}"
	// );
#endif
	// lineEdit_->setFont(Ya::VisualUtil::normalFont());
	lineEdit_->setFrameShape(QFrame::NoFrame);

	okButton_ = new YaOkButton(this);
	okButton_->setFocusPolicy(Qt::NoFocus);
	okButton_->hide();
	connect(okButton_, SIGNAL(clicked()), SLOT(okClicked()));

	closeButton_ = new YaCloseButton(this);
	closeButton_->setFocusPolicy(Qt::NoFocus);
	closeButton_->hide();
	connect(closeButton_, SIGNAL(clicked()), SLOT(closeClicked()));

	focusInTimer_ = new QTimer(this);
	focusInTimer_->setInterval(100);
	focusInTimer_->setSingleShot(true);
	connect(focusInTimer_, SIGNAL(timeout()), SLOT(doFocusIn()));

	focusOutTimer_ = new QTimer(this);
	focusOutTimer_->setInterval(100);
	focusOutTimer_->setSingleShot(true);
	connect(focusOutTimer_, SIGNAL(timeout()), SLOT(setNormalState()));

	QTimer::singleShot(0, this, SLOT(updateSizes()));
	// updateSizes();
}

void YaSelfMoodExtra::lineEditTextChanged()
{
	emit invalidateExtraPosition();
	updateLineEditPosition();
}

QMenu* YaSelfMoodExtra::menu() const
{
	return menu_;
}

QString YaSelfMoodExtra::statusName(XMPP::Status::Type status) const
{
	QString result;
	switch (status) {
	case XMPP::Status::Offline:
		result = tr("Disconnect");
		break;
	case XMPP::Status::XA:
		if (PsiOptions::instance()->getOption("options.ui.menu.status.xa").toBool()) {
			result = tr("XA");
			break;
		}
	case XMPP::Status::Away:
		result = tr("Away");
		break;
	case XMPP::Status::DND:
		result = useMaleMsg() ?
		         tr("Do not disturb", "contact is male") :
		         tr("Do not disturb", "contact is female");
		break;
	case XMPP::Status::FFC:
		if (PsiOptions::instance()->getOption("options.ui.menu.status.chat").toBool()) {
			result = tr("I want to chat!");
			break;
		}
	case XMPP::Status::Online:
		result = useMaleMsg() ?
		         tr("Online", "contact is male") :
		         tr("Online", "contact is female");
		break;
	case XMPP::Status::Invisible:
		result = useMaleMsg() ?
		         tr("Invisible", "contact is male") :
		         tr("Invisible", "contact is male");
		break;
	default:
		Q_ASSERT(false);
	}

	return result;
}

bool YaSelfMoodExtra::useMaleMsg() const
{
	return Ya::useMaleMessage(gender_);
}

QAction* createSeparatorAction(QObject* parent)
{
	QAction* separator = new QAction(parent);
	separator->setVisible(true);
	separator->setSeparator(true);
	return separator;
}

void YaSelfMoodExtra::createMenu()
{
	menuPastStatuses_ = new YaSelfMoodActionGroup(this);
	connect(menuPastStatuses_, SIGNAL(actionsChanged()), SLOT(invalidateButtons()));

	onlineAction_ = new QAction(this);
	onlineAction_->setProperty("status", QVariant(XMPP::Status::Online));
	onlineAction_->setIcon(Ya::VisualUtil::rosterStatusPixmap(XMPP::Status::Online));
	setActionMood(onlineAction_, statusText());
	// onlineAction_->setVisible(false);
	connect(onlineAction_, SIGNAL(activated()), SLOT(setStatus()));

	awayAction_ = new QAction(this);
	awayAction_->setVisible(true);
	awayAction_->setProperty("status", QVariant(XMPP::Status::Away));
	awayAction_->setIcon(Ya::VisualUtil::rosterStatusPixmap(XMPP::Status::Away));
	connect(awayAction_, SIGNAL(activated()), SLOT(setStatus()));

	changeStatusAction_ = new QAction(this);
	changeStatusAction_->setVisible(true);
	connect(changeStatusAction_, SIGNAL(activated()), SLOT(setStatus()));

	setCustomMoodAction_ = new QAction(this);
	YaBoldMenu::ensureActionBoldText(setCustomMoodAction_);
	setCustomMoodAction_->setText(tr("Custom mood..."));
	setCustomMoodAction_->setVisible(true);
	connect(setCustomMoodAction_, SIGNAL(activated()), SLOT(setCustomMood()));

	offlineAction_ = new QAction(this);
	offlineAction_->setProperty("status", QVariant(XMPP::Status::Offline));
	offlineAction_->setIcon(Ya::VisualUtil::rosterStatusPixmap(XMPP::Status::Offline));
	offlineAction_->setVisible(true);
	connect(offlineAction_, SIGNAL(activated()), SLOT(setStatus()));

	menuActions_ << onlineAction_;
	menuActions_ << awayAction_;
	menuActions_ << changeStatusAction_;
	menuActions_ << createSeparatorAction(this);
	menuActions_ << setCustomMoodAction_;
	menuActions_ << menuPastStatuses_;
#ifndef YAPSI_ACTIVEX_SERVER
	menuActions_ << createSeparatorAction(this);
	menuActions_ << offlineAction_;
#endif

	QTimer::singleShot(0, this, SLOT(invalidateButtons()));
}

QSize YaSelfMoodExtra::arrowSize() const
{
	return QSize(15, 9);
}

void YaSelfMoodExtra::setHaveConnectingAccounts(bool haveConnectingAccounts)
{
	currentActionButton_->enableRotation(haveConnectingAccounts);
	statusButton_->enableRotation(haveConnectingAccounts);
}

void YaSelfMoodExtra::startAnimation(State state)
{
	int animatingTo = -1;
	ExpandingExtraWidget::AnimationType type = ExpandingExtraWidget::ExpandingAnimation;

	switch (state) {
	// case ExpandingToMenu:
	// 	animatingTo = Menu;
	// 	break;
	case EditingToNormal:
	// case MenuToNormal:
		animatingTo = Normal;
		type = ExpandingExtraWidget::CollapsingAnimation;
		break;
	// case MenuToEditing:
	case NormalToEditing:
		animatingTo = Editing;
		break;
	default:
		;
	}

	if (animatingTo != -1) {
		animatingTo_ = animatingTo;
		ExpandingExtraWidget::startAnimation(type, sizeHint((State)animatingTo));
	}
}

void YaSelfMoodExtra::stopAnimation()
{
	animatingTo_ = -1;
	ExpandingExtraWidget::stopAnimation();
}

void YaSelfMoodExtra::animationFinished()
{
	if (animatingTo_ != -1) {
		setState((State)animatingTo_);
	}
}

void YaSelfMoodExtra::showYaruWarning()
{
	Q_ASSERT(state_ == Editing);
	if (state_ != Editing)
		return;

	PsiToolTip::showText(mapToGlobal(rect().bottomLeft()) + QPoint(15, -15),
	                     tr("This mood will be published on Ya.ru and all your contacts will be able to see it."),
	                     0);
	PsiOptions::instance()->setOption(showYaruWarningOptionPath, false);
}

void YaSelfMoodExtra::setState(State state)
{
	if (state_ == state)
		return;

	if (state == NormalToEditing || state == Editing) {
		if (statusType_ == XMPP::Status::Away ||
		    statusType_ == XMPP::Status::XA)
		{
			YaRemoveConfirmationMessageBoxManager::instance()->showInformation("YaSelfMoodExtra_cant_change_mood_while_away",
			        tr("Changing mood"), tr("You can't change mood while being 'away'."),
			        window());
			return;
		}
	}

	state_ = state;
	stopAnimation();

	// updateButtonsVisibility(state_ == Menu);

	bool drawEmpty = true;
	switch (state_) {
	case Editing:
		if (PsiOptions::instance()->getOption(showYaruWarningOptionPath).toBool()) {
			QTimer::singleShot(10, this, SLOT(showYaruWarning()));
		}
		break;
	// case ExpandingToMenu:
	// 	setExpandedState();
	// 	break;
	// case MenuToEditing:
	// 	setExpandedState();
	// 	break;
	case NormalToEditing:
		setExpandedState();
		drawEmpty = false;

		// ONLINE-876
		lineEdit_->selectAll();
		break;
	default:
		;
	}
	startAnimation(state_);
	currentActionButton_->setDrawEmpty(drawEmpty);

	updateLineEdit();
}

void YaSelfMoodExtra::updateLineEdit()
{
	if (!lineEdit_)
		return;

	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	// bool lineEditShouldBeVisible = moodIsEditable() && (!lineEdit_->toPlainText().isEmpty() || state_ == Editing);
	bool lineEditShouldBeVisible = state_ != Normal;
	lineEdit_->setVisible(lineEditShouldBeVisible);
	currentActionButton_->setVisible(!lineEditShouldBeVisible);

	QPalette pal = lineEdit_->palette();
	pal = YaStyle::useNativeTextSelectionColors(pal);
	// pal.setColor(QPalette::Base, editingBackground() ? Ya::VisualUtil::editAreaColor() : Ya::VisualUtil::blueBackgroundColor());
	pal.setColor(QPalette::Base, Qt::transparent);
	lineEdit_->setPalette(pal);

	if (state_ != Editing) {
		lineEdit_->verticalScrollBar()->setValue(0);
	}

	if (lineEdit_->isVisible()) {
		// lineEdit_->raise();
		// lineEdit_->selectAll();
		if (state_ == Editing) {
			lineEdit_->setFocus();
		}
		// lineEdit_->viewport()->update();
	}

	updateLineEditPosition();
	setUpdatesEnabled(updatesEnabled);
}

QSize YaSelfMoodExtra::sizeHint(YaSelfMoodExtra::State state) const
{
	switch (state) {
	// case ExpandingToMenu:
	// case MenuToNormal:
	// case MenuToEditing:
	// 	return currentAnimationFrame();
	// case Menu:
	// 	return QSize(expandedWidth(), expandedHeight());
	case Editing:
		return QSize(editingWidth(), editingHeight());
	case NormalToEditing:
	case EditingToNormal:
		return currentAnimationFrame();
	default:
		return QSize(normalWidth(), normalHeight());
	}
}

QSize YaSelfMoodExtra::sizeHint() const
{
	return sizeHint(state_);
}

void YaSelfMoodExtra::invalidateButtons()
{
	ExpandingExtraWidget::invalidateButtons();

	foreach(QObject* obj, menuActions_) {
		QAction* action = dynamic_cast<QAction*>(obj);
		YaSelfMoodActionGroup* actionGroup = dynamic_cast<YaSelfMoodActionGroup*>(obj);
		Q_ASSERT(action || actionGroup);
		if (action) {
			createButtonFor(action);
		}
		else if (actionGroup) {
			foreach(QAction* act, actionGroup->actions())
				createButtonFor(act);
		}
	}

	// updateButtonsPositions();
	updateMenuFromButtons(menu_);
}

void YaSelfMoodExtra::setCustomMood()
{
	if (state_ == Editing)
		return;

	bringToFront(this);
	setFocus();

	// if (state_ == Normal)
		setState(NormalToEditing);
	// else
	// 	setState(MenuToEditing);

	lineEdit_->selectAll();
}

void YaSelfMoodExtra::setStatus()
{
	emit resetLastManualStatusSafeGuard();

	QAction* action = static_cast<QAction*>(sender());
	QVariant statusType = action->property("status");
	Q_ASSERT(!statusType.isNull() && statusType.isValid());
	setStatusTypeManual(static_cast<XMPP::Status::Type>(statusType.toInt()));
}

void YaSelfMoodExtra::setMood()
{
	QAction* action = static_cast<QAction*>(sender());
	setStatusText(action->property("mood").toString());
}

// QPixmap YaSelfMoodExtra::downArrowPixmap() const
// {
// 	if (isAnimating() || state_ == Menu)
// 		return QPixmap(":/images/down-arrow/pressed.png");
//
// 	if (testAttribute(Qt::WA_UnderMouse) && (!lineEditHovered() || moodIsEditable()))
// 		return QPixmap(":/images/down-arrow/hover.png");
//
// 	return QPixmap(":/images/down-arrow/normal.png");
// }

void YaSelfMoodExtra::drawBackground(QPainter* painter, const QRect& rect, const QRect arrowRect)
{
	QPoint arrow[3];
	arrow[0] = QPoint(arrowRect.left(),  arrowRect.center().y());
	arrow[1] = QPoint(arrowRect.right(), arrowRect.top());
	arrow[2] = QPoint(arrowRect.right(), arrowRect.bottom());

	QRect r(rect);
	r.setLeft(arrowRect.right());
	r.adjust(0, 0, -1, -1);
	painter->drawRect(r);

	painter->setRenderHint(QPainter::Antialiasing);
	painter->drawConvexPolygon(arrow, 3);
}

void YaSelfMoodExtra::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	// p.fillRect(rect(), Qt::white);

	bool editing = false;
	QColor c;
	switch (state_) {
	// case MenuToEditing:
	case Editing:
	case NormalToEditing:
		editing = true;
		break;
	default:
		;
	}

	if (editing) {
		c = Ya::VisualUtil::editAreaColor();
	}
	else {
		c = Ya::VisualUtil::blueBackgroundColor();
	}

	// if (editingBackground()) {
		BorderRenderer* border = BorderRenderer::rendererFor(!editingBackground() ? ":images/balloon/mood" : ":images/balloon/mood_editing");
		border->setBorderRectCalculator(new YaSelfMoodBorderRectCalculator(border));
		border->draw(&p, rect());
		return;
	// }

	p.setPen(c);
	p.setBrush(c);

	QRect r(rect());
	r.adjust(0, 0, -shadowWidth, -shadowHeight);
	drawBackground(&p, r, arrowRect_);

	// p.drawPixmap(downArrowRect_.left() + (downArrowRect_.width()  - downArrowPixmap().width())  / 2,
	//              downArrowRect_.top()  + (downArrowRect_.height() - downArrowPixmap().height()) / 2, downArrowPixmap());

	// p.drawRect(innerRect_);
	// p.drawRect(downArrowRect_);
	// p.drawRect(pixmapRect_);
}

void YaSelfMoodExtra::contextMenuEvent(QContextMenuEvent* e)
{
	e->accept();
}

void YaSelfMoodExtra::updateButtonsPositions()
{
	ExpandingExtraWidget::updateButtonsPositions(innerRect_);
}

bool YaSelfMoodExtra::lineEditHovered() const
{
	return lineEditHovered_;
}

QRect YaSelfMoodExtra::lineEditRect() const
{
	if (!currentActionButton_ || !lineEdit_)
		return QRect();

	QRect global = QRect(currentActionButton_->mapToGlobal(currentActionButton_->textRect().topLeft()),
	                     currentActionButton_->mapToGlobal(currentActionButton_->textRect().bottomRight()));

	global.moveTop(global.top());
	global.setHeight(qMin(int(lineEdit_->document()->documentLayout()->documentSize().height()),
	                      80));
	global.setHeight(global.height() + lineEdit_->lineWidth() * 2);

	return QRect(mapFromGlobal(global.topLeft()),
	             mapFromGlobal(global.bottomRight()));
}

int YaSelfMoodExtra::editingHeight() const
{
	int height = lineEditRect().height() + 3 + 4 + shadowHeight;
	height += okButton_->sizeHint().height() + 3;
	return qMax(height, normalHeight());
}

void YaSelfMoodExtra::updateLineEditPosition()
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	bool buttonsVisible = state_ == Editing;

	// QRect r = lineEditRect().adjusted(-2, 1, 0, -1);
	QRect r = lineEditRect().adjusted(0, 1, 0, -1);
	// r.setBottom(rect().bottom() - shadowHeight - 6 - 4);
	r.setBottom(rect().bottom() - shadowHeight - 5);
	r.translate(0, 2);
	if (state_ != Normal) {
		r.adjust(0, 0, 0, 4);
	}

	QRect buttonRect;
	buttonRect.setSize(okButton_->sizeHint());
	buttonRect.moveTo(r.right() - buttonRect.width() + 4,
	                  r.bottom() - buttonRect.height());
	closeButton_->setGeometry(buttonRect);
	closeButton_->setVisible(buttonsVisible);

	buttonRect.moveLeft(buttonRect.left() - buttonRect.width() - 2);
	okButton_->setGeometry(buttonRect);
	okButton_->setVisible(buttonsVisible);

	if (state_ == Editing) {
		r.setBottom(buttonRect.top() - 3);
	}
	lineEdit_->setGeometry(r);
	lineEdit_->blockSignals(true);
	// PsiRichText::ensureTextLayouted(lineEdit_->document(), r.width());
	QTextFrameFormat fmt = lineEdit_->document()->rootFrame()->frameFormat();
	fmt.setMargin(0);
	// if (buttonsVisible) {
	// 	fmt.setRightMargin(okButton_->sizeHint().width() + 3);
	// }
	// else {
		fmt.setRightMargin(3);
	// }
	lineEdit_->document()->rootFrame()->setFrameFormat(fmt);
	lineEdit_->document()->setTextWidth(r.width());
	lineEdit_->blockSignals(false);

	if (state_ != Editing) {
		lineEdit_->verticalScrollBar()->setValue(0);
	}

	setUpdatesEnabled(updatesEnabled);
}

void YaSelfMoodExtra::lineEditDocumentLayoutChanged()
{
	if (state_ != Editing) {
		lineEdit_->verticalScrollBar()->setValue(0);
	}
}

void YaSelfMoodExtra::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	// if (event->oldSize().width() == event->size().width())
	// 	return;

	updateSizes();
}

void YaSelfMoodExtra::updateSizes()
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	innerRect_.setTop(0 + 5);
	innerRect_.setLeft(rect().left() + arrowSize().width() + margin() + statusButtonMargin + 2);
	innerRect_.setRight(rect().right() - margin() - margin());
	innerRect_.setBottom(normalHeight() - 3 - shadowHeight + 3);

	arrowRect_ = innerRect_;
	arrowRect_.setLeft(0);
	arrowRect_.setRight(arrowSize().width());
	arrowRect_.setBottom(arrowSize().height());
	arrowRect_.moveTop(6);

	// downArrowRect_ = innerRect_;
	// downArrowRect_.setLeft(downArrowRect_.right() - downArrowPixmap().width());
	//
	// innerRect_.setRight(downArrowRect_.left() - margin());

	currentActionButton_->setGeometry(innerRect_);
	currentActionButton_->updateRects();

	QRect statusButtonRect(innerRect_.topLeft(), statusButton_->sizeHint());
	statusButtonRect.moveLeft(statusButtonRect.left() - statusButtonMargin);
	statusButtonRect.moveTop(statusButtonRect.top() + 7);
	statusButton_->setGeometry(statusButtonRect);

	innerRect_.setRight(rect().right() - margin());

	updateButtonsPositions();
	updateLineEditPosition();

	setUpdatesEnabled(updatesEnabled);
}

QString YaSelfMoodExtra::statusText() const
{
	return currentAction_->text();
}

void YaSelfMoodExtra::setNormalState()
{
	focusInTimer_->stop();
	ExpandingExtraWidget::setNormalState();

	if (state_ == Normal ||
	    state_ == EditingToNormal
	    // || state_ == MenuToNormal
	) {
		return;
	}

	if (state_ == Editing || state_ == NormalToEditing)
		setState(EditingToNormal);
	// else if (state_ == Menu || isAnimating())
	// 	setState(MenuToNormal);
	else
		setState(Normal);

	lineEdit_->setPlainText(currentAction_->text().trimmed());
	disconnect(lineEdit_->document(), SIGNAL(contentsChanged()), this, SLOT(lineEditDocumentLayoutChanged()));
	connect(lineEdit_->document(),    SIGNAL(contentsChanged()), this, SLOT(lineEditDocumentLayoutChanged()));
	disconnect(lineEdit_->document(), SIGNAL(documentLayoutChanged()), this, SLOT(lineEditDocumentLayoutChanged()));
	connect(lineEdit_->document(),    SIGNAL(documentLayoutChanged()), this, SLOT(lineEditDocumentLayoutChanged()));

	lineEdit_->clearFocus();
	QTextCursor cursor = lineEdit_->textCursor();
	cursor.clearSelection();
	lineEdit_->setTextCursor(cursor);
}

void YaSelfMoodExtra::setStatusTextUserAction(const QString& _statusText)
{
	emit resetLastManualStatusSafeGuard();

	QString statusText = _statusText.trimmed();
	// ONLINE-876
	// if (statusText.isEmpty() && !currentAction_->text().isEmpty()) {
	// 	setStatusText(currentAction_->text().trimmed());
	// 	return;
	// }

	setStatusText(statusText);
}

void YaSelfMoodExtra::setStatusText(const QString& statusText)
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	currentAction_->setText(statusText);
	// setActionMood(onlineAction_, statusText);
	lineEdit_->setPlainText(statusText);
	setNormalState();
	updateLineEdit();

	setUpdatesEnabled(updatesEnabled);

	emit statusChanged(statusText);
	emit invalidateExtraPosition();
}

XMPP::Status::Type YaSelfMoodExtra::statusType() const
{
	return statusType_;
}

void YaSelfMoodExtra::setStatusType(XMPP::Status::Type type)
{
	statusType_ = type;
	currentAction_->setIcon(Ya::VisualUtil::rosterStatusPixmap(statusType_));
	statusButton_->setStatus(statusType_);

	// onlineAction_->setProperty("emptyText", QVariant(useMaleMsg() ?
	//                            tr("Connect", "contact is male") :
	//                            tr("Connect", "contact is female")
	//                                                 ));
	onlineAction_->setText(statusName(XMPP::Status::Online));
	awayAction_->setText(statusName(XMPP::Status::Away));
	offlineAction_->setText(statusName(XMPP::Status::Offline));

	// XMPP::Status::Type changeToStatus = (statusType_ == XMPP::Status::DND) ?
	//                                     XMPP::Status::Online :
	//                                     XMPP::Status::DND;
	XMPP::Status::Type changeToStatus = XMPP::Status::DND;

	changeStatusAction_->setProperty("status", QVariant(changeToStatus));
	changeStatusAction_->setIcon(Ya::VisualUtil::rosterStatusPixmap(changeToStatus));
	changeStatusAction_->setText(statusName(changeToStatus));

	bool offline = type == XMPP::Status::Offline;
	offlineAction_->setVisible(!offline);
	// onlineAction_->setVisible(offline);
	setCustomMoodAction_->setVisible(!offline);
	foreach(QAction* action, menuPastStatuses_->actions())
		action->setVisible(!offline);

	setNormalState();
	emit statusChanged(statusType_);
	invalidateButtons();

	updateLineEdit();
}

void YaSelfMoodExtra::setStatusTypeManual(XMPP::Status::Type type)
{
	emit statusChangedManually(type);
	setStatusType(type);
}

XMPP::VCard::Gender YaSelfMoodExtra::gender() const
{
	return gender_;
}

void YaSelfMoodExtra::setGender(XMPP::VCard::Gender gender)
{
	gender_ = gender;
	setStatusType(statusType_);
}

void YaSelfMoodExtra::setActionMood(QAction* action, const QString& mood)
{
	action->setText(Ya::limitText(mood, 25));
	action->setProperty("mood", mood);
}

void YaSelfMoodExtra::setMoods(QStringList moods)
{
	menuPastStatuses_->blockSignals(true);
	menuPastStatuses_->clear();
	foreach(QString mood, moods.mid(1, numberOfPastStatuses)) {
		if (mood.isEmpty())
			continue;

		QAction* action = new QAction(this);
		menuPastStatuses_->addAction(action);
		setActionMood(action, mood);
		connect(action, SIGNAL(activated()), SLOT(setMood()));
	}
	menuPastStatuses_->blockSignals(false);

	invalidateButtons();
}

void YaSelfMoodExtra::doFocusIn()
{
	if (state_ != Editing) {
		setState(NormalToEditing);
	}
}

bool YaSelfMoodExtra::eventFilter(QObject* obj, QEvent* event)
{
	if (!lineEdit_)
		return false;

#if 0
	if (event->type() == QEvent::Paint && (obj == lineEdit_ || obj == lineEdit_->viewport())) {
		QWidget* w = dynamic_cast<QWidget*>(obj);
		QPainter p(w);
		p.fillRect(w->rect(), Qt::red);
	}
#endif

	if (event->type() == QEvent::ContextMenu && (obj == lineEdit_ || obj == lineEdit_->viewport())) {
		showingLineEditMenu_ = true;
		YaEditorContextMenu menu(this);
		menu.exec(static_cast<QContextMenuEvent*>(event), lineEdit_);
		showingLineEditMenu_ = false;
		return true;
	}

	if (event->type() == QEvent::FocusOut && !showingLineEditMenu_) {
		if (obj == lineEdit_ || obj == lineEdit_->viewport()) {
			// if (statusText() != lineEdit_->toPlainText()) {
			// 	setStatusTextUserAction(lineEdit_->toPlainText());
			// }
			focusOutTimer_->start();
		}
	}

	if (event->type() == QEvent::FocusIn) {
		if (state_ == Normal && !showingLineEditMenu_ && obj == lineEdit_) {
			focusInTimer_->start();
			return false;
		}
		else if (state_ == Editing) {
			if (obj == okButton_ || obj == closeButton_ || obj == this) {
				focusOutTimer_->stop();
				return false;
			}
		}
	}

	if (event->type() == QEvent::MouseButtonPress && moodIsEditable() && !showingLineEditMenu_) {
		QList<QObject*> lineEdit;
		lineEdit << lineEdit_;
		lineEdit << lineEdit_->viewport();

		QList<QObject*> emptyArea;
		emptyArea << this;
		emptyArea << currentActionButton_;

		if (emptyArea.contains(obj)) {
			if (cursor().shape() != Qt::PointingHandCursor) {
				event->ignore();
				return false;
			}

			if (state_ != Editing) {
				setState(NormalToEditing);
			}
			event->accept();
			return true;
		}

		if (lineEdit.contains(obj)) {
			if (state_ != Editing) {
				setState(NormalToEditing);
			}
			return false;
		}

		if (obj == okButton_ || obj == closeButton_) {
			event->ignore();
			return false;
		}

		if (state_ != Normal) {
			setNormalState();
			return false;
		}
	}

	if (event->type() == QEvent::MouseMove ||
	    event->type() == QEvent::HoverLeave ||
	    event->type() == QEvent::HoverEnter ||
	    event->type() == QEvent::HoverMove)
	{
		QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

		if (obj == this && event->type() == QEvent::MouseMove) {
			Q_ASSERT(mouseEvent);
			QRect r(rect());
			r.adjust(0, 0, -shadowWidth, -shadowHeight);

			if (r.contains(mouseEvent->pos()) && (state_ == Normal)) {
				setCursor(Qt::PointingHandCursor);
			}
			else {
				unsetCursor();
				mouseEvent->ignore();
				return false;
			}
		}

		if (obj == this || obj == lineEdit_->viewport()) {
			if (mouseEvent) {
				QPoint pos = mapFromGlobal(mouseEvent->globalPos());
				lineEditHovered_ = lineEditRect().contains(pos);

				// if (lineEditHovered_ && moodIsEditable())
				// 	currentActionButton_->setCursor(Qt::IBeamCursor);
				// else
				// 	currentActionButton_->unsetCursor();
			}

			updateLineEdit();

			// update();

			if (obj == lineEdit_->viewport()) {
				return false;
			}

			if (!mouseEvent || mouseEvent->buttons() & Qt::LeftButton) {
				event->accept();
				return true;
			}
		}
	}

	if (obj == lineEdit_ && event->type() == QEvent::KeyPress) {
		QKeyEvent* e = static_cast<QKeyEvent*>(event);
		if (e->key() == Qt::Key_Escape) {
			cancelAction();
			return true;
		}
		else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
			setStatusTextUserAction(lineEdit_->toPlainText());
			return true;
		}
		else if (e->key() == Qt::Key_Tab && obj == lineEdit_) {
			focusNextChild();
			return true;
		}
	}

	return false;
	// return ExpandingExtraWidget::eventFilter(obj, event);
}

void YaSelfMoodExtra::closeClicked()
{
	setNormalState();
}

void YaSelfMoodExtra::okClicked()
{
	setStatusTextUserAction(lineEdit_->toPlainText());
}

int YaSelfMoodExtra::normalHeight() const
{
	return placeholderParent()->height() + shadowHeight;
}

int YaSelfMoodExtra::expandedHeight() const
{
	Q_ASSERT(false);
	return normalHeight();
}

int YaSelfMoodExtra::normalWidth() const
{
	return currentActionButton_->sizeHint().width() + margin() + margin() + shadowWidth + statusButtonMargin;
}

int YaSelfMoodExtra::expandedWidth() const
{
	QRect r(placeholderParent()->mapToGlobal(placeholderParent()->rect().topLeft()),
	        placeholderParent()->mapToGlobal(placeholderParent()->rect().bottomRight()));
	YaWindow* window = dynamic_cast<YaWindow*>(placeholderParent()->window());
	r.setRight(window->mapToGlobal(window->rect().topRight()).x() + 1 - window->additionalRightMargin());
	return r.width();
}

int YaSelfMoodExtra::editingWidth() const
{
	return expandedWidth();
}

bool YaSelfMoodExtra::isExpanded() const
{
	return state_ != Normal;
}

void YaSelfMoodExtra::animate()
{
	switch (state_) {
	// case ExpandingToMenu:
	// 	if (currentAnimationFrame() == sizeHint(Menu)) {
	// 		setState(Menu);
	// 	}
	// 	break;
	// case MenuToNormal:
	case EditingToNormal:
		if (currentAnimationFrame() == sizeHint(Normal)) {
			setState(Normal);
		}
		break;
	// case MenuToEditing:
	case NormalToEditing:
		if (currentAnimationFrame() == sizeHint(Editing)) {
			setState(Editing);
		}
		break;
	default:
		;
	}

	emit invalidateExtraPosition();
}

void YaSelfMoodExtra::changeEvent(QEvent* e)
{
	QWidget::changeEvent(e);
	animate();
}

bool YaSelfMoodExtra::editingBackground() const
{
	// bool result = (underMouse() &&
	//                // lineEdit_->underMouse() &&
	//                // !statusButton_->underMouse() &&
	//                moodIsEditable())
	//               || state_ != Normal;
	bool result = (state_ != Normal); // || (lineEdit_->toPlainText().isEmpty() && moodIsEditable());

	if (result != editingBackground_) {
		QTimer::singleShot(0, (QObject*)this, SLOT(update()));
		editingBackground_ = result;
	}
	return result;
}

bool YaSelfMoodExtra::moodIsEditable() const
{
	return statusType() != XMPP::Status::Offline;
}

void YaSelfMoodExtra::focusOut()
{
	ExpandingExtraWidget::focusOut();
}

void YaSelfMoodExtra::cancelAction()
{
	lineEdit_->setPlainText(statusText());
	focusNextChild();
	ExpandingExtraWidget::cancelAction();
}

void YaSelfMoodExtra::setContactList(PsiContactList* contactList)
{
	statusButton_->setContactList(contactList);
}

//----------------------------------------------------------------------------
// YaSelfMood
//----------------------------------------------------------------------------

YaSelfMood::YaSelfMood(QWidget* parent)
	: OverlayWidget<QFrame, YaSelfMoodExtra>(parent,
		new YaSelfMoodExtra(parent->window(), this))
{
	setFocusPolicy(Qt::NoFocus);
	connect(extra(), SIGNAL(statusChanged(const QString&)), SLOT(statusTextChanged()));
	connect(extra(), SIGNAL(statusChanged(XMPP::Status::Type)), SLOT(statusTypeChanged()));
	connect(extra(), SIGNAL(statusChangedManually(XMPP::Status::Type)), SIGNAL(statusChangedManually(XMPP::Status::Type)));
	connect(extra(), SIGNAL(resetLastManualStatusSafeGuard()), SIGNAL(resetLastManualStatusSafeGuard()));

	extra()->setStatusText(statuses().isEmpty() ? QString() : statuses().first());
	extra()->setMoods(statuses());
}

YaSelfMood::~YaSelfMood()
{
	saveStatusMessages(statuses());
}

QRect YaSelfMood::extraGeometry() const
{
	int maxW = width() - 6;
	int w = qMin(extra()->sizeHint().width(), maxW);
	QRect result(globalRect().x() - extra()->arrowSize().width() + 3,
	             globalRect().y() - 6,
	             w + extra()->arrowSize().width(),
	             qMax(extra()->sizeHint().height(), height()));
	return result;
}

bool YaSelfMood::extraShouldBeVisible() const
{
	return true;
}

QString YaSelfMood::statusText() const
{
	return extra()->statusText();
}

void YaSelfMood::setStatusText(const QString& statusText)
{
	extra()->setStatusText(statusText);
}

XMPP::Status::Type YaSelfMood::statusType() const
{
	return extra()->statusType();
}

void YaSelfMood::setStatusType(XMPP::Status::Type type)
{
	blockSignals(true);
	extra()->setStatusType(type);
	blockSignals(false);
}

void YaSelfMood::setHaveConnectingAccounts(bool haveConnectingAccounts)
{
	extra()->setHaveConnectingAccounts(haveConnectingAccounts);
}

void YaSelfMood::statusTextChanged()
{
	QList<QString> list = statuses();
	list.removeAll(statusText());
	list.push_front(statusText());
	saveStatusMessages(list);
	emit statusChanged(statusText());

	extra()->setMoods(statuses());
}

void YaSelfMood::statusTypeChanged()
{
	emit statusChanged(statusType());
}

QList<QString> YaSelfMood::statuses() const
{
	QList<QString> list;
	QVariant variant = PsiOptions::instance()->getOption(statusMessagesOptionPath);
	QString type = variant.typeName();
	if (type == "QVariantList") {
		foreach(QVariant variant, variant.toList()) {
			list << variant.toString();
		}
	}
	return list;
}

void YaSelfMood::saveStatusMessages(QList<QString> statuses)
{
	QList<QVariant> list;
	foreach(QString s, statuses) {
		if (list.count() > numberOfPastStatuses)
			break;
		list << QVariant(s);
	}
	PsiOptions::instance()->setOption(statusMessagesOptionPath, list);
}

QMenu* YaSelfMood::menu() const
{
	return extra()->menu();
}

void YaSelfMood::clearMoods()
{
	saveStatusMessages(QStringList());
	setStatusText(QString());
	statusTextChanged();
}

void YaSelfMood::paintEvent(QPaintEvent*)
{
#if 0
	QPainter p(this);
	p.fillRect(rect(), Qt::red);

	QRect r(globalRect());
	r.moveTo(rect().topLeft());
	p.fillRect(r, Qt::gray);
#endif
}

void YaSelfMood::setContactList(PsiContactList* contactList)
{
	extra()->setContactList(contactList);
}

XMPP::VCard::Gender YaSelfMood::gender() const
{
	return extra()->gender();
}

void YaSelfMood::setGender(XMPP::VCard::Gender gender)
{
	extra()->setGender(gender);
}

#include "yaselfmood.moc"
