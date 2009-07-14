/*
 * listhighlighter.cpp - highlight email-style quotations
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

#include "quotationhighlighter.h"

QuotationHighlighter::QuotationHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit)
	: SyntaxHighlighter(parent, textEdit)
{
}

QuotationHighlighter::QuotationHighlighter(CombinedSyntaxHighlighter* parent, QTextDocument* textDocument)
	: SyntaxHighlighter(parent, textDocument)
{
}

bool QuotationHighlighter::highlightBlock(const QString &text)
{
	QRegExp quoteRegExp = QRegExp("(^|\\x2028|\\r|\\n)\\s*([>/])([^\\x2028\\n\\r]*)");
	QTextCharFormat quoteFormat;
	quoteFormat.setFontItalic(true);
	int index = text.indexOf(quoteRegExp);
	while (index >= 0) {
		int len = quoteRegExp.matchedLength();
		quoteFormat.setForeground(quoteRegExp.capturedTexts()[2] == ">" ? Qt::darkRed : Qt::darkGray);
		setFormat(index, len, quoteFormat);
		index = text.indexOf(quoteRegExp, index + len);
	}

	return false;
}
