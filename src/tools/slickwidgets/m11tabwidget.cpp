/*
 * m11tabwidget.cpp
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

#include <QPen>
#include <QPainter>

#include "m11tabwidget.h"

#include "m11tabbar.h"
#include "utils.h"

namespace Ya
{
	M11TabWidget::M11TabWidget(QWidget* parent)
		: ::QTabWidget(parent)
		, m_frameShape(QFrame::Box)
	{
		setTabBar(new Ya::M11TabBar(this));
		setTabPosition(South);
	}

	M11TabWidget::~M11TabWidget()
	{
	}

	void M11TabWidget::setFrameShape(QFrame::Shape frameShape)
	{
		if (frameShape != m_frameShape) {
			m_frameShape = frameShape;
			update();
		}
	}

	/** @todo more accuracy here: for HLine, VLine etc. */
	void M11TabWidget::paintEvent(QPaintEvent* event)
	{
		bool needBox = false;
		switch (m_frameShape) {
			case QFrame::HLine:
			case QFrame::VLine:
			case QFrame::Panel:
			case QFrame::WinPanel:
				::QTabWidget::paintEvent(event);
				return;
			case QFrame::NoFrame:
				return;
			case QFrame::StyledPanel:
				break;
			case QFrame::Box:
				needBox = true;
				break;
			default:
				Q_ASSERT(false);
				break;
		}
		QPainter p(this);
		QRect r;
		switch (tabPosition()) {
			case North:
			case South:
				r = rect().adjusted(1, 1 + height() - tabBar()->height(), 0, 0);
				p.fillRect(r, Utils::instance().gradientTopToBottom(r));
				break;
			case West:
			case East:
				r = rect().adjusted(1 + width() - tabBar()->width(), 0, 0, 0);
				p.fillRect(r, Utils::instance().gradientTopToBottom(r));
				break;
			default:
				Q_ASSERT(false);
				break;
		}
 
		p.save();
		// p.setPen(Utils::instance().palette().alternateBase().color());
		// p.drawLine(rect().topRight(), rect().bottomRight());
		p.setPen(Utils::instance().palette().dark().color());
		int pos, w = width() - 1, h = height() - 1;
		switch (tabPosition()) {
			case North:
				pos = tabBar()->height() - 1;
				if (needBox) {
					p.drawLine(QPoint(0, pos), QPoint(0, h));
					p.drawLine(QPoint(0, h), QPoint(w, h));
					p.drawLine(QPoint(w, pos), QPoint(w, h));
					p.drawLine(QPoint(0, pos), QPoint(w, pos));
				} else {
					p.drawLine(QPoint(0, pos), QPoint(w, pos));
				}
				break;
			case South:
				pos = ++h - tabBar()->height();
				if (needBox) {
					// p.drawLine(QPoint(0, 0), QPoint(0, pos));
					p.drawLine(QPoint(0, pos), QPoint(w, pos));
					// p.drawLine(QPoint(w, 0), QPoint(w, pos));
					// p.drawLine(QPoint(0, 0), QPoint(w, 0));
				} else {
					p.drawLine(QPoint(0, pos), QPoint(w, pos));
				}
				break;
			case West:
				pos = tabBar()->width() - 1;
				if (needBox) {
					p.drawLine(QPoint(pos, 0), QPoint(pos, h));
					p.drawLine(QPoint(pos, h), QPoint(w, h));
					p.drawLine(QPoint(w, 0), QPoint(w, h));
					p.drawLine(QPoint(pos, 0), QPoint(w, 0));
				} else {
					p.drawLine(QPoint(pos, 0), QPoint(pos, h));
				}
				break;
			case East:
				pos = ++w - tabBar()->width();
				if (needBox) {
					p.drawLine(QPoint(0, 0), QPoint(0, h));
					p.drawLine(QPoint(0, h), QPoint(pos, h));
					p.drawLine(QPoint(pos, 0), QPoint(pos, h));
					p.drawLine(QPoint(0, 0), QPoint(pos, 0));
				} else {
					p.drawLine(QPoint(pos, 0), QPoint(pos, h));
				}
				break;
			default:
				Q_ASSERT(false);
				break;
		}
		p.restore();
	}

	void M11TabWidget::setTabTextColor(int index, QColor color) {
		static_cast<M11TabBar *>(tabBar())->setTabTextColor(index, color);
	}
}
