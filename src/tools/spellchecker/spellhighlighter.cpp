#include "spellhighlighter.h"
#include "spellchecker.h"
#include "common.h"

SpellHighlighter::SpellHighlighter(QTextDocument* d) : QSyntaxHighlighter(d)
{
}

void SpellHighlighter::highlightBlock(const QString& text)
{
	// Underline 
	QTextCharFormat tcf;
	tcf.setUnderlineColor(QColor(Qt::red));
	tcf.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

	// Match words (minimally)
	QRegExp expression("\\b\\w+\\b");

	// Iterate through all words
	int index = text.indexOf(expression);
	while (index >= 0) {
		int length = expression.matchedLength();
		if (!SpellChecker::instance()->isSpeltCorrectly(expression.cap(), 0))
			setFormat(index, length, tcf);
		index = text.indexOf(expression, index + length);
	}
}
