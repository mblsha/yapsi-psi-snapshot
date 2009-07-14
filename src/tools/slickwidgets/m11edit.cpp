/*
 * m11edit.cpp
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

#include <QPainter>

#include "m11edit.h"
#include "utils.h"

namespace Ya
{

	M11Edit::M11Edit(QWidget *parent)
		: QLineEdit(parent)
		, m_margin(0)
		, m_rounded(false)
		, m_mouseState(Utils::Normal)
	{
		setMouseTracking(true);
	}

	M11Edit::M11Edit(const QString &text, QWidget *parent)
		: QLineEdit(text, parent)
		, m_margin(0)
		, m_rounded(false)
		, m_mouseState(Utils::Normal)
	{
		setMouseTracking(true);
	}

	M11Edit::~M11Edit()
	{}

	void M11Edit::paintEvent(QPaintEvent *event)
	{
		QLineEdit::paintEvent(event);
		QPainter p(this);
		QRect r = rect().adjusted(0, 0, -1, -1);
		QPen pen = p.pen();
/*
		p.setPen(
			r.contains(mapFromGlobal(QCursor::pos())) ?
				Utils::instance().palette().light().color() :
				Utils::instance().palette().alternateBase().color()
		);
*/
		switch (m_mouseState) {
			case Utils::Active:
				p.setPen(Utils::instance().palette().light().color());
				break;
			case Utils::Hover:
				p.setPen(Utils::instance().palette().dark().color());
				break;
			case Utils::Normal:
			default:
				p.setPen(Utils::instance().palette().alternateBase().color());
		}
		if (m_rounded) {
			p.drawRoundRect(r);
		} else {
			p.drawRect(r);
		}
		p.setPen(pen);
	}

	void M11Edit::enterEvent(QEvent *e)
	{
		if (m_mouseState != Utils::Active) { // if so, let's wait until focusOutEvent
			m_mouseState = Utils::Hover;
			update();
		}
		::QLineEdit::enterEvent(e);
	}

	void M11Edit::leaveEvent(QEvent *e)
	{
		if (m_mouseState != Utils::Active) { // if so, let's wait until focusOutEvent
			m_mouseState = Utils::Normal;
			update();
		}
		::QLineEdit::leaveEvent(e);
	}

	void M11Edit::focusInEvent(QFocusEvent *e)
	{
		m_mouseState = Utils::Active;
		update();
		::QLineEdit::focusInEvent(e);
	}

	void M11Edit::focusOutEvent(QFocusEvent *e)
	{
		m_mouseState = Utils::Normal;
		update();
		::QLineEdit::focusOutEvent(e);
	}
}
