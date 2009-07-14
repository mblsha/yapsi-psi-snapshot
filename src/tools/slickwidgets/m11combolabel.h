/*
 * m11combolabel.h
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

#ifndef M11COMBOLABEL_H
#define M11COMBOLABEL_H

#include "m11menu.h"

#include "m11editablelabel.h"

namespace Ya
{
	class M11ComboLabel : public M11EditableLabel
	{
		Q_OBJECT
		public:
			M11ComboLabel(QWidget *parent = 0);
			M11ComboLabel(const QString &, QWidget *parent = 0);
			virtual ~M11ComboLabel();

			virtual void setMenu(Ya::M11Menu *menu);
			virtual Ya::M11Menu *menu();

		protected:
			// reimplemented
			virtual void mousePressEvent(QMouseEvent *);

			// internal helpers
			virtual void doMenu(const QPoint &);

		protected:
			Ya::M11Menu *m_menu;
	};
}

#endif
