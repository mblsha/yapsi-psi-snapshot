/*
 * combinedsyntaxhighlighter.h - a class to apply multiple syntax highlighters on a single QTextEdit
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

#ifndef COMBINEDSYNTAXHIGHLIGHTER_H
#define COMBINEDSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class CombinedSyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT
public:
	CombinedSyntaxHighlighter(QTextEdit* parent);
	CombinedSyntaxHighlighter(QTextDocument* parent);

protected:
	// reimplemented
	virtual void highlightBlock(const QString &text);

protected:
	void setCurrentBlockState(int newState) { QSyntaxHighlighter::setCurrentBlockState(newState); }
	int previousBlockState() const { return QSyntaxHighlighter::previousBlockState(); }
	void setFormat(int start, int count, const QTextCharFormat& format) { QSyntaxHighlighter::setFormat(start, count, format); }
	void setFormat(int start, int count, const QColor& color) { QSyntaxHighlighter::setFormat(start, count, color); }
	void setFormat(int start, int count, const QFont& font) { QSyntaxHighlighter::setFormat(start, count, font); }

	friend class SyntaxHighlighter;
};

#endif
