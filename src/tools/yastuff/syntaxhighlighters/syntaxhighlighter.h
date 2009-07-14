/*
 * syntaxhighlighter.h - base class for combined syntax highlighters
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

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QObject>

class QTextCharFormat;
class QColor;
class QFont;
class QTextEdit;
class QKeyEvent;

#include "combinedsyntaxhighlighter.h"

class SyntaxHighlighter : public QObject
{
	Q_OBJECT
public:
	SyntaxHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit = 0);
	SyntaxHighlighter(CombinedSyntaxHighlighter* parent, QTextDocument* textDocument = 0);

	virtual bool highlightBlock(const QString &text) = 0;

private:
	CombinedSyntaxHighlighter* hl() const { return hl_; }
	bool eventFilter(QObject*, QEvent*);

protected:
	QTextEdit* textEdit() const { return textEdit_; }
	virtual bool keyPressEvent(QKeyEvent*) { return false; }

	void insertTypedText(QKeyEvent*);

protected:
	int cursorPositionInCurrentBlock() const;
	int currentBlockState() const { return hl()->currentBlockState(); }
	void setCurrentBlockState(int newState) { hl()->setCurrentBlockState(newState); }
	int previousBlockState() const { return hl()->previousBlockState(); }
	void setFormat(int start, int count, const QTextCharFormat& format) { hl()->setFormat(start, count, format); }
	void setFormat(int start, int count, const QColor& color) { hl()->setFormat(start, count, color); }
	void setFormat(int start, int count, const QFont& font) { hl()->setFormat(start, count, font); }

protected slots:
	void rehighlight() { hl()->rehighlight(); }

protected:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> rules;

private:
	CombinedSyntaxHighlighter* hl_;
	QTextEdit* textEdit_;
};

#endif
