/*
 * syntaxhighlighter.cpp - base class for combined syntax highlighters
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

#include "syntaxhighlighter.h"

#include <QTextEdit>
#include <QKeyEvent>

SyntaxHighlighter::SyntaxHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* _textEdit)
	: QObject(parent)
	, hl_(parent)
	, textEdit_(_textEdit)
{
	if (textEdit())
		textEdit()->installEventFilter(this);
}

SyntaxHighlighter::SyntaxHighlighter(CombinedSyntaxHighlighter* parent, QTextDocument* textDocument)
	: QObject(parent)
	, hl_(parent)
	, textEdit_(0)
{
	Q_UNUSED(textDocument);
}

bool SyntaxHighlighter::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::KeyPress && obj == textEdit()) {
		return keyPressEvent(static_cast<QKeyEvent*>(e));
	}

	return QObject::eventFilter(obj, e);
}

void SyntaxHighlighter::insertTypedText(QKeyEvent* e)
{
	QTextCursor cursor = textEdit()->textCursor();
	cursor.insertText(e->text());
	textEdit()->setTextCursor(cursor);
}

/**
 * Returns -1 if cursor is located not in the text block we're currently highlighting.
 */
int SyntaxHighlighter::cursorPositionInCurrentBlock() const
{
	int cursorPosition = -1;

	QTextCursor cursor = textEdit()->textCursor();
	int currentPosition = cursor.position();

	static int cursorMagic = 0xC0125012;
	int oldState = currentBlockState();
	hl()->setCurrentBlockState(cursorMagic);
	if (cursor.block().userState() == cursorMagic) {
		cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
		int blockStart = cursor.position();

		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		int blockEnd = cursor.position();
		cursor.setPosition(currentPosition);

		cursorPosition = currentPosition - blockStart;
	}
	hl()->setCurrentBlockState(oldState);
	return cursorPosition;
}
