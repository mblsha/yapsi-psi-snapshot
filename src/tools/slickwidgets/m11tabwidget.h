/*
 * m11tabwidget.h
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

#ifndef M11TABWIDGET_H
#define M11TABWIDGET_H

class QMenu;
class QPaintEvent;

#include <QFrame>
#include <QTabWidget>

namespace Ya
{
	class M11TabWidget : public ::QTabWidget
	{
		Q_OBJECT
		public:
			M11TabWidget(QWidget *parent = 0);
			virtual ~M11TabWidget();
		protected:
			// reimplemented
			virtual void paintEvent(QPaintEvent *);

		public:
			void setFrameShape(QFrame::Shape frameShape);
			void setTabTextColor(int index, QColor color);
			QFrame::Shape frameShape() { return m_frameShape; }

		protected:
			QFrame::Shape m_frameShape;

		signals:
			void mouseDoubleClickTab(QWidget *);
			void currentChanged(QWidget *);
			void closeButtonClicked();
			void aboutToShowMenu(QMenu *);
			// context menu on the blank space will have tab==-1
			void tabContextMenu(int, QPoint, QContextMenuEvent *);
	};
}

#endif
