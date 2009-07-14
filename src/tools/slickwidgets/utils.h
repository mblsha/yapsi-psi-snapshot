/*
 * utils.h
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

#ifndef YAUTILS_H
#define YAUTILS_H

#include <QColor>
#include <QCoreApplication>
#include <QPalette>
#include <QRect>

namespace Ya 
{
	class Utils : public QObject
	{
		Q_OBJECT
	public:
		enum MouseState { Normal, Hover, Active/*, Visited*/ };

		static Utils& instance();

		QPalette palette() const;
		int textMargin() const;
		QColor baseColor() const;
		QColor lightColor() const;
		QBrush gradientTopToBottom(const QRect& rect) const;

	private:
		static Utils *utils;
		QPalette palette_;

		Utils(QObject *parent);
		virtual ~Utils() {};

	};
}

#endif
