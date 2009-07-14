/*
 * cpphighlighter.cpp - simplistic highlighter of C++ code
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

#include "cpphighlighter.h"

#include <QKeyEvent>
#include <QTextCursor>
#include <QTextEdit>

static const QString S_MONOFONT = QString::fromUtf8("Lucidatypewriter");

CppHighlighter::CppHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit)
	: SyntaxHighlighter(parent, textEdit)
{
}

CppHighlighter::CppHighlighter(CombinedSyntaxHighlighter* parent, QTextDocument* textDocument)
	: SyntaxHighlighter(parent, textDocument)
{
}

bool CppHighlighter::highlightBlock(const QString &text)
{
	QRegExp cppRegExp = QRegExp("#c.*$");
	int index = text.indexOf(cppRegExp);
	bool formatted = false;
	while (index >= 0) {
		int nextIndex = index + cppRegExp.matchedLength();
		setFormat(index, 2, Qt::lightGray);
		highlightCppCode(text, index, nextIndex);
		index = text.indexOf(cppRegExp, nextIndex);
		formatted = true;
	}

	return formatted;
}

void CppHighlighter::highlightCppCode(const QString & text, int iFrom, int iTo)
{
	HighlightingRule rule;
	QVector<HighlightingRule> highlightingRules;
	QTextCharFormat keywordFormat;
	QTextCharFormat classFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;

	QFont font;
	font.setFamily(S_MONOFONT);
	font.setFixedPitch(true);
	setFormat(iFrom + 3, iTo, font);

	keywordFormat.setFontFamily(S_MONOFONT);
	keywordFormat.setFontFixedPitch(true);
	keywordFormat.setFontWeight(QFont::Bold);
	keywordFormat.setForeground(Qt::darkBlue);
	QStringList keywordPatterns;
	keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
	                << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
	                << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
	                << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
	                << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
	                << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
	                << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
	                << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
	                << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
	                << "\\bvoid\\b" << "\\bvolatile\\b" << "\\breturn\\b";
	foreach(QString pattern, keywordPatterns) {
		rule.pattern = QRegExp(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	functionFormat.setFontFamily(S_MONOFONT);
	functionFormat.setFontFixedPitch(true);
	functionFormat.setFontItalic(true);
	functionFormat.setForeground(Qt::blue);
	rule.pattern = QRegExp("\\b[A-Za-z0-9_]+\\s*(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	classFormat.setFontFamily(S_MONOFONT);
	classFormat.setFontFixedPitch(true);
	classFormat.setFontWeight(QFont::Bold);
	classFormat.setForeground(Qt::darkMagenta);
	rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
	rule.format = classFormat;
	highlightingRules.append(rule);

	singleLineCommentFormat.setFontFamily(S_MONOFONT);
	singleLineCommentFormat.setFontFixedPitch(true);
	singleLineCommentFormat.setForeground(Qt::gray);
	rule.pattern = QRegExp("//[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	quotationFormat.setFontFamily(S_MONOFONT);
	quotationFormat.setFontFixedPitch(true);
	quotationFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("\".*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	foreach(HighlightingRule rule, highlightingRules) {
		QRegExp expression(rule.pattern);
		int index = text.indexOf(expression);
		while (index >= iFrom && (iTo == -1 || index <= iTo)) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = text.indexOf(expression, index + length);
		}
	}

	setCurrentBlockState(0);
	int startIndex = iFrom;
	QRegExp commentStartExpression = QRegExp("/\\*");
	QRegExp commentEndExpression = QRegExp("\\*/");
	QTextCharFormat multiLineCommentFormat;
	multiLineCommentFormat.setFontFamily(S_MONOFONT);
	multiLineCommentFormat.setFontFixedPitch(true);
	multiLineCommentFormat.setForeground(Qt::gray);

	if (previousBlockState() != 1)
		startIndex = text.indexOf(commentStartExpression);

	while (startIndex >= iFrom && (iTo == -1 || startIndex <= iTo)) {
		int endIndex = text.indexOf(commentEndExpression, startIndex);
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else {
			commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
	}
}

bool CppHighlighter::keyPressEvent(QKeyEvent* e)
{
	switch (e->key()) {
	case 16777220:
	case '\r':
	case '\n':  // carriage return
		QTextCursor cursor = textEdit()->textCursor();
		int index = cursor.position();

		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
		bool needCppLeader = QString("#c") == cursor.selectedText().left(2);
		cursor.setPosition(index, QTextCursor::KeepAnchor);

		if (needCppLeader) {
			textEdit()->setUpdatesEnabled(false);
			insertTypedText(e);
			cursor = textEdit()->textCursor();
			cursor.insertText(QString::fromUtf8("#c "));
			textEdit()->setTextCursor(cursor);
			textEdit()->setUpdatesEnabled(true);
			return true;
		}
	}

	return false;
}
