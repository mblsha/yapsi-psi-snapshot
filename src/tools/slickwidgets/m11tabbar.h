/*
 * m11tabbar.h
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

#ifndef M11TABBAR_H
#define M11TABBAR_H

#include <QEvent>
#include <QMap>
#include <QStyleOptionTabV2>
#include <QTabBar>

#include "utils.h"

namespace Ya {

	class M11TabWidget;

	class M11TabInfo
	{
		public:
			M11TabInfo() : color_(Qt::black) {};
			M11TabInfo(QColor color) : color_(color) {};
			virtual ~M11TabInfo() {};

			QColor color() const { return color_; };
		protected:
			QColor color_;
	};

	class M11TabBar : public ::QTabBar
	{
		Q_OBJECT
		public:
			M11TabBar(M11TabWidget *parent);
			virtual ~M11TabBar();

		public:
			// reimplemented
			virtual QSize sizeHint() const;

		signals:
			void mouseDoubleClickTab(int index);
			void tabDropped(int index, M11TabBar* source);

		protected:
			// reimplemented
			virtual void enterEvent(QEvent *);
			virtual void leaveEvent(QEvent *);
			virtual void mouseDoubleClickEvent( QMouseEvent* event );
			virtual void paintEvent(QPaintEvent *);
			
			// internal helpers
			QStyleOptionTabV2 getStyleOption(int) const;

		public:
			void setTabTextColor(int index, QColor color);

		protected:
			QMap<int, M11TabInfo> m_tabLabelColors;
	};
}

#endif
