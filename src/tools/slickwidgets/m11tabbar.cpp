/*
 * m11tabbar.cpp
 * Copyright (C) 2008  Yandex LLC (Alexei Matiouchkine)
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

#include <QMouseEvent>
#include <QPalette>
#include <QStyleOptionTab>
#include <QStylePainter>

#include "m11tabbar.h"
#include "utils.h"

static const int SUPERFLUOUS_MYSTERY = 10;

namespace Ya {

	M11TabBar::M11TabBar(M11TabWidget *parent)
		: ::QTabBar((QWidget *)parent)
	{
		setMouseTracking(true);
	}
	
	
	M11TabBar::~M11TabBar()
	{
	}

	QSize M11TabBar::sizeHint() const
	{
		return QSize(QTabBar::sizeHint().width(), QTabBar::sizeHint().height() - SUPERFLUOUS_MYSTERY);
	}

	QStyleOptionTabV2 M11TabBar::getStyleOption(int tab) const
	{
		QStyleOptionTabV2 opt;
		opt.init(this);
		opt.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
		opt.rect = tabRect(tab).adjusted(0, 0, 0, - SUPERFLUOUS_MYSTERY);
		bool isCurrent = tab == currentIndex();
		opt.row = 0;
	//    if (tab == pressedIndex)
	//        opt.state |= QStyle::State_Sunken;
		if (isCurrent)
			opt.state |= QStyle::State_Selected;
		if (isCurrent && hasFocus())
			opt.state |= QStyle::State_HasFocus;
		if (isTabEnabled(tab))
			opt.state &= ~QStyle::State_Enabled;
		if (isActiveWindow())
			opt.state |= QStyle::State_Active;
	//    if (opt.rect == hoverRect)
	//        opt.state |= QStyle::State_MouseOver;
		opt.shape = shape();
		opt.text = tabText(tab);
	
		if (tabTextColor(tab).isValid())
			opt.palette.setColor(foregroundRole(), tabTextColor(tab));
	
		opt.icon = tabIcon(tab);
		opt.iconSize = opt.icon.actualSize(QSize(32, 32));  // Will get the default value then.
	
		int totalTabs = count();
	
		if (tab > 0 && tab - 1 == currentIndex())
			opt.selectedPosition = QStyleOptionTab::PreviousIsSelected;
		else if (tab < totalTabs - 1 && tab + 1 == currentIndex())
			opt.selectedPosition = QStyleOptionTab::NextIsSelected;
		else
			opt.selectedPosition = QStyleOptionTab::NotAdjacent;
	
		if (tab == 0) {
			if (totalTabs > 1)
				opt.position = QStyleOptionTab::Beginning;
			else
				opt.position = QStyleOptionTab::OnlyOneTab;
		} else if (tab == totalTabs - 1) {
			opt.position = QStyleOptionTab::End;
		} else {
			opt.position = QStyleOptionTab::Middle;
		}
		if (const QTabWidget *tw = qobject_cast<const QTabWidget *>(parentWidget())) {
			if (tw->cornerWidget(Qt::TopLeftCorner) || tw->cornerWidget(Qt::BottomLeftCorner))
				opt.cornerWidgets |= QStyleOptionTab::LeftCornerWidget;
			if (tw->cornerWidget(Qt::TopRightCorner) || tw->cornerWidget(Qt::BottomRightCorner))
				opt.cornerWidgets |= QStyleOptionTab::RightCornerWidget;
		}
		return opt;
	}
	
	void M11TabBar::paintEvent(QPaintEvent*)
	{
		QStyleOptionTab tabOverlap;
		tabOverlap.shape = shape();
		int overlap = style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, this);
		QWidget *theParent = parentWidget();
		QStyleOptionTabBarBase optTabBase;
		optTabBase.init(this);
		optTabBase.shape = shape();
		if (theParent && overlap > 0) {
			QRect rect;
			switch (tabOverlap.shape) {
			case M11TabBar::RoundedNorth:
			case M11TabBar::TriangularNorth:
				rect.setRect(0, height() - overlap, width(), overlap);
				break;
			case M11TabBar::RoundedSouth:
			case M11TabBar::TriangularSouth:
				rect.setRect(0, 0, width(), overlap);
				break;
			case M11TabBar::RoundedEast:
			case M11TabBar::TriangularEast:
				rect.setRect(0, 0, overlap, height());
				break;
			case M11TabBar::RoundedWest:
			case M11TabBar::TriangularWest:
				rect.setRect(width() - overlap, 0, overlap, height());
				break;
			}
			optTabBase.rect = rect;
		}

		QStylePainter p(this);
		p.fillRect(rect(), Utils::instance().gradientTopToBottom(rect()));
		QPen pen = p.pen();

		int selected = -1;
		int cut = -1;
		bool rtl = optTabBase.direction == Qt::RightToLeft;
		bool verticalTabs = (shape() == M11TabBar::RoundedWest || shape() == M11TabBar::RoundedEast
		                     || shape() == M11TabBar::TriangularWest
		                     || shape() == M11TabBar::TriangularEast);
		QStyleOptionTab cutTab;
		QStyleOptionTab selectedTab;
		for (int i = 0; i < count(); ++i) {
			QStyleOptionTabV2 tab = getStyleOption(i);
			if (!(tab.state & QStyle::State_Enabled)) {
				tab.palette.setCurrentColorGroup(QPalette::Disabled);
			}
			// If this tab is partially obscured, make a note of it so that we can pass the information
			// along when we draw the tear.
			if ((!verticalTabs && (!rtl && tab.rect.left() < 0) ||
			     (rtl && tab.rect.right() > width())) ||
			    (verticalTabs && tab.rect.top() < 0)) {
				cut = i;
				cutTab = tab;
			}
			// Don't bother drawing a tab if the entire tab is outside of the visible tab bar.
			if ((!verticalTabs && (tab.rect.right() < 0 || tab.rect.left() > width()))
			    || (verticalTabs && (tab.rect.bottom() < 0 || tab.rect.top() > height())))
				continue;

			optTabBase.tabBarRect |= tab.rect;
			if (i == currentIndex()) {
				selected = i;
				selectedTab = tab;
				optTabBase.selectedTabRect = tab.rect;
			}

			QIcon icon = tabIcon(i);
			QSize iconSize = icon.actualSize(QSize(tab.rect.height(), tab.rect.height()));
			int iconMargin = (tab.rect.height() - iconSize.height()) / 2;
			QRect iconRect(tab.rect.left() + iconMargin, tab.rect.top(), iconSize.width(), tab.rect.height());
			QString text = tabText(i);
			QRect textRect(
			    tab.rect.left() + iconMargin + iconSize.width(),
			    tab.rect.top(),
			    tab.rect.width() - iconSize.width(),
			    tab.rect.height()
			);

			p.fillRect(
			    tab.rect,
			    i == currentIndex() ?
			    Utils::instance().palette().base() :
			    tab.rect.contains(mapFromGlobal(QCursor::pos())) ?
			    Utils::instance().palette().light() : Utils::instance().gradientTopToBottom(tab.rect)
			);

			p.setPen(Utils::instance().palette().dark().color());

			switch (shape()) { // override the widget's border to make it consistent over inactive tabs
			case M11TabBar::RoundedNorth:
			case M11TabBar::TriangularNorth:
				if (i == currentIndex()) {
					p.drawRect(tab.rect.adjusted(0, 0, -1, 0));
				}
				else {
					p.drawLine(tab.rect.bottomLeft(), tab.rect.bottomRight());
				}
				break;
			case M11TabBar::RoundedSouth:
			case M11TabBar::TriangularSouth:
				if (i == currentIndex()) {
					p.drawRect(tab.rect.adjusted(0, -1, -1, -1));
				}
				else {
					p.drawLine(tab.rect.topLeft(), tab.rect.topRight());
				}
				break;
			case M11TabBar::RoundedEast:
			case M11TabBar::TriangularEast:
				if (i == currentIndex()) {
					p.drawRect(tab.rect.adjusted(-1, 0, -1, -1));
				}
				else {
					p.drawLine(tab.rect.topLeft(), tab.rect.bottomLeft());
				}
				break;
			case M11TabBar::RoundedWest:
			case M11TabBar::TriangularWest:
				if (i == currentIndex()) {
					p.drawRect(tab.rect.adjusted(0, 0, 0, -1));
				}
				else {
					p.drawLine(tab.rect.topRight(), tab.rect.bottomRight());
				}
				break;
			default:
				Q_ASSERT(false);
				break;
			}

			p.setPen(m_tabLabelColors.contains(i) ? m_tabLabelColors.value(i).color() : pen);

			QString textToDraw = fontMetrics().elidedText(
			                         text,
			                         elideMode(),
			                         1 + (
			                             verticalTabs ? tabRect(i).height() : tabRect(i).width()
			                         ) - style()->pixelMetric(QStyle::PM_TabBarTabHSpace, &tab, this),
			                         Qt::TextShowMnemonic
			                     );

			p.drawItemPixmap(iconRect, Qt::AlignCenter, icon.pixmap(iconSize));

			p.drawItemText(
			    textRect,
			    Qt::AlignLeft | Qt::AlignVCenter,
			    QPalette(),
			    tab.state & QStyle::State_Enabled,
			    textToDraw
			);

			p.setPen(pen);
		}

		if (!drawBase()) {
			p.setBackgroundMode(Qt::TransparentMode);
		}
		else {
			// p.drawPrimitive(QStyle::PE_FrameTabBarBase, optTabBase);
		}

		// Only draw the tear indicator if necessary. Most of the time we don't need too.
		if (cut >= 0) {
			cutTab.rect = rect();
			cutTab.rect = style()->subElementRect(QStyle::SE_TabBarTearIndicator, &cutTab, this);
			p.drawPrimitive(QStyle::PE_IndicatorTabTear, cutTab);
		}

		// p.setPen(Qt::black);
		// p.drawRect(rect());
	}

	void M11TabBar::enterEvent(QEvent *e)
	{
		::QTabBar::enterEvent(e);
	}

	void M11TabBar::leaveEvent(QEvent *e)
	{
		::QTabBar::leaveEvent(e);
	}

	void M11TabBar::setTabTextColor(int index, QColor color) {
		Q_ASSERT(index > 0 || index < count());
		m_tabLabelColors.insert(index, M11TabInfo(color));
	}

	/**
	* Overriding this allows us to emit signals for double clicks
	*/
	void M11TabBar::mouseDoubleClickEvent(QMouseEvent* event)
	{
		for (int i = 0; i < count(); i++) {
			if (tabRect(i).contains(event->pos())) {
				emit mouseDoubleClickTab(i);
			}
		}
	}
	
}
