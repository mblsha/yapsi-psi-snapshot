/*
 * yachatseparator.cpp
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

#include "yachatseparator.h"

#include <QTimeLine>
#include <QPainter>
#include <QToolButton>
#include <QToolBar>
#include <QAction>
#include <QLayout>
#include <QStyle>
#include <QStyleOption>
#include <QPlastiqueStyle>
#include <QMouseEvent>
#include <QApplication>
#include <QStyleOptionToolButton>
#include <QCursor>

#include "iconset.h"
#include "yaclosebutton.h"
#include "psitooltip.h"
#include "yacommon.h"
#include "psioptions.h"
#include "msgmle.h"
#include "yapushbutton.h"
#include "yavisualutil.h"
#include "yachevronbutton.h"

// #define YCS_CHEVRON_BUTTON

static const int expandDuration = 200;

static const QString emoticonsPanelEnabledOptionPath = "options.ya.chat-window.emoticons-enabled";
static const QString editorHeightOptionPath = "options.ya.chat-window.editor-height";
static const QString enableTypographyOptionPath = "options.ya.typography.enable";

static QPixmap buttonBackgroundLeft(bool pressed)
{
	static QPixmap pix;
	static QPixmap pix_pressed;

	if (pressed) {
		if (pix_pressed.isNull()) {
			pix_pressed = QPixmap(":images/chat/button_pressed.png");
		}
		return pix_pressed;
	}

	if (pix.isNull()) {
		pix = QPixmap(":images/chat/button.png");
	}
	return pix;
}

static QPixmap buttonBackground(bool pressed)
{
	static QPixmap pix;
	static QPixmap pix_pressed;

	if (pressed) {
		if (pix_pressed.isNull()) {
			pix_pressed = QPixmap(":images/chat/button_pressed_background.png");
		}
		return pix_pressed;
	}

	if (pix.isNull()) {
		pix = QPixmap(":images/chat/emoticon_background.png");
	}
	return pix;
}

static QPixmap emoticonOverlay(bool hover)
{
	static QPixmap pix;
	static QPixmap pix_hover;

	if (hover) {
		if (pix_hover.isNull()) {
			pix_hover = QPixmap(":images/chat/emoticon_overlay_hover.png");
		}
		return pix_hover;
	}

	if (pix.isNull()) {
		pix = QPixmap(":images/chat/emoticon_overlay.png");
	}
	return pix;
}

static QPixmap typoButton(bool enabled, bool pressed)
{
	static QPixmap pix_enabled;
	static QPixmap pix_enabled_pressed;
	static QPixmap pix_disabled;
	static QPixmap pix_disabled_pressed;

	if (enabled) {
		if (pressed) {
			if (pix_enabled_pressed.isNull()) {
				pix_enabled_pressed = QPixmap(":images/chat/typo_enabled_hover.png");
			}
			return pix_enabled_pressed;
		}
		else {
			if (pix_enabled.isNull()) {
				pix_enabled = QPixmap(":images/chat/typo_enabled.png");
			}
			return pix_enabled;
		}
	}
	else {
		if (pressed) {
			if (pix_disabled_pressed.isNull()) {
				pix_disabled_pressed = QPixmap(":images/chat/typo_disabled_hover.png");
			}
			return pix_disabled_pressed;
		}
		else {
			if (pix_disabled.isNull()) {
				pix_disabled = QPixmap(":images/chat/typo_disabled.png");
			}
			return pix_disabled;
		}
	}

	return QPixmap();
}

//----------------------------------------------------------------------------
// YaChatSeparatorPushButton
//----------------------------------------------------------------------------

YaChatSeparatorPushButton::YaChatSeparatorPushButton(const QString& text, QWidget* parent)
	: QPushButton(text, parent)
{
	setFocusPolicy(Qt::NoFocus);
}

QSize YaChatSeparatorPushButton::sizeHint() const
{
	return QSize(fontMetrics().width(text()) + 10 * 2, 16);
}

void YaChatSeparatorPushButton::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	QPixmap background = buttonBackground(isDown());
	p.drawTiledPixmap(rect(), background);

	QPixmap left = buttonBackgroundLeft(isDown());
	p.drawPixmap(0, 0, left);

	drawForeground(&p);
}

void YaChatSeparatorPushButton::drawForeground(QPainter* p)
{
	// p->setPen(down || underMouse() ? QColor(0x27, 0x5A, 0x07) : Qt::black);
	p->setPen(Qt::black);

	QRect textRect(rect());
	if (isDown())
		textRect.adjust(0, 1, 0, 1);
	p->drawText(textRect, Qt::AlignCenter | Qt::AlignVCenter, text());
}

//----------------------------------------------------------------------------
// YaChatSeparatorOverlayButton
//----------------------------------------------------------------------------

YaChatSeparatorOverlayButton::YaChatSeparatorOverlayButton(QWidget* parent)
	: QPushButton(parent)
{
	setAttribute(Qt::WA_Hover, true);
}

YaChatSeparatorOverlayButton::~YaChatSeparatorOverlayButton()
{
}

void YaChatSeparatorOverlayButton::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.drawPixmap(rect(), emoticonOverlay(underMouse()));
}

//----------------------------------------------------------------------------
// YaChatSeparatorTypoButton
//----------------------------------------------------------------------------

YaChatSeparatorTypoButton::YaChatSeparatorTypoButton(QWidget* parent)
	: YaChatSeparatorPushButton(QString(), parent)
	, typoEnabled_(true)
{
	setToolTip(tr("Typographica"));
	PsiToolTip::install(this);
}

QSize YaChatSeparatorTypoButton::sizeHint() const
{
	QSize sh = YaChatSeparatorPushButton::sizeHint();
	sh.setWidth(typoButton(true, false).width() + 7);
	return sh;
}

void YaChatSeparatorTypoButton::drawForeground(QPainter* p)
{
	QPixmap pix = typoButton(typoEnabled(), isDown() || underMouse());

	QPoint point((rect().width() - pix.width())/2, (rect().height() - pix.height())/2);
	if (isDown())
		point += QPoint(0, 1);
	p->drawPixmap(point, pix);
}

bool YaChatSeparatorTypoButton::typoEnabled()
{
	return typoEnabled_;
}

void YaChatSeparatorTypoButton::setTypoEnabled(bool enabled)
{
	typoEnabled_ = enabled;
	update();
}

//----------------------------------------------------------------------------
// YaChatEmoticonToolBarStyle
//----------------------------------------------------------------------------

class YaChatEmoticonToolBarStyle : public QPlastiqueStyle
{
public:
	YaChatEmoticonToolBarStyle(QStyle* defaultStyle)
		: defaultStyle_(defaultStyle)
	{
#ifdef YCS_CHEVRON_BUTTON
		chevron_ = new YaChevronButton(0);
		chevron_->hide();
#endif
	}

	~YaChatEmoticonToolBarStyle()
	{
	}

	// reimplemented
	void polish(QWidget* widget)
	{
		defaultStyle()->polish(widget);
	}

	void unpolish(QWidget* widget)
	{
		defaultStyle()->unpolish(widget);
	}

	QSize sizeFromContents(ContentsType ct, const QStyleOption* opt, const QSize& contentsSize, const QWidget* w) const
	{
		if (ct == CT_ToolButton &&
		    dynamic_cast<const YaChatEmoticonToolBar*>(w->parentWidget()))
		{
			const QToolButton* toolButton = dynamic_cast<const QToolButton*>(w);
			if (toolButton && toolButton->objectName() != QLatin1String("qt_toolbar_ext_button")) {
				QSize maxSize(100, static_cast<const YaChatEmoticonToolBar*>(w->parentWidget())->iconSize().height());
				int spacing = 1;
				return QSize(toolButton->icon().actualSize(maxSize).width() + 2 + spacing, maxSize.height());
			}
			else if (toolButton) {
				QSize size = QPlastiqueStyle::sizeFromContents(ct, opt, contentsSize, w);
				size.setWidth(size.width() + 10);
				return size;
			}
		}

		return QPlastiqueStyle::sizeFromContents(ct, opt, contentsSize, w);
	}

	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
	{
		switch (metric) {
		case PM_ToolBarExtensionExtent: {
#ifdef YCS_CHEVRON_BUTTON
			return chevron_->sizeHint().width() + 4;
#else
			int ext = QPlastiqueStyle::pixelMetric(metric, option, widget);
			return ext;
#endif
		}
		case PM_ToolBarItemMargin:
		case PM_ToolBarFrameWidth:
		case PM_ToolBarItemSpacing:
			if (dynamic_cast<const YaChatEmoticonToolBar*>(widget))
				return 0;
			break;
		default:
			break;
		}
		return QPlastiqueStyle::pixelMetric(metric, option, widget);
	}

	void drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		switch (ce) {
		case CE_ToolButtonLabel:
			if (dynamic_cast<const YaChatEmoticonToolBar*>(w->parentWidget())) {
				const QToolButton* toolButton = dynamic_cast<const QToolButton*>(w);
				if (toolButton) {
					QRect paintRect = opt->rect;
					paintRect.adjust(-paintRect.left(), -paintRect.top(), 2*paintRect.left(), paintRect.top());
					paintRect.translate(1, 0);
					toolButton->icon().paint(p, paintRect, Qt::AlignLeft | Qt::AlignVCenter);
					return;
				}
			}
			break;
		default:
			;
		}

		defaultStyle()->drawControl(ce, opt, p, w);
	}

	void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		if (pe == PE_IndicatorArrowDown || pe == PE_IndicatorButtonDropDown)
			return;

		if (pe == PE_PanelButtonTool) {
			QPlastiqueStyle::drawPrimitive(pe, opt, p, w);
			return;
		}

		defaultStyle()->drawPrimitive(pe, opt, p, w);
	}

	void drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* p, const QWidget* widget) const
	{
		if (cc == CC_ToolButton) {
#ifdef YCS_CHEVRON_BUTTON
			const QToolButton* toolButton = dynamic_cast<const QToolButton*>(widget);
			if (toolButton && toolButton->objectName() == QLatin1String("qt_toolbar_ext_button")) {
				QRect r = opt->rect;
				p->drawPixmap(r.left() + ((r.width() - chevron_->sizeHint().width()) / 2),
				              r.top()  + ((r.height() - chevron_->sizeHint().height()) / 2) + 1,
				              chevron_->buttonPixmap(toolButton->underMouse() || toolButton->isDown()));
				return;
			}
#else
			const QStyleOptionToolButton* toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
			Q_ASSERT(toolbutton);
			QStyleOptionToolButton o = toolbutton ? *toolbutton : QStyleOptionToolButton();
			if (widget->objectName() == QLatin1String("qt_toolbar_ext_button")) {
				o.rect.setWidth(o.rect.width() - 1);
			}
#endif

			QPlastiqueStyle::drawComplexControl(cc, &o, p, widget);
			return;
		}

		defaultStyle()->drawComplexControl(cc, opt, p, widget);
	}

	SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, const QPoint& pos, const QWidget* widget) const
	{
		if (cc == CC_ToolButton) {
			return QPlastiqueStyle::hitTestComplexControl(cc, opt, pos, widget);
		}

		return defaultStyle()->hitTestComplexControl(cc, opt, pos, widget);
	}

	QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt, SubControl sc, const QWidget* widget) const
	{
		if (cc == CC_ToolButton) {
			return QPlastiqueStyle::subControlRect(cc, opt, sc, widget);
		}

		return defaultStyle()->subControlRect(cc, opt, sc, widget);
	}

	int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
	{
		return defaultStyle()->styleHint(hint, option, widget, returnData);
	}

	QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
	{
		return defaultStyle()->subElementRect(element, option, widget);
	}

	void drawItemText(QPainter* painter, const QRect& rectangle, int alignment, const QPalette& palette, bool enabled, const QString& text, QPalette::ColorRole textRole) const
	{
		defaultStyle()->drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
	}

private:
	QStyle* defaultStyle_;
	QStyle* defaultStyle() const { return defaultStyle_; }

#ifdef YCS_CHEVRON_BUTTON
	YaChevronButton* chevron_;
#endif
};

//----------------------------------------------------------------------------
// YaChatEmoticonToolBar
//----------------------------------------------------------------------------

YaChatEmoticonToolBar::YaChatEmoticonToolBar(QWidget* parent)
	: QToolBar(parent)
{
	style_ = new YaChatEmoticonToolBarStyle(qApp->style());
	setStyle(style_);
	setIconSize(QSize(18,18));
	setToolButtonStyle(Qt::ToolButtonIconOnly);
#ifndef YAPSI_NO_STYLESHEETS
	setStyleSheet(
	"*::down-arrow, *::menu-indicator {"
	"    image: none;"
	"    width: 0px;"
	"    height: 0px;"
	"}"
	);
#endif
}

YaChatEmoticonToolBar::~YaChatEmoticonToolBar()
{
	delete style_;
}

void YaChatEmoticonToolBar::setIconset(const Iconset &iconset)
{
	QListIterator<PsiIcon *> it = iconset.iterator();
	while (it.hasNext()) {
		PsiIcon *icon = it.next();
		if (!icon->isUserSelectable())
			continue;

		QAction* action = new QAction(this);
		action->setIcon(icon->icon());
		action->setProperty("icon-text", icon->defaultText());
		action->setText(Ya::emoticonToolTipSimple(icon));
		action->setToolTip(Ya::emoticonToolTip(icon));
		connect(action, SIGNAL(activated()), SLOT(actionActivated()));
		addAction(action);
	}

	foreach(QToolButton* button, findChildren<QToolButton*>()) {
		PsiToolTip::install(button);
		button->setCursor(Qt::PointingHandCursor);
	}
}

void YaChatEmoticonToolBar::actionActivated()
{
	QAction* action = static_cast<QAction*>(sender());
	QString text = action->property("icon-text").toString();
	emit textSelected(text);
}

//----------------------------------------------------------------------------
// YaChatSeparatorExtra
//----------------------------------------------------------------------------

YaChatSeparatorExtra::YaChatSeparatorExtra(QWidget* parent, YaChatSeparator* overlayParent)
	: QWidget(parent)
	, addButton_(0)
	, authButton_(0)
	, typographyButton_(0)
	, state_(Normal)
	, overlayParent_(overlayParent)
{
	expandCollapseTimeLine_ = new QTimeLine(expandDuration, this);
	connect(expandCollapseTimeLine_, SIGNAL(frameChanged(int)), SLOT(animate()));

	closeButton_ = new YaCloseButton(this);
	closeButton_->setFocusPolicy(Qt::NoFocus);
	closeButton_->setChatButton(true);
	connect(closeButton_, SIGNAL(clicked()), SLOT(collapse()));

	toolBar_ = new YaChatEmoticonToolBar(this);
	connect(toolBar_, SIGNAL(textSelected(QString)), SIGNAL(textSelected(QString)));

	pixmap_ = emoticonOverlay(false);
	background_ = buttonBackground(false);

	overlayButton_ = new YaChatSeparatorOverlayButton(this);
	connect(overlayButton_, SIGNAL(clicked()), SLOT(overlayClicked()));

	addButton_ = new YaChatSeparatorPushButton(tr("Add"), this);
	connect(addButton_, SIGNAL(clicked()), SIGNAL(addContact()));
	addButton_->hide();
	authButton_ = new YaChatSeparatorPushButton(tr("Auth"), this);
	connect(authButton_, SIGNAL(clicked()), SIGNAL(authContact()));
	authButton_->hide();
	// typographyButton_ = new YaChatSeparatorTypoButton(this);
	// connect(typographyButton_, SIGNAL(clicked()), SIGNAL(typographyToggled()));
	// typographyButton_->show();

	installEventFilter(this);
}

void YaChatSeparatorExtra::setTypographyEnabled(bool enabled)
{
	if (!typographyButton_)
		return;
	typographyButton_->setTypoEnabled(enabled);
}

void YaChatSeparatorExtra::setExpanded(bool expanded)
{
	setState(expanded ? Expanded : Normal);
}

bool YaChatSeparatorExtra::isExpanded() const
{
	return state_ != Normal;
}

QSize YaChatSeparatorExtra::sizeHint() const
{
	QSize sh = pixmap_.size();
	switch (state_) {
	case Normal:
		sh.setWidth(normalWidth());
		break;
	case Expanded:
		sh.setWidth(expandedWidth());
		break;
	case Expanding:
	case Collapsing:
		sh.setWidth(expandCollapseTimeLine_->currentFrame());
		break;
	}

	return sh;
}

QRect YaChatSeparatorExtra::pixmapRect() const
{
	// QRect r = rect().adjusted(expandCollapseTimeLine_->currentFrame(), 0, 0, 0);
	QRect r = rect().adjusted(0, 0, 0, 0);
	r.setWidth(pixmap_.width());
	return r;
}

void YaChatSeparatorExtra::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	// p.fillRect(rect(), Qt::red);

	QRect backgroundRect(rect());
	backgroundRect.setLeft(pixmapRect().right());
	p.drawTiledPixmap(backgroundRect, background_);

	// p.fillRect(toolBar_->geometry(), Qt::red);
}

bool YaChatSeparatorExtra::eventFilter(QObject* obj, QEvent* e)
{
	// if (obj == this && e->type() == QEvent::MouseButtonPress) {
	// 	expand();
	// }

	return QWidget::eventFilter(obj, e);
}

void YaChatSeparatorExtra::overlayClicked()
{
	expand();
}

int YaChatSeparatorExtra::normalWidth() const
{
	int w = pixmap_.width();

	if (addButton_->isVisible())
		w += addButton_->sizeHint().width();
	if (authButton_->isVisible())
		w += authButton_->sizeHint().width();
	if (typographyButton_ && typographyButton_->isVisible())
		w += typographyButton_->sizeHint().width();

	return w;
}

int YaChatSeparatorExtra::expandedWidth() const
{
	return overlayParent_->width() + pixmap_.width();
}

void YaChatSeparatorExtra::animate()
{
	switch (state_) {
	case Collapsing:
	case Expanding:
		if (state_ == Expanding && expandCollapseTimeLine_->currentFrame() >= expandCollapseTimeLine_->endFrame())
			setState(Expanded);

		if (state_ == Collapsing && expandCollapseTimeLine_->currentFrame() <= expandCollapseTimeLine_->endFrame())
			setState(Normal);
		break;
	default:
		;
	}

	// TODO: FIXME: Doesn't fixes initial flicker on re-layout and resize on Windows and Linux.
	QWidget* widget = overlayParent_;
	// QWidget* widget = window();

	bool updatesEnabled = widget->updatesEnabled();
	widget->setUpdatesEnabled(false);

	overlayParent_->parentWidget()->layout()->activate();
	emit invalidateExtraPosition();
	updateChildWidgets();

	widget->setUpdatesEnabled(updatesEnabled);
}

void YaChatSeparatorExtra::setState(State state)
{
	if (state_ == Normal && state == Collapsing)
		state = Normal;

	state_ = state;
	overlayButton_->update();

	switch (state_) {
	case Normal:
		setCursor(Qt::PointingHandCursor);
		break;
	case Collapsing:
		startAnimation(normalWidth());
		break;
	case Expanding:
		startAnimation(expandedWidth());
	case Expanded:
		unsetCursor();
		break;
	}

	animate();
}

void YaChatSeparatorExtra::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	switch (state_) {
	case Normal:
		expandCollapseTimeLine_->setFrameRange(width(), width());
		break;
	case Expanded:
		expandCollapseTimeLine_->setFrameRange(width(), width());
		break;
	default:
		;
	}

	updateChildWidgets();
}

void YaChatSeparatorExtra::updateChildWidgets()
{
	QRect fullRect(rect());
	fullRect.setWidth(expandedWidth());
	fullRect.adjust(0, 2, 0, -4);

	int widthLimit = 0;

	if (addButton_->isVisible()) {
		QRect addButtonRect(rect());
		addButtonRect.setRight(addButtonRect.right() - widthLimit);
		addButtonRect.setLeft(addButtonRect.right() - addButton_->sizeHint().width());
		addButton_->setGeometry(addButtonRect);

		widthLimit += addButtonRect.width();
	}

	if (authButton_->isVisible()) {
		QRect authButtonRect(rect());
		authButtonRect.setRight(authButtonRect.right() - widthLimit);
		authButtonRect.setLeft(authButtonRect.right() - authButton_->sizeHint().width());
		authButton_->setGeometry(authButtonRect);

		widthLimit += authButtonRect.width();
	}

	if (typographyButton_ && typographyButton_->isVisible()) {
		QRect typoButtonRect(rect());
		typoButtonRect.setRight(typoButtonRect.right() - widthLimit);
		typoButtonRect.setLeft(typoButtonRect.right() - typographyButton_->sizeHint().width());
		typographyButton_->setGeometry(typoButtonRect);

		widthLimit += typoButtonRect.width();
	}

	int spacing = 5;
	QRect closeRect(fullRect);
	closeRect.setWidth(16);
	closeRect.moveLeft(fullRect.right() - widthLimit - spacing - closeRect.width());

	QRect toolBarRect(fullRect);
	toolBarRect.setLeft(pixmapRect().right() + spacing);
	toolBarRect.setRight(closeRect.left() - spacing);
	// toolBarRect.setRight(toolBarRect.right() - spacing);
	// toolBarRect.setLeft(closeRect.right() + spacing);
	//
	// int offset = pixmapRect().left() - expandedWidth();
	// closeRect.adjust(offset, 0, offset, 0);
	// toolBarRect.adjust(offset, 0, offset, 0);

	overlayButton_->setGeometry(pixmapRect());
	closeButton_->setGeometry(closeRect);
	toolBar_->setGeometry(toolBarRect);
}

void YaChatSeparatorExtra::startAnimation(int toWidth)
{
	int w = expandCollapseTimeLine_->currentFrame();
	expandCollapseTimeLine_->setFrameRange(w, toWidth);
	if (expandCollapseTimeLine_->state() != QTimeLine::Running)
		expandCollapseTimeLine_->start();
}

void YaChatSeparatorExtra::expand()
{
	if (state_ == Normal) {
		setState(Expanding);
		emit expanding();
	}
}

void YaChatSeparatorExtra::collapse()
{
	if (state_ != Normal) {
		setState(Collapsing);
		emit collapsing();
	}
}

void YaChatSeparatorExtra::updateButtons()
{
	// doing this will result in error in such use-case:
	// enable emoticons in chats, open chat to the person who haven't authorized you
	// "Auth" button is now incorrectly positioned in top-left corner

	// if (addButton_->isVisible() == overlayParent_->showAddButton() &&
	//     authButton_->isVisible() == overlayParent_->showAuthButton())
	// {
	// 	return;
	// }

	// bool updatesEnabled = this->updatesEnabled();
	// setUpdatesEnabled(false);

	addButton_->setVisible(overlayParent_->showAddButton());
	authButton_->setVisible(overlayParent_->showAuthButton());
	addButton_->raise();
	authButton_->raise();
	if (typographyButton_) {
		typographyButton_->raise();
	}

	emit invalidateExtraPosition();
	updateChildWidgets();

	// setUpdatesEnabled(updatesEnabled);
}

//----------------------------------------------------------------------------
// YaChatSeparator
//----------------------------------------------------------------------------

YaChatSeparator::YaChatSeparator(QWidget* parent)
	: OverlayWidget<QWidget, YaChatSeparatorExtra>(parent,
		new YaChatSeparatorExtra(parent->parentWidget(), this))
	, showAddButton_(false)
	, showAuthButton_(false)
	, resizingChatEdit_(false)
{
	connect(extra(), SIGNAL(textSelected(QString)), SIGNAL(textSelected(QString)));
	connect(extra(), SIGNAL(expanding()), SLOT(expanding()));
	connect(extra(), SIGNAL(collapsing()), SLOT(collapsing()));
	connect(extra(), SIGNAL(addContact()), SIGNAL(addContact()));
	connect(extra(), SIGNAL(authContact()), SIGNAL(authContact()));
	connect(extra(), SIGNAL(typographyToggled()), SLOT(typographyToggled()));

	pixmap_ = Ya::VisualUtil::dashBackgroundPixmap();

	parent->installEventFilter(this);

	setMouseTracking(true);

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
	optionChanged(emoticonsPanelEnabledOptionPath);
	optionChanged(enableTypographyOptionPath);
}

YaChatSeparator::~YaChatSeparator()
{
}

QSize YaChatSeparator::minimumSizeHint() const
{
	QSize sh(14, 14);

	// if (extra()->isExpanded())
		sh.setHeight(extra()->sizeHint().height());

	return sh;
}

QSize YaChatSeparator::sizeHint() const
{
	return minimumSizeHint();
}

void YaChatSeparator::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.fillRect(rect(), Qt::white);

	int y = rect().top();
	// if (extra()->isExpanded())
		y += 11;

	p.drawTiledPixmap(QRect(QPoint(rect().left(), y), QSize(rect().right(), pixmap_.height())), pixmap_);
}

bool YaChatSeparator::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == parentWidget() && (e->type() == QEvent::Move || e->type() == QEvent::Resize)) {
		moveExtra();
	}

	return OverlayWidget<QWidget, YaChatSeparatorExtra>::eventFilter(obj, e);
}

QRect YaChatSeparator::extraGeometry() const
{
	QSize sh = extra()->sizeHint();
	int y = -sh.height()/2 + 1;
	// if (extra()->isExpanded())
		y = 0;

	return QRect(globalRect().x() + width() - sh.width(),
	             globalRect().y() + y,
	             sh.width(),
	             sh.height());
}

void YaChatSeparator::expanding()
{
	PsiOptions::instance()->setOption(emoticonsPanelEnabledOptionPath, true);
}

void YaChatSeparator::collapsing()
{
	PsiOptions::instance()->setOption(emoticonsPanelEnabledOptionPath, false);
}

void YaChatSeparator::typographyToggled()
{
	PsiOptions::instance()->setOption(enableTypographyOptionPath, !PsiOptions::instance()->getOption(enableTypographyOptionPath).toBool());
}

void YaChatSeparator::optionChanged(const QString& option)
{
	if (option == emoticonsPanelEnabledOptionPath) {
		if (!isVisible()) {
			extra()->setExpanded(PsiOptions::instance()->getOption(emoticonsPanelEnabledOptionPath).toBool());
		}
	}
	else if (option == editorHeightOptionPath && !chatEdit_.isNull()) {
		const int numLines = this->numLines();
		chatEdit_->setMinimumHeight(qMin(chatEditMaximumHeight(), chatEditHeight(numLines)));
	}
	else if (option == enableTypographyOptionPath) {
		extra()->setTypographyEnabled(PsiOptions::instance()->getOption(enableTypographyOptionPath).toBool());
	}
}

int YaChatSeparator::numLines() const
{
	return qMax(1, qMin(100, PsiOptions::instance()->getOption(editorHeightOptionPath).toInt()));
}

// based on LineEdit::minimumSizeHint() code
int YaChatSeparator::chatEditHeight(const int numLines) const
{
	int height = chatEdit_->fontMetrics().height() * numLines;
	height += 10;
	// height += chatEdit_->lineWidth() * 2;
	return height;
}

int YaChatSeparator::chatEditMaximumHeight() const
{
	if (!window()->isVisible()) {
		return chatEditHeight(numLines());
	}

	Q_ASSERT(!chatEdit_.isNull());
	Q_ASSERT(!chatView_.isNull());
	int height = chatEdit_->parentWidget()->mapToGlobal(chatEdit_->parentWidget()->rect().bottomLeft()).y() - chatView_->mapToGlobal(chatView_->rect().topLeft()).y();
	height -= sizeHint().height();
	height -= 145;
	// qWarning("chatEditMaximumHeight %d %d %d; %d %d %d", height, chatView_->minimumSizeHint().height(), chatView_->minimumHeight(),
	// chatEdit_->mapToGlobal(chatEdit_->rect().bottomLeft()).y(), chatView_->mapToGlobal(chatView_->rect().topLeft()).y(), sizeHint().height());
	return qMax(height, chatEditHeight(1));
}

void YaChatSeparator::setChatWidgets(ChatEdit* chatEdit, QWidget* chatView)
{
	chatEdit_ = chatEdit;
	chatView_ = chatView;
	Q_ASSERT(chatEdit_);
	Q_ASSERT(chatView_);

	updateChatEditHeight();
}

void YaChatSeparator::updateChatEditHeight()
{
	optionChanged(editorHeightOptionPath);
}

void YaChatSeparator::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton && cursor().shape() == Qt::SizeVerCursor) {
		Q_ASSERT(!chatEdit_.isNull());
		resizingChatEdit_ = true;
		mousePressPosition_ = e->globalPos();
		chatEditStartHeight_ = chatEdit_->height();
	}
	e->accept();
}

void YaChatSeparator::mouseReleaseEvent(QMouseEvent* e)
{
	if (resizingChatEdit_) {
		Q_ASSERT(!chatEdit_.isNull());
		int height = chatEdit_->height() - chatEditHeight(0);
		int lineHeight = chatEdit_->fontMetrics().height();
		int numLines = height / lineHeight;
		if (height % lineHeight) {
			numLines += 1;
		}

		// qWarning("%d %d %d", chatEdit_->height(), height, chatEdit_->fontMetrics().height());
		// qWarning("setNumLines: %d", numLines);
		PsiOptions::instance()->setOption(editorHeightOptionPath, numLines);
		updateChatEditHeight();
	}
	resizingChatEdit_ = false;
	mousePressPosition_ = QPoint();
	e->accept();
}

void YaChatSeparator::mouseMoveEvent(QMouseEvent* e)
{
	if (e->buttons() & Qt::LeftButton && resizingChatEdit_) {
		Q_ASSERT(!chatEdit_.isNull());
		int minimumHeight = chatEditHeight(1);
		int height = chatEditStartHeight_ + mousePressPosition_.y() - e->globalPos().y();
		chatEdit_->setMinimumHeight(qMin(chatEditMaximumHeight(), qMax(minimumHeight, height)));
	}
	else {
		if (e->y() >= 10 && e->y() <= 13)
			setCursor(Qt::SizeVerCursor);
		else
			unsetCursor();
	}
	e->accept();
}

bool YaChatSeparator::showAddButton() const
{
	return showAddButton_;
}

void YaChatSeparator::setShowAddButton(bool showAddButton)
{
	showAddButton_ = showAddButton;
	extra()->updateButtons();
}

bool YaChatSeparator::showAuthButton() const
{
	return showAuthButton_;
}

void YaChatSeparator::setShowAuthButton(bool showAuthButton)
{
	showAuthButton_ = showAuthButton;
	extra()->updateButtons();
}
