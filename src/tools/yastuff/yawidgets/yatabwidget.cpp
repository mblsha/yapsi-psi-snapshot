/*
 * yatabwidget.cpp
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

#include "yatabwidget.h"

#include <QSignalMapper>
#include <QPainter>
#include <QEvent>
#include <QPlastiqueStyle>
#include <QApplication>
#include <QVariant>
#include <QAction>
#include <QKeyEvent>

#ifndef WIDGET_PLUGIN
#include "tabbablewidget.h"
#include "yavisualutil.h"
#endif

#ifdef USE_YAMULTILINETABBAR
#include "yamultilinetabbar.h"
#else
#include "yatabbar.h"
#endif

#ifdef YAPSI_ACTIVEX_SERVER
#if QT_VERSION >= 0x040500
#define CUSTOM_SHADOW
#endif
#endif

//----------------------------------------------------------------------------
// YaTabWidgetStyle
//----------------------------------------------------------------------------

class YaTabWidgetStyle : public QPlastiqueStyle
{
public:
	YaTabWidgetStyle(QStyle* defaultStyle)
		: defaultStyle_(defaultStyle)
	{}

	// reimplemented
	void polish(QWidget* widget)
	{
		defaultStyle()->polish(widget);
	}

	void unpolish(QWidget* widget)
	{
		defaultStyle()->unpolish(widget);
	}

	void drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		defaultStyle()->drawControl(ce, opt, p, w);
	}

	void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		defaultStyle()->drawPrimitive(pe, opt, p, w);
	}

	void drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* p, const QWidget* widget) const
	{
		defaultStyle()->drawComplexControl(cc, opt, p, widget);
	}

	QSize sizeFromContents(ContentsType ct, const QStyleOption* opt, const QSize& contentsSize, const QWidget* w) const
	{
		return defaultStyle()->sizeFromContents(ct, opt, contentsSize, w);
	}

	SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, const QPoint& pos, const QWidget* widget) const
	{
		return defaultStyle()->hitTestComplexControl(cc, opt, pos, widget);
	}

	QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt, SubControl sc, const QWidget* widget) const
	{
		return defaultStyle()->subControlRect(cc, opt, sc, widget);
	}

	int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
	{
		switch (hint) {
		case SH_TabBar_Alignment:
			return Qt::AlignLeft;
		case SH_TabBar_PreferNoArrows:
			return true;
#ifdef USE_YAMULTILINETABBAR
		case SH_TabBar_SelectMouseType:
			return QEvent::MouseButtonRelease;
#endif
		default:
			break;
		}
		return defaultStyle()->styleHint(hint, option, widget, returnData);
	}

	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
	{
		switch (metric) {
		case PM_TabBarScrollButtonWidth:
			return 0;
		default:
			break;
		}
		return QPlastiqueStyle::pixelMetric(metric, option, widget);
		// this one gets us into forever recursion on Qt 4.3.3.
		// return defaultStyle()->pixelMetric(metric, option, widget);
	}

	QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
	{
		QRect result = defaultStyle()->subElementRect(element, option, widget);
		switch (element) {
		case SE_TabWidgetTabBar: {
			const YaTabWidget* tw = dynamic_cast<const YaTabWidget*>(widget);
			if (tw) {
				QSize sh = tw->tabSizeHint();
				QRect r  = tw->tabRect();
				result.setLeft(r.left());
				result.setWidth(qMin(r.right(), sh.width()));
				result.setHeight(sh.height() - (widget->height() - r.height()));
#ifdef CUSTOM_SHADOW
				result.moveTop(result.top() - Ya::VisualUtil::windowShadowSize());
#endif
			}
			return result;
		}
		case SE_TabWidgetTabContents:
#ifdef CUSTOM_SHADOW
			return result.adjusted(Ya::VisualUtil::windowShadowSize() - 4,
			                       Ya::VisualUtil::windowShadowSize() + 2,
			                       -Ya::VisualUtil::windowShadowSize() + 4,
			                       -Ya::VisualUtil::windowShadowSize());
#else
			return result.adjusted(-4, 2, 4, 0);
#endif
		default:
			return result;
		}
	}

private:
	QStyle* defaultStyle_;
	QStyle* defaultStyle() const { return defaultStyle_; }
};

//----------------------------------------------------------------------------
// YaTabWidget
//----------------------------------------------------------------------------

YaTabWidget::YaTabWidget(QWidget* parent)
	: QTabWidget(parent)
{
	YaTabWidgetStyle* style = new YaTabWidgetStyle(qApp->style());
	style->setParent(this);
	setStyle(style);

	YaTabBarBaseClass* tabBar = new YaTabBarBaseClass(this);
	connect(tabBar, SIGNAL(closeTab(int)), SIGNAL(closeTab(int)));
	connect(tabBar, SIGNAL(aboutToShow(int)), SLOT(aboutToShow(int)));
	connect(tabBar, SIGNAL(reorderTabs(int, int)), SIGNAL(reorderTabs(int, int)));

	setTabBar(tabBar);
	setTabPosition(South);

	QSignalMapper* activateTabMapper_ = new QSignalMapper(this);
	connect(activateTabMapper_, SIGNAL(mapped(int)), tabBar, SLOT(setCurrentIndex(int)));
	for (int i = 0; i < 10; ++i) {
		QAction* action = new QAction(this);
		connect(action, SIGNAL(triggered()), activateTabMapper_, SLOT(map()));
		action->setShortcuts(QList<QKeySequence>() << QKeySequence(QString("Ctrl+%1").arg(i))
		                                           << QKeySequence(QString("Alt+%1").arg(i)));
		activateTabMapper_->setMapping(action, (i > 0 ? i : 10) - 1);
		addAction(action);
	}
}

YaTabWidget::~YaTabWidget()
{
}

YaTabBarBaseClass* YaTabWidget::yaTabBar() const
{
	return static_cast<YaTabBarBaseClass*>(tabBar());
}

void YaTabWidget::setDrawTabNumbersHelper(QKeyEvent* event)
{
	// ONLINE-2737
	// yaTabBar()->setDrawTabNumbers(event->modifiers().testFlag(Qt::ControlModifier));
}

void YaTabWidget::keyPressEvent(QKeyEvent* event)
{
	QTabWidget::keyPressEvent(event);
	setDrawTabNumbersHelper(event);
}

void YaTabWidget::keyReleaseEvent(QKeyEvent* event)
{
	QTabWidget::keyReleaseEvent(event);
	setDrawTabNumbersHelper(event);
}

const YaWindowTheme& YaTabWidget::theme() const
{
	TabbableWidget* dlg = dynamic_cast<TabbableWidget*>(currentWidget());
	if (dlg) {
		return dlg->theme();
	}

	static YaWindowTheme dummy;
	return dummy;
}

void YaTabWidget::paintEvent(QPaintEvent*)
{
	return;
	QPainter p(this);

#ifndef WIDGET_PLUGIN
	QRect rect = this->rect().adjusted(0, 0, 0, -50);
	QRect contentsRect = rect.adjusted(Ya::VisualUtil::windowShadowSize(), Ya::VisualUtil::windowShadowSize(), -Ya::VisualUtil::windowShadowSize(), 0);
	Ya::VisualUtil::drawWindowTheme(&p, theme(), rect, contentsRect, isActiveWindow());
	Ya::VisualUtil::drawAACorners(&p, theme(), rect, contentsRect);
#else
	p.fillRect(rect(), Qt::red);
#endif

#if 0
	if (tabBar()->isVisible()) {
		QRect r = tabRect();
		int y = tabBar()->geometry().top();
		p.fillRect(QRect(QPoint(r.left(), y), r.bottomRight()), static_cast<YaTabBarBaseClass*>(tabBar())->tabBackgroundColor());

#ifndef WIDGET_PLUGIN
		p.setPen(Ya::VisualUtil::rosterTabBorderColor());
#endif
		p.drawLine(tabBar()->geometry().topLeft(), QPoint(r.right(), y));
	}
#endif
}

QSize YaTabWidget::tabSizeHint() const
{
	return tabBar()->sizeHint();
}

QRect YaTabWidget::tabRect() const
{
#ifdef CUSTOM_SHADOW
	return rect().adjusted(Ya::VisualUtil::windowShadowSize(),
	                       0,
	                       -Ya::VisualUtil::windowShadowSize(),
	                       0);
#else
	return rect().adjusted(0, 0, 1, 0);
#endif
}

void YaTabWidget::resizeEvent(QResizeEvent* e)
{
	QTabWidget::resizeEvent(e);
	static_cast<YaTabBarBaseClass*>(tabBar())->updateLayout();
}

void YaTabWidget::tabInserted(int)
{
	updateLayout();
}

void YaTabWidget::tabRemoved(int)
{
	updateLayout();
}

void YaTabWidget::updateLayout()
{
	// force re-layout
	QEvent e(QEvent::LayoutRequest);
	event(&e);
}

bool YaTabWidget::tabHighlighted(int index) const
{
	return tabBar()->tabData(index).toBool();
}

void YaTabWidget::setTabHighlighted(int index, bool highlighted)
{
	YaTabBarBaseClass* tb = static_cast<YaTabBarBaseClass*>(tabBar());
	bool previouslyHighlighted = tb->tabData(index).toBool();
	tb->setTabData(index, QVariant(highlighted));
	if (previouslyHighlighted != highlighted)
		tb->updateFading();
}

void YaTabWidget::updateHiddenTabs()
{
	static_cast<YaTabBarBaseClass*>(tabBar())->updateHiddenTabs();
}

void YaTabWidget::aboutToShow(int index)
{
#ifndef WIDGET_PLUGIN
	TabbableWidget* tab = dynamic_cast<TabbableWidget*>(widget(index));
	if (tab)
		tab->aboutToShow();
#endif
}

void YaTabWidget::setLayoutUpdatesEnabled(bool enabled)
{
	static_cast<YaTabBarBaseClass*>(tabBar())->setLayoutUpdatesEnabled(enabled);
}

void YaTabWidget::doBlockSignals(bool blockSignals)
{
	this->blockSignals(blockSignals);
	this->tabBar()->blockSignals(blockSignals);
}
