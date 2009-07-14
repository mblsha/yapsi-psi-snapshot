/*
 * m11button.cpp
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
#include <QStylePainter>

#include "m11button.h"
#include "utils.h"

namespace Ya
{
	M11Button::M11Button()
		: ::QPushButton()
		, m_margin(0)
		, m_rounded(false)
		, m_mouseState(Utils::Normal)
	{
		setMouseTracking(true);
	}


	M11Button::~M11Button()
	{
	}

	void M11Button::paintEvent(QPaintEvent *)
	{
		QStylePainter p(this);
		QRect r = rect().adjusted(m_margin, m_margin, -m_margin - 1, -m_margin - 1);
		p.fillRect(
			r,
			r.contains(mapFromGlobal(QCursor::pos())) ?
				Utils::instance().palette().light() :
				Utils::instance().palette().base()
		);
		QPen pen = p.pen();
		int mh = Utils::instance().textMargin();
		int mv = 0;
		switch (m_mouseState) {
			case Utils::Active:
				p.setPen(Utils::instance().palette().light().color());
				p.fillRect(r, Utils::instance().palette().base());
				mh += mv = 2;
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
		QRect iconRect = r.adjusted(mv + (height() - iconSize().height()) / 2, mv, iconSize().width() - width() + 2 * (mv + 1), 0);
		p.drawItemPixmap(iconRect, Qt::AlignCenter, icon().pixmap(iconSize()));
		QRect textRect = r.adjusted(mh + (height() - iconSize().height()) / 2 + iconRect.width(), mv, 0, 0);
		p.drawItemText(
			textRect,
			Qt::AlignLeft | Qt::AlignVCenter,
			Utils::instance().palette(),
			isEnabled(),
			text()
		);
	}

	void M11Button::enterEvent(QEvent *e)
	{
		m_mouseState = Utils::Hover;
		::QPushButton::enterEvent(e);
	}

	void M11Button::leaveEvent(QEvent *e)
	{
		m_mouseState = Utils::Normal;
		::QPushButton::leaveEvent(e);
	}

	void M11Button::mousePressEvent(QMouseEvent *e)
	{
		m_mouseState = Utils::Active;
		::QPushButton::mousePressEvent(e);
	}

	void M11Button::mouseReleaseEvent(QMouseEvent *e)
	{
		m_mouseState = Utils::Normal;
		::QPushButton::mouseReleaseEvent(e);
	}

}
