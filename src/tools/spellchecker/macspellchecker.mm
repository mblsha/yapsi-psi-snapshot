/*
 * macspellchecker.mm
 * Copyright (C) 2006-2009  Remko Troncon, Yandex LLC (Michail Pishchagin)
 * based on http://doc.trolltech.com/solutions/4/qtspellcheckingtextedit/
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

#include <Cocoa/Cocoa.h>

#include <QStringList>

#include "macspellchecker.h"
#include "privateqt_mac.h"

#ifdef __LP64__
# define TAG_TYPE NSInteger
#else
# define TAG_TYPE int
#endif

//----------------------------------------------------------------------------
// QtTextRange
//----------------------------------------------------------------------------

QtTextRange::QtTextRange(const NSRange &nsrange)
	: index(nsrange.location), length(nsrange.length)
{ }

//----------------------------------------------------------------------------
// MacSpellChecker
//----------------------------------------------------------------------------

MacSpellChecker::MacSpellChecker()
{
}

MacSpellChecker::~MacSpellChecker()
{
}

bool MacSpellChecker::isSpeltCorrectly(const QString& word, SyntaxHighlighter* highlighter)
{
	return spellingErrorIndexes(word, highlighter, -1, 0, 1).isEmpty();
}

QList<QString> MacSpellChecker::suggestions(const QString& word)
{
	NSArray* const array = [[NSSpellChecker sharedSpellChecker] guessesForWord : (NSString *)QtCFString::toCFStringRef(word)];

	QStringList result;
	if (array) {
		for (unsigned int i = 0; i < [array count]; ++i)
			result.append(QtCFString::toQString((CFStringRef)[array objectAtIndex: i]));
	}

	return result;
}

bool MacSpellChecker::learnSpelling(const QString& word)
{
	[[NSSpellChecker sharedSpellChecker] learnWord: (NSString *)QtCFString::toCFStringRef(word)];
}

void MacSpellChecker::ignoreSpelling(const QString &word, SyntaxHighlighter* highlighter)
{
	[[NSSpellChecker sharedSpellChecker]
	ignoreWord : (NSString *)QtCFString::toCFStringRef(word)
	inSpellDocumentWithTag : reinterpret_cast<TAG_TYPE>(highlighter)];
}

bool MacSpellChecker::available() const
{
	return true;
}

bool MacSpellChecker::writable() const
{
	return false;
}

QList<QtTextRange> MacSpellChecker::spellingErrorIndexes(const QString& text, SyntaxHighlighter* highlighter, int cursorPositionInCurrentBlock, bool* needRehighlight, int blockCount)
{
	Q_UNUSED(blockCount);

	const QtCFString string(text);
	const int textLenght = text.length();

	int index = 0;
	QList<QtTextRange> ranges;
	while (index < textLenght) {
		const QtTextRange range = [[NSSpellChecker sharedSpellChecker]
		                          checkSpellingOfString: (NSString *)(CFStringRef)string
		                          startingAt: index
		                          language: nil
		                          wrap : false
		                          inSpellDocumentWithTag : reinterpret_cast<TAG_TYPE>(highlighter)
		                          wordCount : nil];
		const int rangeEnd = range.index + range.length;
		index = rangeEnd;
		if (range.index != INT_MAX) {
			if (needRehighlight) {
				*needRehighlight = true;
			}
			if (cursorPositionInCurrentBlock >= range.index && (cursorPositionInCurrentBlock <= range.index + range.length)) {
				continue;
			}
			ranges.append(range);
		}
	}
	return ranges;
}
