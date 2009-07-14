/*
 * m11menu.h
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

#ifndef M11MENU_H
#define M11MENU_H

#include <QAction>
#include <QBasicTimer>
#include <QList>
#include <QMenu>
#include <QRect>
#include <QStyleOptionMenuItem>

namespace Ya
{
	class M11Menu : public ::QMenu
	{
		Q_OBJECT
		public:
			M11Menu(QWidget *parent = 0);
			M11Menu(const QString &title, QWidget *parent = 0);
			virtual ~M11Menu();

		protected:
			// reimplemented
			void paintEvent(QPaintEvent *);

			// internal helpers
			QList<QAction*> actionList();
			QRect actionRect(QAction *) const;
			QStyleOptionMenuItem getStyleOption(const QAction *) const;
			bool hasCheckableItems() const;
			int tabWidth() const;
			int maxIconWidth() const;

		protected:
			uint m_tearoff : 1, m_tornoff : 1, m_tearoffHighlighted : 1;
			bool m_hasCheckableItems;
			int m_tabWidth, m_maxIconWidth;
			struct M11MenuScroller {
				enum ScrollLocation { ScrollStay, ScrollBottom, ScrollTop, ScrollCenter };
				enum ScrollDirection { ScrollNone=0x00, ScrollUp=0x01, ScrollDown=0x02 };
				uint scrollFlags : 2, scrollDirection : 2;
				int scrollOffset;
				QBasicTimer *scrollTimer;
				M11MenuScroller() : scrollFlags(ScrollNone), scrollDirection(ScrollNone), scrollOffset(0), scrollTimer(0) { }
				~M11MenuScroller() { delete scrollTimer; }
			} *scroll;
	};
}

#endif
