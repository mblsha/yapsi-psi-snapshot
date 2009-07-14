/*
 * m11checkbox.cpp
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

#include <QStylePainter>

#include "m11checkbox.h"

static char * combo_on_xpm[] = {
"12 12 2 1",
"   c None",
".  c #000000",
"............",
".          .",
".          .",
".    ..    .",
".    ..    .",
".  ......  .",
".  ......  .",
".    ..    .",
".    ..    .",
".          .",
".          .",
"............"};

static char * combo_off_xpm[] = {
"12 12 2 1",
"   c None",
".  c #000000",
"............",
".          .",
".          .",
".          .",
".          .",
".  ......  .",
".  ......  .",
".          .",
".          .",
".          .",
".          .",
"............"};

static char * combo_onoff_xpm[] = {
"12 12 2 1",
"   c None",
".  c #000000",
"............",
".          .",
".          .",
".    ..    .",
".    ..    .",
".    ..    .",
".    ..    .",
".    ..    .",
".    ..    .",
".          .",
".          .",
"............"};

namespace Ya
{

	M11CheckBox::M11CheckBox()
		: ::QCheckBox()
		, m_margin(0)
		, m_rounded(false)
		, m_mouseState(Utils::Normal)
		, imgComboOff(combo_off_xpm)
		, imgComboOn(combo_on_xpm)
		, imgComboOnOff(combo_onoff_xpm)
	{
		setMouseTracking(true);
	}

	M11CheckBox::M11CheckBox(const QString & text)
		: ::QCheckBox(text)
		, m_margin(0)
		, m_rounded(false)
		, m_mouseState(Utils::Normal)
		, imgComboOff(combo_off_xpm)
		, imgComboOn(combo_on_xpm)
		, imgComboOnOff(combo_onoff_xpm)
	{
		setMouseTracking(true);
	}

	M11CheckBox::~M11CheckBox()
	{}

	void M11CheckBox::drawCombo(QStylePainter *p, const QRect &rect, const Qt::CheckState checkState)
	{
		switch (checkState) {
			case Qt::Unchecked:
				p->drawItemPixmap(rect, Qt::AlignCenter, imgComboOff);
				break;
			case Qt::PartiallyChecked:
				p->drawItemPixmap(rect, Qt::AlignCenter, imgComboOnOff);
				break;
			case Qt::Checked:
				p->drawItemPixmap(rect, Qt::AlignCenter, imgComboOn);
				break;
			default:
				qWarning ("======================= Ya::QCheckBox::drawCombo() =============================================");
				qWarning ("Qt API seems to be changed! Unpredictable behaviour is possible! Please update the library ASAP!");
				break;
		}
	}

	void M11CheckBox::paintEvent(QPaintEvent *)
	{
		QStylePainter p(this);
		QRect r = rect().adjusted(m_margin, m_margin, -m_margin - 1, -m_margin - 1);
		QRect comboRect = r.adjusted(0, 0, r.height() - r.width(), 0);
		QRect textRect = r.adjusted(comboRect.width(), 0, 0, 0);
		drawCombo(&p, comboRect, checkState());
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
				p.setPen(Utils::instance().palette().base().color());
		}
		if (m_rounded) {
			p.drawRoundRect(comboRect);
		} else {
			p.drawRect(comboRect);
		}
		p.setPen(pen);
		p.drawItemText(
			textRect.adjusted(mh, mv, 0, 0),
			Qt::AlignLeft | Qt::AlignVCenter,
			Utils::instance().palette(),
			isEnabled(),
			text()
		);
	}

	void M11CheckBox::enterEvent(QEvent *e)
	{
		m_mouseState = Utils::Hover;
		::QCheckBox::enterEvent(e);
	}

	void M11CheckBox::leaveEvent(QEvent *e)
	{
		m_mouseState = Utils::Normal;
		::QCheckBox::leaveEvent(e);
	}

	void M11CheckBox::mousePressEvent(QMouseEvent *e)
	{
		m_mouseState = Utils::Active;
		::QCheckBox::mousePressEvent(e);
	}

	void M11CheckBox::mouseReleaseEvent(QMouseEvent *e)
	{
		m_mouseState = Utils::Normal;
		::QCheckBox::mouseReleaseEvent(e);
	}

}
