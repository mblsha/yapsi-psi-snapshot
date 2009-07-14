/*
 * yastyle.cpp - custom style where some widgets are native
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

#include "yastyle.h"

#include <QApplication>
#include <QToolButton>
#include <QPushButton>
#include <QStyleOptionButton>
#include <QMenu>
#include <QScrollBar>

#ifndef WIDGET_PLUGIN
#include "yacontactlistview.h"
#endif

QStyle* YaStyle::defaultStyle_ = 0;
static YaStyle* instance_;

// extern QHash<QByteArray, QPalette> *qt_app_palettes_hash(); //qapplication.cpp

/**
 * YaStyle could be effectively instantiated only once per application
 */
YaStyle::YaStyle(QStyle* defaultStyle)
	: QPlastiqueStyle()
{
	Q_ASSERT(!instance_);
	instance_ = this;
	Q_ASSERT(!defaultStyle_);
	Q_ASSERT(defaultStyle);
	defaultStyle_ = defaultStyle;

	defaultPalette_ = qApp->palette();
	menuPalette_ = qApp->palette("QMenu");
	menuFont_ = qApp->font("QMenu");

	// it should be easy to copy data based on names provided in qapplication_mac.cpp:
	// void QApplicationPrivate::initializeWidgetPaletteHash() and
	// mac_widget_fonts structure.

	// paletteHash_ = *qt_app_palettes_hash();
	paletteHash_["QMenu"] = menuPalette_;

	// work-around for incorrect separators background color on Mac OS X
	// TODO: check how it looks on Windows XP with classic and luna styles
	QBrush menuBrush = menuPalette_.brush(QPalette::Window);
	// menuBrush.setTexture(QPixmap());
	menuBrush.setStyle(Qt::SolidPattern);
	menuPalette_.setBrush(QPalette::Window, menuBrush);

	defaultStyle_->setParent(this);
}

YaStyle::~YaStyle()
{
}

void YaStyle::polish(QWidget* widget)
{
	if (qobject_cast<QMenu*>(widget) || qobject_cast<QScrollBar*>(widget)) {
		if (qobject_cast<QMenu*>(widget)) {
			widget->setPalette(menuPalette_);
			widget->setFont(menuFont_);
		}
		else {
			widget->setPalette(defaultPalette_);
		}
		defaultStyle()->polish(widget);
		return;
	}

	QPlastiqueStyle::polish(widget);
}

void YaStyle::unpolish(QWidget* widget)
{
	if (qobject_cast<QMenu*>(widget) || qobject_cast<QScrollBar*>(widget)) {
		defaultStyle()->unpolish(widget);
		return;
	}

	QPlastiqueStyle::unpolish(widget);
}

int YaStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
	switch (hint) {
	case SH_DrawMenuBarSeparator:
	case SH_MainWindow_SpaceBelowMenuBar:
	case SH_MenuBar_AltKeyNavigation:
	case SH_MenuBar_DismissOnSecondClick:
	case SH_MenuBar_MouseTracking:
	case SH_Menu_AllowActiveAndDisabled:
	case SH_Menu_FillScreenWithScroll:
	case SH_Menu_KeyboardSearch:
#if QT_VERSION >= 0x040303
	case SH_Menu_Mask:
#endif
	case SH_Menu_MouseTracking:
	case SH_Menu_Scrollable:
	case SH_Menu_SelectionWrap:
	case SH_Menu_SloppySubMenus:
	case SH_Menu_SpaceActivatesItem:
	case SH_Menu_SubMenuPopupDelay:
	case SH_ScrollBar_ContextMenu:
	case SH_ScrollBar_MiddleClickAbsolutePosition:
	case SH_ScrollBar_LeftClickAbsolutePosition:
	case SH_ScrollBar_ScrollWhenPointerLeavesControl:
	case SH_ScrollBar_RollBetweenButtons:
#ifdef QT3_SUPPORT
	case SH_ScrollBar_BackgroundMode:
#endif
	case SH_SpellCheckUnderlineStyle:
		return defaultStyle()->styleHint(hint, option, widget, returnData);
#ifndef WIDGET_PLUGIN
	case SH_ItemView_ActivateItemOnSingleClick: {
		const YaContactListView* yaContactListView = dynamic_cast<const YaContactListView*>(widget);
		if (yaContactListView)
			return yaContactListView->activateItemsOnSingleClick();
	}
#endif
	default:
		break;
	}
	return QPlastiqueStyle::styleHint(hint, option, widget, returnData);
}

QStyle::SubControl YaStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, const QPoint& pos, const QWidget* widget) const
{
	switch (cc) {
	case CC_ScrollBar:
		return defaultStyle()->hitTestComplexControl(cc, opt, pos, widget);
	default:
		break;
	}
	return QPlastiqueStyle::hitTestComplexControl(cc, opt, pos, widget);
}

QRect YaStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex* opt, SubControl sc, const QWidget* widget) const
{
	switch (cc) {
	case CC_ScrollBar:
		return defaultStyle()->subControlRect(cc, opt, sc, widget);
	default:
		break;
	}
	return QPlastiqueStyle::subControlRect(cc, opt, sc, widget);
}

void YaStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* p, const QWidget* widget) const
{
	switch (cc) {
	case QStyle::CC_ComboBox: {
	        if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(opt)){
			QStyleOptionComboBox newopt(*combo);
			// make sure that hover / focus / selection states look no different
			// than normal unfocused one
			newopt.state &= ~QStyle::State_Selected;
			newopt.state &= ~QStyle::State_MouseOver;
			newopt.state &= ~QStyle::State_HasFocus;
			QPlastiqueStyle::drawComplexControl(cc, &newopt, p, widget);
			return;
		}
		break;
	}
	case QStyle::CC_ScrollBar:
		defaultStyle()->drawComplexControl(cc, opt, p, widget);
		return;
	default:
		;
	}

	QPlastiqueStyle::drawComplexControl(cc, opt, p, widget);
}

void YaStyle::processStyleOptionMenuItem(QStyleOptionMenuItem* option, const QWidget* w) const
{
	option->palette = menuPalette();
	if (option->state & QStyle::State_Enabled)
		option->palette.setCurrentColorGroup(QPalette::Active);
	else
		option->palette.setCurrentColorGroup(QPalette::Disabled);

	if (w->property("hide-shortcuts").toBool()) {
		if (option->text.contains(QLatin1Char('\t'))) {
			option->text.replace(QRegExp("\t.+"), "");
		}
	}

	bool bold = option->font.bold();
	option->font = menuFont_;
	option->font.setBold(bold);
	option->fontMetrics = QFontMetrics(menuFont_);

	// if (option->text.length() > 30) {
	// 	option->text = option->text.left(30);
	// }
}

QSize YaStyle::sizeFromContents(ContentsType ct, const QStyleOption* opt, const QSize& contentsSize, const QWidget* w) const
{
	switch (ct) {
	case CT_MenuItem:
	case CT_Menu:
	case CT_MenuBar:
	case CT_MenuBarItem:
		if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
			QStyleOptionMenuItem newopt(*mi);
			processStyleOptionMenuItem(&newopt, w);
			return defaultStyle()->sizeFromContents(ct, &newopt, contentsSize, w);
	        }
	case CT_ScrollBar:
		return defaultStyle()->sizeFromContents(ct, opt, contentsSize, w);
	default:
		break;
	}

	return QPlastiqueStyle::sizeFromContents(ct, opt, contentsSize, w);
}

int YaStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
	switch (metric) {
	case PM_MenuButtonIndicator:
	case PM_MenuBarPanelWidth:
	case PM_MenuBarItemSpacing:
	case PM_MenuBarHMargin:
	case PM_MenuBarVMargin:
	case PM_MenuDesktopFrameWidth:
	case PM_MenuHMargin:
	case PM_MenuPanelWidth:
	case PM_MenuScrollerHeight:
	case PM_MenuTearoffHeight:
	case PM_MenuVMargin:
	case PM_SmallIconSize:
	case PM_ScrollBarExtent:
	case PM_ScrollBarSliderMin:
		return defaultStyle()->pixelMetric(metric, option, widget);
	default:
		break;
	}
	return QPlastiqueStyle::pixelMetric(metric, option, widget);
}

void YaStyle::drawItemText(QPainter* painter, const QRect& rectangle, int alignment, const QPalette& palette, bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
	defaultStyle()->drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void YaStyle::drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const
{
	switch (ce) {
	case CE_MenuItem:
	case CE_MenuBarItem:
	case CE_MenuBarEmptyArea:
	case CE_MenuScroller:
	case CE_MenuTearoff:
	case CE_MenuHMargin:
	case CE_MenuVMargin:
	case CE_MenuEmptyArea:
		if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
			QStyleOptionMenuItem newopt(*mi);
			processStyleOptionMenuItem(&newopt, w);
			defaultStyle()->drawControl(ce, &newopt, p, w);
			return;
	        }
	case CE_ScrollBarAddLine:
	case CE_ScrollBarSubLine:
	case CE_ScrollBarAddPage:
	case CE_ScrollBarSubPage:
	case CE_ScrollBarSlider:
	case CE_ScrollBarFirst:
	case CE_ScrollBarLast:
		defaultStyle()->drawControl(ce, opt, p, w);
		return;
	// case CE_PushButtonLabel: {
	// 	// Right now QStyleSheetStyle calls ParentStyle::drawControl() instead
	// 	// of baseStyle()->drawControl(), so we can't override label-painting
	// 	// code in this class yet. And this is the reason YaPushButton exists.
	//
	// 	const QPushButton* pushButton = dynamic_cast<const QPushButton*>(w);
	// 	Q_ASSERT(pushButton);
	// 	QStyleOptionButton o = *static_cast<const QStyleOptionButton*>(opt);
	// 	int leftMargin = 0;
	// 	int topMargin = 0;
	//
	// 	QIcon icon = o.icon;
	// 	o.icon = QIcon();
	// 	leftMargin = !icon.isNull() ? 10 : 0;
	// 	topMargin  = pushButton->isDown() ? 1 : 0;
	//
	// 	o.rect.adjust(leftMargin, topMargin, 0, topMargin);
	//
	// 	QPlastiqueStyle::drawControl(ce, &o, p, w);
	//
	// 	if (!icon.isNull()) {
	// 		QRect r(QPoint(1, topMargin), icon.actualSize(pushButton->size()));
	// 		icon.paint(p, r, Qt::AlignCenter, pushButton->isEnabled() ? QIcon::Normal : QIcon::Disabled);
	// 	}
	// 	break;
	// }
	default:
		;
	}
	QPlastiqueStyle::drawControl(ce, opt, p, w);
}

void YaStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const
{
	switch (pe) {
	case PE_FrameMenu:
	case PE_PanelMenuBar:
	case PE_IndicatorMenuCheckMark:
		defaultStyle()->drawPrimitive(pe, opt, p, w);
	default:
		break;
	}

	if (pe == PE_FrameFocusRect)
		return;

	QPlastiqueStyle::drawPrimitive(pe, opt, p, w);
}

QRect YaStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
{
	QRect result = QPlastiqueStyle::subElementRect(element, option, widget);
	if (element == QStyle::SE_LineEditContents) {
		int margin = widget ? widget->property("margin-left-right").toInt() : 0;
		result.adjust(margin, 0, -margin, 0);

		int rightMargin = widget ? widget->property("margin-right").toInt() : 0;
		if (rightMargin) {
			result.adjust(0, 0, -rightMargin + margin, 0);
		}
	}
	return result;
}

QStyle* YaStyle::defaultStyle()
{
	return defaultStyle_;
}

QPalette YaStyle::defaultPalette()
{
	return instance_->defaultPalette_;
}

QPalette YaStyle::menuPalette()
{
	return instance_->menuPalette_;
}

QPalette YaStyle::defaultPaletteFor(QWidget* w) const
{
	if (w && paletteHash_.size()) {
		QHash<QByteArray, QPalette>::ConstIterator it = paletteHash_.constFind(w->metaObject()->className());
		if (it != paletteHash_.constEnd())
			return *it;
		for (it = paletteHash_.constBegin(); it != paletteHash_.constEnd(); ++it) {
			if (w->inherits(it.key()))
				return it.value();
		}
	}
	return defaultPalette_;
}

void YaStyle::makeMeNativeLooking(QWidget* widget)
{
	QList<QWidget*> widgets;
	widgets << widget;
	widgets << widget->findChildren<QWidget*>();

	foreach(QWidget* w, widgets) {
		w->setPalette(instance_->defaultPaletteFor(w));
	}
}

QPalette YaStyle::useNativeTextSelectionColors(const QPalette& pal)
{
	QPalette result(pal);
	result.setColor(QPalette::Highlight,       YaStyle::defaultPalette().color(QPalette::Highlight));
	result.setColor(QPalette::HighlightedText, YaStyle::defaultPalette().color(QPalette::HighlightedText));
	return result;
}
