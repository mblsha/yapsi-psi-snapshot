/*
 * wikihighlighter.cpp - simple wiki-style hightligher
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

#include "wikihighlighter.h"

static const QRegExp RE_BOLD = QRegExp(QString::fromUtf8     ("[\\s^(\\[{]\\*+[\\w+\\s\\d()&_/-]+\\*+[\\s)\\]},.?!:;$]"));
static const QRegExp RE_ITALIC = QRegExp(QString::fromUtf8     ("[\\s^(\\[{]/+[\\w+\\s\\d()&_*-]+/+[\\s)\\]},.?!:;$]"));
static const QRegExp RE_UNDERLINE = QRegExp(QString::fromUtf8  ("[\\s^(\\[{]_+[\\w+\\s\\d()&*/-]+_+[\\s)\\]},.?!:;$]"));
static const QRegExp RE_STRIKEOUT = QRegExp(QString::fromUtf8("[\\s^(\\[{]\\-+[\\w+\\d()&*/_]+[\\w+\\s\\d()&*/_]+\\-+[\\s)\\]},.?!:;$]"));
static const QRegExp RE_ADDRESS = QRegExp(QString::fromUtf8    ("[\\s^(\\[{]\\w+\\:[^\\s\\(\\)]+[\\s)\\]},.?!:;]"));

WikiHighlighter::WikiHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit)
	: SyntaxHighlighter(parent, textEdit)
{
	init();
}

WikiHighlighter::WikiHighlighter(CombinedSyntaxHighlighter* parent, QTextDocument* textDocument)
	: SyntaxHighlighter(parent, textDocument)
{
	init();
}

void WikiHighlighter::init()
{
	HighlightingRule boldRule;
	boldRule.pattern = RE_BOLD;
	boldRule.format.setFontWeight(QFont::Bold);
	rules.append(boldRule);

	HighlightingRule italicRule;
	italicRule.pattern = RE_ITALIC;
	italicRule.format.setFontItalic(true);
	rules.append(italicRule);

	HighlightingRule underlineRule;
	underlineRule.pattern = RE_UNDERLINE;
	underlineRule.format.setFontUnderline(true);
	rules.append(underlineRule);

	HighlightingRule strikeoutRule;
	strikeoutRule.pattern = RE_STRIKEOUT;
	strikeoutRule.format.setFontStrikeOut(true);
	rules.append(strikeoutRule);
}

bool WikiHighlighter::highlightBlock(const QString &text)
{
	foreach(HighlightingRule rule, rules) {
		QRegExp expression(rule.pattern);
		int index = text.indexOf(expression);
		while (index >= 0) {
			int length = expression.matchedLength();
			setFormat(index + 1, 1, Qt::lightGray);
			setFormat(index + 2, length - 4, rule.format);
			setFormat(index + length - 2, 1, Qt::lightGray);
			index = text.indexOf(expression, index + length);
		}
	}
/*
	HighlightingRule addressRule;
	addressRule.pattern = RE_ADDRESS;
	addressRule.format.setFontWeight(QFont::Bold);
	addressRule.format.setFontItalic(false);
	addressRule.format.setFontUnderline(true);
	addressRule.format.setForeground(Qt::green);
	QRegExp expression(addressRule.pattern);
	int index = text.indexOf(expression);
	while (index >= 0) {
		int length = expression.matchedLength();
		setFormat(index, length, addressRule.format);
		index = text.indexOf(expression, index + length);
	}
*/
	return false;
}
