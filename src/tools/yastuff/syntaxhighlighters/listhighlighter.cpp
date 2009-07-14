/*
 * listhighlighter.cpp - an easier way to insert bullet lists
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

#include "listhighlighter.h"

#include <QTextEdit>
#include <QKeyEvent>

static const QString S_UL_LI = QString::fromUtf8("  •");
static const QRegExp RE_UL_LI = QRegExp(QString::fromUtf8("^\\s*[•\\*]"));

ListHighlighter::ListHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit)
	: SyntaxHighlighter(parent, textEdit)
{}

bool ListHighlighter::highlightBlock(const QString &)
{
	return false;
}

bool ListHighlighter::keyPressEvent(QKeyEvent* e)
{
	switch (e->key()) {
	case 16777220:
	case '\r':
	case '\n':  // carriage return
		QTextCursor cursor = textEdit()->textCursor();
		int index = cursor.position();

		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
		bool needUlLeader = cursor.selectedText().contains(RE_UL_LI);
		cursor.setPosition(index, QTextCursor::KeepAnchor);

		if (needUlLeader) {
			textEdit()->setUpdatesEnabled(false);
			insertTypedText(e);
			cursor = textEdit()->textCursor();

			index = cursor.position();
			cursor.beginEditBlock();
			cursor.movePosition(QTextCursor::Left); // We are before the symbol we just typed
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
			cursor.movePosition(
			    QTextCursor::Right,
			    QTextCursor::MoveAnchor,
			    cursor.selectedText().indexOf(QRegExp(QString::fromUtf8("[\\*•]"))) + 1 // with asterisk itself
			);
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
			cursor.insertText(S_UL_LI);
			cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
			cursor.movePosition(QTextCursor::Right);
			cursor.insertText(S_UL_LI + " ");
			cursor.endEditBlock();
			textEdit()->setTextCursor(cursor);
			textEdit()->setUpdatesEnabled(true);
			return true;
		}
	}

	return false;
}
