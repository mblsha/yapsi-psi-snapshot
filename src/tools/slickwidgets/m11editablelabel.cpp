/*
 * m11editablelabel.cpp
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

#include <QFocusEvent>
#include <QMouseEvent>

#include "m11editablelabel.h"

#include "m11edit.h"

namespace Ya
{

	M11EditableLabel::M11EditableLabel(bool editable, QWidget *parent)
			: ::QLabel(parent)
			, m_edit(new M11Edit(this))
			, m_editable(!editable)
	{
		m_edit->move(0, 0);
		m_edit->resize(size());
		m_edit->hide();
		setEditable(editable);
	}

	M11EditableLabel::M11EditableLabel(const QString &s, bool editable, QWidget *parent)
			: ::QLabel(s, parent)
			, m_edit(new M11Edit(this))
			, m_editable(!editable)
	{
		m_edit->move(0, 0);
		m_edit->resize(size());
		m_edit->hide();
		setEditable(editable);
	}

	M11EditableLabel::~M11EditableLabel()
	{
		setEditable(false);
		if (m_edit) {
			delete m_edit;
			m_edit = 0;
		}
	}


	QString M11EditableLabel::text() const
	{
		return QLabel::text();
	}


	void M11EditableLabel::setEditable(bool editable) {
		if (editable != m_editable) {
			m_editable = editable;
			if (m_editable) {
				connect (m_edit, SIGNAL(textChanged(const QString &)), this, SLOT(onTextChanged(const QString &)));
				connect (m_edit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
				setCursor(Qt::PointingHandCursor);
			} else {
				disconnect (m_edit, 0, 0, 0);
				setCursor(Qt::ArrowCursor);
			}
		}
	}

	void M11EditableLabel::doEdit(bool on) {
		if (!m_editable) {
			return;
		}
		if (on) {
			m_edit->setText(text());
			m_edit->show();
			m_edit->setFocus(Qt::PopupFocusReason);
		} else {
			setText(m_edit->text());
			m_edit->hide();
		}
	}

	void M11EditableLabel::resizeEvent(QResizeEvent *e) {
		m_edit->resize(size());
		::QLabel::resizeEvent(e);
	}

	void M11EditableLabel::mousePressEvent(QMouseEvent *e)
	{
		doEdit(true);
		::QLabel::mousePressEvent(e);
	}

	void M11EditableLabel::mouseReleaseEvent(QMouseEvent *e)
	{
		::QLabel::mousePressEvent(e);
	}

	void M11EditableLabel::onTextChanged(const QString &/* text*/) {
		// setText(m_edit->text()); // superfluous
	}

	void M11EditableLabel::onEditingFinished() {
		doEdit(false);
		emit labelChanged(m_edit->text());
	}

}
