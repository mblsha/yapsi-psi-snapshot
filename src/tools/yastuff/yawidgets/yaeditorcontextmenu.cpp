/*
 * yaeditorcontextmenu.cpp - custom context menu for text editor widgets
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

#include "yaeditorcontextmenu.h"

#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <private/qtextcontrol_p.h>

YaEditorContextMenu::YaEditorContextMenu(QObject* parent)
	: QObject(parent)
	, widget_(0)
{
	copyAction_ = new QAction(tr("&Copy"), this);
	connect(copyAction_, SIGNAL(triggered()), SLOT(copy()));

	selectAllAction_ = new QAction(tr("Select &All"), this);
	connect(selectAllAction_, SIGNAL(triggered()), SLOT(selectAll()));

	pasteAction_ = new QAction(tr("&Paste"), this);
	connect(pasteAction_, SIGNAL(triggered()), SLOT(paste()));
}

void YaEditorContextMenu::exec(QContextMenuEvent* e, QTextEdit* textEdit)
{
	widget_ = textEdit;
	exec(e);
}

void YaEditorContextMenu::exec(QContextMenuEvent* e, QLineEdit* lineEdit)
{
	widget_ = lineEdit;
	exec(e);
}

void YaEditorContextMenu::exec(QContextMenuEvent* e, QLabel* label)
{
	widget_ = label;
	exec(e);
}

void YaEditorContextMenu::exec(QContextMenuEvent* e)
{
	Q_ASSERT(widget_);

	copyAction_->setEnabled(hasSelection());
	pasteAction_->setEnabled(canPaste());

	QMenu menu;
	menu.addAction(copyAction_);
	if (supportsEditing())
		menu.addAction(pasteAction_);
	else
		menu.addAction(selectAllAction_);
	menu.exec(e->globalPos());
}

QTextEdit* YaEditorContextMenu::textEdit() const
{
	Q_ASSERT(widget_);
	return dynamic_cast<QTextEdit*>(widget_);
}

QLineEdit* YaEditorContextMenu::lineEdit() const
{
	Q_ASSERT(widget_);
	return dynamic_cast<QLineEdit*>(widget_);
}

QTextControl* YaEditorContextMenu::textControl() const
{
	Q_ASSERT(widget_);
	QTextControl* result = widget_->findChild<QTextControl*>();
	// Q_ASSERT(result);
	return result;
}

bool YaEditorContextMenu::supportsEditing() const
{
	if (dynamic_cast<QTextEdit*>(widget_))
		return true;
	if (dynamic_cast<QLineEdit*>(widget_))
		return true;
	return false;
}

bool YaEditorContextMenu::hasSelection() const
{
	if (textEdit())
		return textEdit()->textCursor().hasSelection();
	if (lineEdit())
		return lineEdit()->hasSelectedText();
	if (textControl())
		return textControl()->textCursor().hasSelection();
	return false;
}

bool YaEditorContextMenu::canPaste() const
{
	const QMimeData* mime = QApplication::clipboard()->mimeData();
	return mime && canInsertFromMimeData(mime);
}

bool YaEditorContextMenu::canInsertFromMimeData(const QMimeData* source) const
{
	// Q_D(const QTextControl);
	// if (d->acceptRichText)
	// 	return (source->hasText() && !source->text().isEmpty())
	// 	       || source->hasHtml()
	// 	       || source->hasFormat(QLatin1String("application/x-qrichtext"))
	// 	       || source->hasFormat(QLatin1String("application/x-qt-richtext"));
	// else
		return source->hasText() && !source->text().isEmpty();
}

void YaEditorContextMenu::copy()
{
	if (textEdit())
		textEdit()->copy();
	else if (lineEdit())
		lineEdit()->copy();
	else if (textControl())
		textControl()->copy();
}

void YaEditorContextMenu::selectAll()
{
	if (textEdit())
		textEdit()->selectAll();
	else if (lineEdit())
		lineEdit()->selectAll();
	else if (textControl())
		textControl()->selectAll();
}

void YaEditorContextMenu::paste()
{
	if (textEdit())
		textEdit()->paste();
	else if (lineEdit())
		lineEdit()->paste();
	else if (textControl())
		textControl()->paste();
}
