/*
 * macspellchecker.h
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

#ifndef MACSPELLCHECKER_H
#define MACSPELLCHECKER_H

#include <QList>
#include <QString>

#include "spellchecker.h"

class QtMacCocoaAutoReleasePool;

class MacSpellChecker : public SpellChecker
{
public:
	MacSpellChecker();
	~MacSpellChecker();

	// reimplemented
	virtual QList<QString> suggestions(const QString&);
	virtual bool isSpeltCorrectly(const QString&, SyntaxHighlighter* highlighter);
	virtual void ignoreSpelling(const QString &word, SyntaxHighlighter* highlighter);
	virtual bool learnSpelling(const QString&);
	virtual bool available() const;
	virtual bool writable() const;
	virtual QList<QtTextRange> spellingErrorIndexes(const QString& text, SyntaxHighlighter* highlighter, int cursorPositionInCurrentBlock, bool* needRehighlight, int blockCount);

private:
	QtMacCocoaAutoReleasePool pool_;
};

#endif
