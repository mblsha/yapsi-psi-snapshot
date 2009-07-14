/*
 * yachatview.cpp - custom chatlog widget
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

#include "yachatview.h"

#include <QScrollBar>
#include <QResizeEvent>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QApplication>
#include <QClipboard>
#include <QTextEdit>
#include <QDateTime>
#include <QPainter>

#include "yachatviewdelegate.h"
#include "smoothscrollbar.h"
#ifndef NO_SHORTCUTMANAGER
#include "shortcutmanager.h"
#endif
#ifndef NO_DESKTOPUTIL
#include "desktoputil.h"
#endif
#include "urlobject.h"
#include "psitooltip.h"
#include "yacommon.h"
#include "iconset.h"
#include "psioptions.h"
#include "shortcutmanager.h"

static const int SCROLL_SINGLE_STEP = 32;
static const int COMPOSING_PIXMAP_COUNT = 12;

static QPixmap composingPixmap(int index)
{
	if (index < 0 || index >= COMPOSING_PIXMAP_COUNT)
		return QPixmap();

	static QPixmap pix[COMPOSING_PIXMAP_COUNT];
	if (pix[index].isNull()) {
		QString fileName;
		if (index == 0) {
			fileName = ":images/chat/composing/keyboard.png";
		}
		else {
			fileName.sprintf(":images/chat/composing/keys%04d.png", index);
		}
		pix[index] = QPixmap(fileName);
	}
	Q_ASSERT(!pix[index].isNull());
	return pix[index];
}

static QPixmap faderPixmap()
{
	static QPixmap pix;
	if (pix.isNull()) {
		pix = QPixmap(":images/chat/fader.png");
	}
	Q_ASSERT(!pix.isNull());
	return pix;
}

YaChatView::YaChatView(QWidget* parent)
	: QListView(parent)
	, dialog_(0)
	, autoScrollCount_(0)
	, doubleClicked_(false)
	, hoveringLink_(false)
	, hoveringEmoticon_(false)
	, hoveringText_(false)
	, composingEventVisible_(false)
	, composingEventPixmapIndex_(0)
{
	QAbstractItemDelegate* oldDelegate = itemDelegate();
	delete oldDelegate;

	copyAction_ = new QAction(tr("&Copy"), this);
	copyAction_->setShortcut(QKeySequence("Ctrl+C"));
	connect(copyAction_, SIGNAL(activated()), SLOT(copy()));
	// addAction(copyAction_);
	copyAsHtmlAction_ = new QAction(tr("Copy as &HTML"), this);
	copyAsHtmlAction_->setShortcut(QKeySequence("Ctrl+H"));
	connect(copyAsHtmlAction_, SIGNAL(activated()), SLOT(copyAsHtml()));
	// addAction(copyAction_);
	selectAllAction_ = new QAction(tr("Select &All"), this);
	selectAllAction_->setShortcut(QKeySequence("Ctrl+A"));
	connect(selectAllAction_, SIGNAL(activated()), SLOT(selectAll()));
	// addAction(selectAllAction_);

	autoScrollTimer_ = new QTimer(this);
	autoScrollTimer_->setSingleShot(false);
	autoScrollTimer_->setInterval(50);
	connect(autoScrollTimer_, SIGNAL(timeout()), SLOT(autoScroll()));

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// setCursor(Qt::IBeamCursor);
	setMouseTracking(true);
	setSelectionMode(QAbstractItemView::NoSelection);

	setItemDelegate(new YaChatViewDelegate(this));
	delegate()->setParentFont(font());
	connect(delegate(), SIGNAL(updateRequest()), SLOT(updateRequest()));
	connect(delegate(), SIGNAL(linkHovered(QString)), SLOT(linkHovered(QString)));
	connect(delegate(), SIGNAL(linkActivated(QString)), SLOT(linkActivated(QString)));

	SmoothScrollBar::install(this, SmoothScrollBar::PreferScrollToBottom);

	composingEventTimer_ = new QTimer(this);
	composingEventTimer_->setSingleShot(false);
	composingEventTimer_->setInterval(100);
	connect(composingEventTimer_, SIGNAL(timeout()), SLOT(composingEventTimerTimeout()));

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
	optionChanged("options.shortcuts.chat.send");
}

YaChatView::~YaChatView()
{
}

/**
 * We're reimplementing doItemsLayout() in order to make sure that vertical
 * scrollbars' singleStep value stays the same even in case of relayouts
 */
void YaChatView::doItemsLayout()
{
	QListView::doItemsLayout();
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	verticalScrollBar()->setSingleStep(SCROLL_SINGLE_STEP);
}

void YaChatView::setModel(QAbstractItemModel* model)
{
	QListView::setModel(model);
	connect(model, SIGNAL(modelReset()), SLOT(modelReset()));
}

void YaChatView::resizeEvent(QResizeEvent *e)
{
	QListView::resizeEvent(e);
	updateDelegateWidth();
}

void YaChatView::updateDelegateWidth()
{
	int width = viewport()->width();
	if (!verticalScrollBar()->isVisible())
		width -= verticalScrollBar()->sizeHint().width();
	bool relayout = width != delegate()->width();
	delegate()->setWidth(width);
	if (relayout)
		scheduleDelayedItemsLayout();
}

void YaChatView::keyPressEvent(QKeyEvent* e)
{
	// TODO FIXME: duplicated from msgmle.cpp
	if(dialog_) {
		QKeySequence k(e->key() + (e->modifiers() & ~Qt::KeypadModifier));

#ifndef NO_SHORTCUTMANAGER
		// Temporary workaround for what i think is a Qt bug
		if(ShortcutManager::instance()->shortcuts("common.close").contains(k)
			|| ShortcutManager::instance()->shortcuts("message.send").contains(k)) {
			e->ignore();
			return;
		}
#endif

		// Ignore registered key sequences (and pass them up)
		foreach(QAction* act, dialog_->actions()) {
			foreach(QKeySequence keyseq, act->shortcuts()) {
				if(!keyseq.isEmpty() && keyseq.matches(k) == QKeySequence::ExactMatch) {
					e->ignore();
					//act->trigger();
					return;
				}
			}
		}
	}

	QAbstractSlider::SliderAction action = QAbstractSlider::SliderNoAction;
	switch (e->key()) {
	case Qt::Key_Up:
		action = QAbstractSlider::SliderSingleStepSub;
		break;
	case Qt::Key_Down:
		action = QAbstractSlider::SliderSingleStepAdd;
		break;
	case Qt::Key_PageUp:
		action = QAbstractSlider::SliderPageStepSub;
		break;
	case Qt::Key_PageDown:
		action = QAbstractSlider::SliderPageStepAdd;
		break;
	default:
		;
	}
	verticalScrollBar()->triggerAction(action);
}

void YaChatView::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
		return;

	// TODO FIXME: clearing should only be performed only if clicked on
	// empty area otherwise press-hold-move should be used for drag-n-drop.

	doubleClicked_ = false;
	handleMousePress(e);
}

void YaChatView::clearSelectedIndices()
{
	foreach(QModelIndex index, selectedIndices_) {
		deselectIndex(index);
	}
	selectedIndices_.clear();
	selectedIndex_ = QModelIndex();
}

void YaChatView::handleMousePress(QMouseEvent* e)
{
	clearSelectedIndices();

	QModelIndex index = indexAt(e->pos());
	mousePressPosition_ = e->pos() - visualRect(index).topLeft();
	mousePressGlobalPosition_ = e->globalPos();
	mouseMoveLastGlobalPosition_ = mousePressGlobalPosition_;
	autoScrollCount_ = 0;

	if (index.isValid()) {
		// for (int i = 0; i < doubleClicked_ ? 2 : 1; ++i)
			sendControlEvent(index, e, visualRect(index));

		selectedIndex_ = index;
		selectedIndices_.append(index);
	}
}

void YaChatView::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
		return;

	doubleClicked_ = true;
	handleMousePress(e);
}

static const int AUTO_SCROLL_MARGIN = 32;

bool YaChatView::autoScrollToBottom() const
{
	return !mousePressGlobalPosition_.isNull() &&
	       mouseMoveLastGlobalPosition_.y() > (viewport()->mapToGlobal(viewport()->rect().bottomLeft()).y() - AUTO_SCROLL_MARGIN);
}

bool YaChatView::autoScrollToTop() const
{
	return !mousePressGlobalPosition_.isNull() &&
	       mouseMoveLastGlobalPosition_.y() < (viewport()->mapToGlobal(viewport()->rect().topLeft()).y() + AUTO_SCROLL_MARGIN);
}

void YaChatView::autoScroll()
{
	Q_ASSERT(autoScrollToBottom() || autoScrollToTop());
	if (!autoScrollToBottom() && !autoScrollToTop())
		return;

	if (autoScrollCount_ >= 0 && autoScrollToBottom()) {
		autoScrollCount_ = qMin(autoScrollCount_ + 1, verticalScrollBar()->pageStep());
	}
	else if (autoScrollCount_ <= 0 && autoScrollToTop()) {
		autoScrollCount_ = -qMin(qAbs(autoScrollCount_ - 1), verticalScrollBar()->pageStep());
	}
	else {
		autoScrollCount_ = 0;
	}

	int value = verticalScrollBar()->value() + autoScrollCount_;

	SmoothScrollBar* smoothScrollBar = dynamic_cast<SmoothScrollBar*>(verticalScrollBar());
	if (smoothScrollBar)
		smoothScrollBar->setValueImmediately(value);
	else
		verticalScrollBar()->setValue(value);

	QMouseEvent fakeMove(QEvent::MouseMove,
	                     viewport()->mapFromGlobal(mouseMoveLastGlobalPosition_),
	                     mouseMoveLastGlobalPosition_,
	                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	qApp->sendEvent(viewport(), &fakeMove);
}

void YaChatView::mouseMoveEvent(QMouseEvent* e)
{
	mouseMoveLastGlobalPosition_ = e->globalPos();

	QModelIndex currentIndex = indexAt(e->pos());
	{
		QPoint delegatePoint = e->pos() - visualRect(currentIndex).topLeft();
		hoveringText_ = currentIndex.isValid() &&
		                delegate()->isHoveringText(currentIndex, delegatePoint);
		hoveringEmoticon_ = currentIndex.isValid() &&
		                    delegate()->isHoveringEmoticon(currentIndex, delegatePoint);
		QTimer::singleShot(0, this, SLOT(updateCursor()));
	}

	if (!(e->buttons() & Qt::LeftButton)) {
		// Update cursor when hovering hyperlinks
		sendControlEvent(currentIndex, e, visualRect(currentIndex));
		return;
	}

	if (autoScrollToBottom() || autoScrollToTop()) {
		autoScrollTimer_->start();
	}
	else {
		autoScrollTimer_->stop();
	}

	int currentIndexRow = currentIndex.row();
	if (!currentIndex.isValid()) {
		currentIndexRow = model()->rowCount(QModelIndex()) - 1;
		if (e->pos().y() < 0)
			currentIndexRow = 0;
	}

	int selectedIndexRow = selectedIndex_.row();
	if (!selectedIndex_.isValid())
		selectedIndexRow = model()->rowCount(QModelIndex());

	int start = qMin(currentIndexRow, selectedIndexRow);
	int end   = qMax(currentIndexRow, selectedIndexRow);
	QList<QModelIndex> selected;
	for (int i = start; i <= end; ++i) {
		QModelIndex index = this->index(i);
		selected << index;

		if (!selectedIndices_.contains(index)) {
			selectedIndices_.append(index);
		}
	}

	foreach(QModelIndex index, selectedIndices_) {
		if (!selected.contains(index)) {
			deselectIndex(index);
			selectedIndices_.removeAll(index);
		}
	}

	QPoint startPoint = visualRect(selectedIndex_).topLeft() + mousePressPosition_;
	QPoint endPoint   = e->pos();

	if (startPoint.y() > endPoint.y()) {
		QPoint tmp = startPoint;
		startPoint = endPoint;
		endPoint = tmp;
	}

	if (start == end) {
		QRect rect = visualRect(selectedIndex_);
		selectIndex(selectedIndex_, startPoint - rect.topLeft(), endPoint - rect.topLeft());
	}
	else {
		int i = start;
		foreach(QModelIndex index, selected) {
			QRect vrect = visualRect(index);
			QPoint sp, ep;
			fullSelectionPoints(index, &sp, &ep);

			if (i == start) {
				sp = startPoint - vrect.topLeft();
			}

			if (i == end) {
				ep = endPoint - vrect.topLeft();
			}

			selectIndex(index, sp, ep);
			++i;
		}
	}

#ifdef Q_WS_X11
	if (hasSelectedText())
		QApplication::clipboard()->setText(selectedText(), QClipboard::Selection);
#endif
}

void YaChatView::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
		return;

	autoScrollTimer_->stop();

	QModelIndex index = indexAt(e->pos());
	if (index.isValid()) {
		sendControlEvent(index, e, visualRect(index));
	}
}

void YaChatView::sendControlEvent(QMouseEvent* e)
{
	QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(e);
	Q_ASSERT(mouseEvent);
	QModelIndex index = indexAt(mouseEvent->pos());
	sendControlEvent(index, mouseEvent, visualRect(index));
	if (!selectedIndices_.contains(index))
		selectedIndices_.append(index);
}

QModelIndex YaChatView::index(int row) const
{
	return model()->index(row, 0, QModelIndex());
}

QRect YaChatView::visualRect(const QModelIndex& index) const
{
	QRect rect = QListView::visualRect(index);
	if (!index.isValid()) {
		int lastRow = model()->rowCount(QModelIndex()) - 1;
		QModelIndex lastIndex = this->index(lastRow);
		if (lastIndex.isValid()) {
			rect.setTopLeft(visualRect(lastIndex).bottomLeft());
		}
	}
	return rect;
}

QRect YaChatView::rect(const QModelIndex& index) const
{
	QRect rect = visualRect(index);
	rect.moveTo(0, 0);
	return rect;
}

void YaChatView::sendControlEvent(const QModelIndex& index, QMouseEvent* e, const QRect& indexRect)
{
	delegate()->sendControlEvent(index, e, -indexRect.topLeft(), this);
}

void YaChatView::deselectIndex(const QModelIndex& index)
{
	QRect rect = this->rect(index);
	QMouseEvent fakePress(QEvent::MouseButtonPress, QPoint(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	sendControlEvent(index, &fakePress, rect);

	QMouseEvent fakeRelease(QEvent::MouseButtonRelease, QPoint(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	sendControlEvent(index, &fakeRelease, rect);
}

void YaChatView::selectIndex(const QModelIndex& index, const QPoint& startPoint, const QPoint& endPoint)
{
	deselectIndex(index);
	QRect rect = this->rect(index);

	QMouseEvent fakePress(QEvent::MouseButtonPress, startPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	// for (int i = 0; i < doubleClicked_ ? 2 : 1; ++i)
		sendControlEvent(index, &fakePress, rect);

	QMouseEvent fakeMove(QEvent::MouseMove, endPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	sendControlEvent(index, &fakeMove, rect);

	QMouseEvent fakeRelease(QEvent::MouseButtonRelease, endPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	sendControlEvent(index, &fakeRelease, rect);
}

YaChatViewDelegate* YaChatView::delegate() const
{
	return static_cast<YaChatViewDelegate*>(itemDelegate());
}

void YaChatView::updateRequest()
{
	// TODO: need to link DelegateMessageData with QPersistentModelIndex
	scheduleDelayedItemsLayout();
}

/**
 * Ripped from PsiTextView
 * Ensures that if YaChatView was scrolled to bottom when resize
 * operation happened, it will still be scrolled to bottom after the fact.
 */
void YaChatView::updateGeometries()
{
	bool atEnd = verticalScrollBar()->value() ==
	             verticalScrollBar()->maximum();
	bool atStart = verticalScrollBar()->value() ==
	               verticalScrollBar()->minimum();
	double value = 0;
	if (!atEnd && !atStart)
		value = (double)verticalScrollBar()->maximum() /
		        (double)verticalScrollBar()->value();

	QListView::updateGeometries();

	if (atEnd)
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	else if (value != 0)
		verticalScrollBar()->setValue((int) ((double)verticalScrollBar()->maximum() / value));
}

void YaChatView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	Q_ASSERT(!parent.isValid()); // otherwise we couldn't use index(i) here
	QListView::rowsAboutToBeRemoved(parent, start, end);

	for (int i = start; i <= end; ++i) {
		delegate()->indexAboutToBeRemoved(index(i));
	}
}

void YaChatView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	QListView::rowsInserted(parent, start, end);
}

void YaChatView::setDialog(QWidget* dialog)
{
	dialog_ = dialog;
}

bool YaChatView::hasSelectedText() const
{
	return selectedIndices_.count() > 0;
}

/**
 * Scrolls the vertical scroll bar to its maximum position i.e. to the bottom.
 */
void YaChatView::scrollToBottom()
{
	if (verticalScrollBar()->isSliderDown())
		return;

	SmoothScrollBar* smoothScrollBar = dynamic_cast<SmoothScrollBar*>(verticalScrollBar());
	if (smoothScrollBar)
		smoothScrollBar->setValueImmediately(verticalScrollBar()->maximum());
	else
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

/**
 * Scrolls the vertical scroll bar to its minimum position i.e. to the top.
 */
void YaChatView::scrollToTop()
{
	if (verticalScrollBar()->isSliderDown())
		return;

	verticalScrollBar()->setValue(verticalScrollBar()->minimum());
}

bool YaChatView::handleCopyEvent(QObject* object, QEvent* event, QTextEdit* chatEdit)
{
	if (object == chatEdit && event->type() == QEvent::KeyPress) {
		QKeyEvent *e = (QKeyEvent *)event;
		if ((e->key() == Qt::Key_C && (e->modifiers() & Qt::ControlModifier)) ||
		    (e->key() == Qt::Key_Insert && (e->modifiers() & Qt::ControlModifier)))
		{
			if (!chatEdit->textCursor().hasSelection() &&
			     hasSelectedText())
			{
				this->copy();
				return true;
			}
		}
	}

	return false;
}

QString YaChatView::formatTimeStamp(const QDateTime& timeStamp) const
{
	Q_UNUSED(timeStamp);
	return QString();
}

void YaChatView::nicksChanged(QString selfNick, QString contactNick)
{
	if (delegate()->selfNick() != selfNick || delegate()->contactNick() != contactNick) {
		delegate()->nicksChanged(selfNick, contactNick);
		scheduleDelayedItemsLayout();
		viewport()->update();
	}
}

void YaChatView::linkHovered(QString link)
{
	hoveringLink_ = !link.isEmpty();
	link_ = link;
	updateCursor();
}

void YaChatView::linkActivated(QString link)
{
	linkHovered(QString());

	clearSelectedIndices();
	mousePressPosition_ = QPoint();
	mousePressGlobalPosition_ = QPoint();
	mouseMoveLastGlobalPosition_ = QPoint();
	autoScrollCount_ = 0;
#ifndef NO_DESKTOPUTIL
	DesktopUtil::openUrl(link);
#endif
}

void YaChatView::updateCursor()
{
	Qt::CursorShape shape = Qt::ArrowCursor;

	if (hoveringLink_ /*|| hoveringEmoticon_*/)
		shape = Qt::PointingHandCursor;
	else if (hoveringText_)
		shape = Qt::IBeamCursor;

	if (shape != Qt::ArrowCursor)
		viewport()->setCursor(shape);
	else
		viewport()->unsetCursor();
}

void YaChatView::contextMenuEvent(QContextMenuEvent* e)
{
	QMenu* menu;
	if (!link_.isEmpty()) {
		menu = URLObject::getInstance()->createPopupMenu(link_);
	}
	else {
		menu = new QMenu();
		menu->setProperty("hide-shortcuts", true);
		menu->addAction(copyAction_);
		menu->addAction(copyAsHtmlAction_);
		// menu->insertSeparator();
		menu->addAction(selectAllAction_);
	}
	menu->exec(e->globalPos());
	e->accept();
	delete menu;

	linkHovered(QString());
}

QString YaChatView::selectedText()
{
	qSort(selectedIndices_.begin(), selectedIndices_.end());

	QStringList text;
	foreach(QModelIndex index, selectedIndices_) {
		text += delegate()->selectedText(index, selectedIndices_.count() > 1 ?
		                                 YaChatViewDelegate::SelectedTextWithHeader :
		                                 YaChatViewDelegate::SelectedTextOnly);
	}
	return text.join("\n");
}

QString YaChatView::selectedHtml()
{
	qSort(selectedIndices_.begin(), selectedIndices_.end());

	QStringList text;
	foreach(QModelIndex index, selectedIndices_) {
		text += delegate()->selectedHtml(index, selectedIndices_.count() > 1 ?
		                                 YaChatViewDelegate::SelectedTextWithHeader :
		                                 YaChatViewDelegate::SelectedTextOnly);
	}
	return text.join("\n");
}

void YaChatView::copy()
{
	if (hasSelectedText())
		QApplication::clipboard()->setText(selectedText());
}

void YaChatView::copyAsHtml()
{
	if (hasSelectedText()) {
		QApplication::clipboard()->setText(selectedHtml());
	}
}

void YaChatView::fullSelectionPoints(const QModelIndex& index, QPoint* sp, QPoint* ep) const
{
	Q_ASSERT(sp);
	Q_ASSERT(ep);
	QRect rect  = this->rect(index);
	*sp = rect.topLeft();
	*ep = rect.bottomRight();
}

void YaChatView::selectAll()
{
	clearSelectedIndices();

	for (int i = 0; i < model()->rowCount(); ++i) {
		const QModelIndex& index = this->index(i);
		QPoint sp, ep;
		fullSelectionPoints(index, &sp, &ep);

		selectedIndices_ += index;
		selectIndex(index, sp, ep);
	}
}

void YaChatView::changeEvent(QEvent* e)
{
	QListView::changeEvent(e);
	if (e->type() == QEvent::FontChange) {
		delegate()->setParentFont(font());
		scheduleDelayedItemsLayout();
	}
}

bool YaChatView::event(QEvent* event)
{
	if (event->type() == QEvent::ToolTip) {
		QPoint pos = dynamic_cast<QHelpEvent *>(event)->globalPos();
		QPoint localPos = dynamic_cast<QHelpEvent *>(event)->pos();
		QString toolTip;

		QModelIndex currentIndex = indexAt(localPos);
		QPoint delegatePoint = localPos - visualRect(currentIndex).topLeft();
		if (currentIndex.isValid()) {
			if (delegate()->isHoveringTime(currentIndex, delegatePoint)) {
				QDateTime dateTime = currentIndex.data(YaChatViewModel::DateTimeRole).toDateTime();
				toolTip = QString("<div style='white-space:pre'>%1</div> <div style='white-space:pre'>%2</div>")
				          .arg(dateTime.toString("hh:mm:ss"))
				          .arg(Ya::DateFormatter::instance()->dateAndWeekday(dateTime.date()));
			}
			else if (delegate()->isHoveringEmoticon(currentIndex, delegatePoint)) {
				QString text = delegate()->messageEmoticonTextUnder(currentIndex, delegatePoint);
				QString iconName = delegate()->messageEmoticonNameUnder(currentIndex, delegatePoint);
				const PsiIcon* icon = IconsetFactory::iconPtr(iconName);
				if (icon) {
					toolTip = Ya::emoticonToolTip(icon, text);
				}
				else {
					toolTip = text;
				}
			}
			else if (delegate()->isHoveringDeliveryConfirmation(currentIndex, delegatePoint)) {
				YaChatViewModel::DeliveryConfirmationType deliveryConfirmation = static_cast<YaChatViewModel::DeliveryConfirmationType>(currentIndex.data(YaChatViewModel::DeliveryConfirmationRole).toInt());
				if (deliveryConfirmation != YaChatViewModel::DeliveryConfirmation_Unknown) {
					QString error = currentIndex.data(YaChatViewModel::ErrorRole).toString().toLower();

					switch (deliveryConfirmation) {
					case YaChatViewModel::DeliveryConfirmation_Querying:
						break;
					case YaChatViewModel::DeliveryConfirmation_Verified:
						toolTip = tr("<div style='white-space:pre'>Message delivered</div>");
						break;
					case YaChatViewModel::DeliveryConfirmation_Timeout:
						error = tr("Timeout").toLower();
					case YaChatViewModel::DeliveryConfirmation_Error:
						toolTip = tr("<div style='white-space:pre'>Error delivering message:</div><br>\n%1.")
						          .arg(error);
						break;
					default:
						;
					}
				}
			}
		}

		PsiToolTip::showText(pos, toolTip, this);
		return true;
	}

	return QListView::event(event);
}

QSize YaChatView::minimumSizeHint() const
{
	QSize sh = QListView::minimumSizeHint();
	sh.setHeight(128);
	return sh;
}

QSize YaChatView::sizeHint() const
{
	return minimumSizeHint();
}

void YaChatView::modelReset()
{
	selectedIndices_.clear();
	selectedIndex_ = QModelIndex();
}

QColor YaChatView::textColor() const
{
	return delegate()->textColor();
}

void YaChatView::setTextColor(const QColor& textColor)
{
	delegate()->setTextColor(textColor);
}

bool YaChatView::composingEventVisible() const
{
	return composingEventVisible_;
}

void YaChatView::setComposingEventVisible(bool visible)
{
	if (!visible) {
		composingEventPixmapIndex_ = 0;
		composingEventTimer_->stop();
	}
	else {
		composingEventTimer_->start();
	}
	composingEventVisible_ = visible;
	viewport()->update();
}

QRect YaChatView::composingEventRect() const
{
	QRect r(0, 0, 40, 18);
	r.moveTo(viewport()->rect().right() - r.width() - 2, viewport()->rect().bottom() - r.height() - 0);
	return r;
}

QRect YaChatView::faderRect()
{
	QRect r(viewport()->rect());
	r.setHeight(faderPixmap().height());
	return r;
}

void YaChatView::paintEvent(QPaintEvent* e)
{
	QListView::paintEvent(e);

	if (composingEventVisible_) {
		QPainter p(viewport());

		QRect r = composingEventRect();
		p.drawPixmap(r.topLeft(), composingPixmap(0));

		if (composingEventPixmapIndex_) {
			p.drawPixmap(r.topLeft(), composingPixmap(composingEventPixmapIndex_));
		}
	}

	{
		QPainter p(viewport());
		QRect r = faderRect();
		p.drawTiledPixmap(r, faderPixmap());
	}
}

void YaChatView::scrollContentsBy(int dx, int dy)
{
	if (composingEventVisible_) {
		setDirtyRegion(composingEventRect());
		// setDirtyRegion(composingEventRect().translated(dx, dy));
	}
	setDirtyRegion(faderRect());
	// setDirtyRegion(faderRect().translated(dx, dy));

	QListView::scrollContentsBy(dx, dy);

	// workaround for ONLINE-2049
	QRect r(viewport()->rect());
	if (dy > 0) {
		r = QRect(r.left(), r.top(), r.width(), dy);
	}
	else {
		r = QRect(r.left(), r.bottom() + dy, r.width(), qAbs(dy));
	}
	setDirtyRegion(r);
}

void YaChatView::composingEventTimerTimeout()
{
	composingEventPixmapIndex_++;
	if (composingEventPixmapIndex_ >= COMPOSING_PIXMAP_COUNT) {
		composingEventPixmapIndex_ = 1;
	}
	viewport()->update();
}

void YaChatView::optionChanged(const QString& option)
{
	if (option == "options.shortcuts.chat.send") {
		delegate()->setSendShortcutText(ShortcutManager::instance()->shortcut("chat.send").toString(QKeySequence::NativeText));
	}
}
