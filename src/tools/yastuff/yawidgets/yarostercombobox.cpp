/*
 * yarostercombobox.cpp
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
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

#include "yarostercombobox.h"

#include <QLayout>
#include <QTimer>

/**
 * \class YaRosterComboBox
 * \brief ComboBox which automatically resizes to fit the item currently
 * displayed.
 */
YaRosterComboBox::YaRosterComboBox(QWidget* parent)
	: QComboBox(parent)
{
	connect(this, SIGNAL(editTextChanged(const QString&)), SLOT(updateSize()));
	connect(this, SIGNAL(currentIndexChanged(int)), SLOT(updateSize()));
}

YaRosterComboBox::~YaRosterComboBox()
{
}

void YaRosterComboBox::updateSize()
{
	adjustSize();
}

/**
 * If horizontal size policy is not QSizePolicy::Maximum, returns the
 * same value as sizePolicy(), and allows widget to be shrunk below
 * sizePolicy() otherwise.
 */
QSize YaRosterComboBox::minimumSizeHint() const
{
	QSize sh = sizeHint();

	if (sizePolicy().horizontalPolicy() == QSizePolicy::Maximum) {
		sh.setWidth(16);
	}

	return sh;
}

/**
 * Returns size to fit currently displaying item, and only it.
 */
QSize YaRosterComboBox::optimumSizeHint() const
{
	QSize sh;
	sh.setWidth(fontMetrics().width(currentText()));
	sh.setHeight(qMax(fontMetrics().lineSpacing(), 14) + 2);

	QStyleOptionComboBox opt;
	opt.init(this);
	if (!isEditable() && hasFocus())
		opt.state |= QStyle::State_Selected;
	opt.subControls = QStyle::SC_All;
	opt.editable = isEditable();
	opt.currentText = currentText();

	sh = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, sh, this);
	return sh;
}

QSize YaRosterComboBox::sizeHint() const
{
	// TODO: think how to display popup in full width
	return QComboBox::sizeHint();
	return optimumSizeHint();
}
