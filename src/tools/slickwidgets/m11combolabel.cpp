/*
 * m11combolabel.cpp
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

#include "m11combolabel.h"

namespace Ya
{

	M11ComboLabel::M11ComboLabel(QWidget *parent)
		: M11EditableLabel(false, parent)
		, m_menu(0)
	{
	}

	M11ComboLabel::M11ComboLabel(const QString &s, QWidget *parent)
		: M11EditableLabel(s, false, parent)
		, m_menu(0)
	{
	}


	M11ComboLabel::~M11ComboLabel()
	{
	}

	void M11ComboLabel::setMenu(Ya::M11Menu *menu)
	{
		if (menu != m_menu) {
			m_menu = menu;
		}
		setCursor(m_menu ? Qt::PointingHandCursor : Qt::ArrowCursor);
	}

	Ya::M11Menu *M11ComboLabel::menu()
	{
		return m_menu;
	}

	void M11ComboLabel::doMenu(const QPoint &pos)
	{
		if (!m_menu) {
			return;
		}
		m_menu->popup(mapToGlobal(pos));
	}

	void M11ComboLabel::mousePressEvent(QMouseEvent *e)
	{
		doMenu(e->pos());
		::QLabel::mousePressEvent(e);
	}


}
