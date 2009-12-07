/*
 * spellchecker.cpp
 *
 * Copyright (C) 2006  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * You can also redistribute and/or modify this program under the
 * terms of the Psi License, specified in the accompanied COPYING
 * file, as published by the Psi Project; either dated January 1st,
 * 2005, or (at your option) any later version.
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

#include "spellchecker.h"

#include <QCoreApplication>
#include <QRegExp>
#include <QtDebug>

#if defined(Q_WS_MAC)
#include "macspellchecker.h"
#elif defined(HAVE_ASPELL)
#include "aspellchecker.h"
#endif

#include "yandexspeller.h"
#include "textutil.h"

//----------------------------------------------------------------------------
// QtTextRange
//----------------------------------------------------------------------------

QtTextRange::QtTextRange(int index, int length)
	: index(index), length(length)
{ }

bool QtTextRange::operator==(const QtTextRange &other) const
{
	return (index == other.index && length == other.length);
}

QDebug operator<<(QDebug d, const QtTextRange &range)
{
	d << "index " << range.index << " length " << range.length;
	return d;
}

//----------------------------------------------------------------------------
// SpellChecker
//----------------------------------------------------------------------------

SpellChecker* SpellChecker::instance() 
{
	if (!instance_) {
		instance_ = new YandexSpeller();
#if 0
#ifdef Q_WS_MAC
		instance_ = new MacSpellChecker();
#elif defined(HAVE_ASPELL)
		instance_ = new ASpellChecker();
#else
		instance_ = new SpellChecker();
#endif
#endif
	}
	return instance_;
}

SpellChecker::SpellChecker()
	: QObject(QCoreApplication::instance())
{
}

SpellChecker::~SpellChecker()
{
}

bool SpellChecker::available() const
{
	return false;
}

bool SpellChecker::writable() const
{
	return false;
}

bool SpellChecker::isSpeltCorrectly(const QString&, SyntaxHighlighter* highlighter)
{
	Q_UNUSED(highlighter);
	return true;
}

QList<QString> SpellChecker::suggestions(const QString&)
{
	return QList<QString>();
}

void SpellChecker::ignoreSpelling(const QString &word, SyntaxHighlighter* highlighter)
{
	Q_UNUSED(highlighter);
	learnSpelling(word);
}

bool SpellChecker::learnSpelling(const QString&)
{
	return false;
}

static QString stripLinksFromText(const QString& _text)
{
	static QRegExp href("\\<a href.+\\>(.+)\\<\\/a\\>");
	href.setMinimal(true);

	QString text = TextUtil::linkify(_text);
	if (text != _text) {
		int index = text.indexOf(href);
		Q_ASSERT(index >= 0);
		while (index >= 0) {
			int length = href.matchedLength();
			QString unescapedUrl = TextUtil::unescape(href.capturedTexts()[1]);
			text.replace(index, length, QString(unescapedUrl.length(), ' '));
			index = text.indexOf(href, index + unescapedUrl.length());
		}
	}
	return text;
}

QList<QtTextRange> SpellChecker::splitTextIntoWords(const QString& _text, bool* needRehighlight, int cursorPositionInCurrentBlock) const
{
	QList<QtTextRange> ranges;

	static QRegExp expression("\\b\\w+\\b");
	static QRegExp number("^\\d+$");

	QString text = stripLinksFromText(_text);

	int index = text.indexOf(expression);
	while (index >= 0) {
		int length = expression.matchedLength();
		if (expression.cap().indexOf(number) >= 0) {
			// skipping numbers
		}
		else if (cursorPositionInCurrentBlock >= index && (cursorPositionInCurrentBlock <= index + length)) {
			*needRehighlight = true;
		}
		else {
			ranges << QtTextRange(index, length);
		}
		index = text.indexOf(expression, index + length);
	}

	return ranges;
}

QList<QtTextRange> SpellChecker::spellingErrorIndexes(const QString& text, SyntaxHighlighter* highlighter, int cursorPositionInCurrentBlock, bool* needRehighlight, int blockCount)
{
	Q_UNUSED(blockCount);
	Q_ASSERT(needRehighlight);

	QList<QtTextRange> ranges;

	foreach(const QtTextRange& range, splitTextIntoWords(text, needRehighlight, cursorPositionInCurrentBlock)) {
		if (!isSpeltCorrectly(text.mid(range.index, range.length), highlighter)) {
			ranges << range;
		}
	}

	return ranges;
}

SpellChecker* SpellChecker::instance_ = NULL;
