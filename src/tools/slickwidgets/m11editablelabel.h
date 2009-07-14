/*
 * m11editablelabel.h
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

#ifndef M11EDITABLELABEL_H
#define M11EDITABLELABEL_H

#include <QLabel>

class QMouseEvent;

namespace Ya
{
	class M11Edit;

	class M11EditableLabel : public ::QLabel
	{
		Q_OBJECT
		public:
			M11EditableLabel(bool editable = true, QWidget *parent = 0);
			M11EditableLabel(const QString &, bool editable = true, QWidget *parent = 0);
			virtual ~M11EditableLabel();

		public:
			// getters and setters
			void setEditable(bool editable);
			bool editable() { return m_editable; };

			// reimplemented to make it virtual
			virtual QString text() const;

		protected:
			// reimplemented
			virtual void mousePressEvent(QMouseEvent *);
			virtual void mouseReleaseEvent(QMouseEvent *);
			virtual void resizeEvent(QResizeEvent *);

			// internal helpers
			virtual void doEdit(bool on = true);

		protected:
			M11Edit *m_edit;
			bool m_editable;

		protected slots:
			// listeners on inplace editor
			void onTextChanged(const QString &);
			void onEditingFinished();

		signals:
			void labelChanged(const QString &);
	};
}

#endif
