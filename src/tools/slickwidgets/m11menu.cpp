/*
 * m11menu.cpp
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

#include <QPaintEvent>
#include <QStyleOptionMenuItem>
#include <QStylePainter>

#include "m11menu.h"
#include "utils.h"

namespace Ya
{
	M11Menu::M11Menu(QWidget * parent)
		: ::QMenu(parent)
		, m_tearoff(0)
		, m_tornoff(0)
		, m_tearoffHighlighted(0)
		, m_hasCheckableItems(false)
		, m_tabWidth(0)
		, m_maxIconWidth(0)
		, scroll(new M11MenuScroller())
	{}

	M11Menu::M11Menu(const QString &title, QWidget *parent)
		: ::QMenu(title, parent)
		, m_tearoff(0)
		, m_tornoff(0)
		, m_tearoffHighlighted(0)
		, m_hasCheckableItems(false)
		, m_tabWidth(0)
		, m_maxIconWidth(0)
		, scroll(new M11MenuScroller())
	{
	}


	M11Menu::~M11Menu()
	{
		delete scroll;
	}

	bool M11Menu::hasCheckableItems() const
	{
		return m_hasCheckableItems;
	}

	int M11Menu::tabWidth() const
	{
		return m_tabWidth;
	}

	int M11Menu::maxIconWidth() const
	{
		return m_maxIconWidth;
	}

	QStyleOptionMenuItem M11Menu::getStyleOption(const QAction *action) const
	{
		QStyleOptionMenuItem opt;
		opt.initFrom(this);
		opt.palette = palette();
		opt.state = QStyle::State_None;
	
		if (window()->isActiveWindow()) {
			opt.state |= QStyle::State_Active;
		}
		if (isEnabled() && action->isEnabled() && (!action->menu() || action->menu()->isEnabled())) {
			opt.state |= QStyle::State_Enabled;
		} else {
			opt.palette.setCurrentColorGroup(QPalette::Disabled);
		}
	
		opt.font = action->font();

/*	
		if (currentAction && currentAction == action && !currentAction->isSeparator()) {
			opt.state |= QStyle::State_Selected | (mouseDown ? QStyle::State_Sunken : QStyle::State_None);
		}
*/
		opt.menuHasCheckableItems = hasCheckableItems();
		if (!action->isCheckable()) {
			opt.checkType = QStyleOptionMenuItem::NotCheckable;
		} else {
			opt.checkType = (action->actionGroup() && action->actionGroup()->isExclusive())
								? QStyleOptionMenuItem::Exclusive : QStyleOptionMenuItem::NonExclusive;
			opt.checked = action->isChecked();
		}
		if (action->menu()) {
			opt.menuItemType = QStyleOptionMenuItem::SubMenu;
		} else if (action->isSeparator()) {
			opt.menuItemType = QStyleOptionMenuItem::Separator;
		} else if (defaultAction() == action) {
			opt.menuItemType = QStyleOptionMenuItem::DefaultItem;
		} else {
			opt.menuItemType = QStyleOptionMenuItem::Normal;
		}
		opt.icon = action->icon();
		QString textAndAccel = action->text();
	#ifndef QT_NO_SHORTCUT
		if (textAndAccel.indexOf(QLatin1Char('\t')) == -1) {
			QKeySequence seq = action->shortcut();
			if (!seq.isEmpty()) {
				textAndAccel += QLatin1Char('\t') + QString(seq);
			}
		}
	#endif
		opt.text = textAndAccel;
		opt.tabWidth = tabWidth();
		opt.maxIconWidth = maxIconWidth();
		opt.menuRect = rect();
		return opt;
	}

	void M11Menu::paintEvent(QPaintEvent *e)
	{
		QStylePainter p(this);
		QRegion emptyArea = QRegion(rect());

		p.fillRect(rect(), Utils::instance().palette().base());

		QPen pen = p.pen();

		//draw the items that need updating..
		for (int i = 0; i < actions().count(); ++i) {
			QAction *action = actions().at(i);
			QRect adjustedActionRect = actionGeometry(action);
			if (!e->rect().intersects(adjustedActionRect) /*|| widgetItems.value(action)*/) {
				continue;
			}
			//set the clip region to be extra safe (and adjust for the scrollers)
			QRegion adjustedActionReg(adjustedActionRect);
			emptyArea -= adjustedActionReg;
			p.setClipRegion(adjustedActionReg);

			QStyleOptionMenuItem opt = getStyleOption(action);
			if (!action->isSeparator()) {
				opt.rect = adjustedActionRect;
				p.fillRect(
					opt.rect.adjusted(1, 1, -1, -1), 
					opt.rect.contains(mapFromGlobal(QCursor::pos())) ?
						Utils::instance().palette().light() :
						Utils::instance().palette().base()
				);
				p.drawItemPixmap(
					opt.rect.adjusted(0, 0, opt.rect.height() - opt.rect.width(), 0),
					Qt::AlignCenter,
					opt.icon.pixmap(QSize(opt.rect.height(), opt.rect.height()))
				);
				int textAlign = opt.rect.contains(mapFromGlobal(QCursor::pos())) ? -1 : 0;
				p.drawItemText(
					opt.rect.adjusted(Utils::instance().textMargin() + opt.rect.height() + textAlign, textAlign, 0, 0),
					Qt::AlignLeft | Qt::AlignVCenter,
					Utils::instance().palette(),
					action->isEnabled(),
					action->text()
				);
			} else { // separator
				p.fillRect(opt.rect, Utils::instance().palette().alternateBase());
				// style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);
			}
		}
	
		const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
		QStyleOptionMenuItem menuOpt;
		menuOpt.initFrom(this);
		menuOpt.palette = palette();
		menuOpt.state = QStyle::State_None;
		menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
		menuOpt.menuRect = rect();
		menuOpt.maxIconWidth = 0;
		menuOpt.tabWidth = 0;
		//draw the scroller regions..
		if (scroll) {
			const int scrollerHeight = style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, this);
			menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;
			menuOpt.state |= QStyle::State_Enabled;
			if (scroll->scrollFlags & M11MenuScroller::ScrollUp) {
				menuOpt.rect.setRect(fw, fw, width() - (fw * 2), scrollerHeight);
				emptyArea -= QRegion(menuOpt.rect);
				p.setClipRect(menuOpt.rect);
				style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
			}
			if (scroll->scrollFlags & M11MenuScroller::ScrollDown) {
				menuOpt.rect.setRect(fw, height() - scrollerHeight - fw, width() - (fw * 2),
										scrollerHeight);
				emptyArea -= QRegion(menuOpt.rect);
				menuOpt.state |= QStyle::State_DownArrow;
				p.setClipRect(menuOpt.rect);
				style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
			}
		}
		//paint the tear off..
		if (m_tearoff) {
			menuOpt.menuItemType = QStyleOptionMenuItem::TearOff;
			menuOpt.rect.setRect(fw, fw, width() - (fw * 2), style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this));
			if (scroll && scroll->scrollFlags & M11MenuScroller::ScrollUp) {
				menuOpt.rect.translate(0, style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, this));
			}
			emptyArea -= QRegion(menuOpt.rect);
			p.setClipRect(menuOpt.rect);
			menuOpt.state = QStyle::State_None;
			if (m_tearoffHighlighted) {
				menuOpt.state |= QStyle::State_Selected;
			}
			style()->drawControl(QStyle::CE_MenuTearoff, &menuOpt, &p, this);
		}
		//draw border
		if (fw) {
			QRegion borderReg;
			borderReg += QRect(0, 0, fw, height()); //left
			borderReg += QRect(width()-fw, 0, fw, height()); //right
			borderReg += QRect(0, 0, width(), fw); //top
			borderReg += QRect(0, height()-fw, width(), fw); //bottom
			p.setClipRegion(borderReg);
			emptyArea -= borderReg;
			p.setPen(Utils::instance().palette().dark().color());
			p.drawRect(rect().adjusted(0, 0, -1, -1));
			p.setPen(pen);
		}
		//finally the rest of the space
		p.setClipRegion(emptyArea);
		menuOpt.state = QStyle::State_None;
		menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
		menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
		menuOpt.rect = rect();
		menuOpt.menuRect = rect();
		style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);
	}

}
