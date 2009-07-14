/*
 * yaspellhighlighter.cpp - highlight typo errors
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

#include "yaspellhighlighter.h"

#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>

#include "spellchecker.h"

YaSpellHighlighter::YaSpellHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit)
	: SyntaxHighlighter(parent, textEdit)
	, rehighlightTimer_(0)
	, needRehighlight_(false)
{
	rehighlightTimer_ = new QTimer(this);
	rehighlightTimer_->setSingleShot(true);
	rehighlightTimer_->setInterval(100);
	connect(rehighlightTimer_, SIGNAL(timeout()), SLOT(rehighlight()));
	connect(textEdit, SIGNAL(cursorPositionChanged()), SLOT(cursorPositionChanged()));
}

void YaSpellHighlighter::cursorPositionChanged()
{
	if (!needRehighlight_)
		return;
	rehighlightTimer_->start();
}

bool YaSpellHighlighter::highlightBlock(const QString& text)
{
	rehighlightTimer_->stop();
	needRehighlight_ = previousBlockState() != -1;
	int cursorPosition = cursorPositionInCurrentBlock();

	// Underline 
	QTextCharFormat tcf;
	tcf.setUnderlineColor(QBrush(QColor(255,0,0)));
	tcf.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

	// Match words (minimally)
	static QRegExp expression("\\b\\w+\\b");
	static QRegExp number("^\\d+$");

	// Iterate through all words
	int index = text.indexOf(expression);
	while (index >= 0) {
		int length = expression.matchedLength();
		if (expression.cap().indexOf(number) >= 0) {
			// skipping numbers
		}
		else if (cursorPosition >= index && (cursorPosition <= index + length)) {
			needRehighlight_ = true;
		}
		else {
			if (!SpellChecker::instance()->isCorrect(expression.cap()))
				setFormat(index, length, tcf);
		}
		index = text.indexOf(expression, index + length);
	}

	setCurrentBlockState(needRehighlight_ ? 0 : -1);
	return false;
}
