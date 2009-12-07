/*
 * spellcheckingtextedit.cpp
 * Copyright (C) 2009  Yandex LLC (Michail Pishchagin)
 * based on http://doc.trolltech.com/solutions/4/qtspellcheckingtextedit/
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

#include "spellcheckingtextedit.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTextEdit>

#include "yaspellhighlighter.h"
#include "combinedsyntaxhighlighter.h"
#include "spellchecker.h"

#ifdef Q_WS_MAC
// #define SPOTLIGHT_GOOGLE
#endif

GSpellCheckingTextEdit::GSpellCheckingTextEdit(QWidget* parent)
	: QObject(parent)
	, checkSpellingEnabled_(false)
	, syntaxHighlighter_(0)
	, spellHighlighter_(0)
{
	parent_ = dynamic_cast<QTextEdit*>(parent);
	Q_ASSERT(parent_);

	syntaxHighlighter_ = new CombinedSyntaxHighlighter(parent_);

	ignoreSpellingAction_ = new QAction(tr("Ignore spelling"), this);
	learnSpellingAction_ = new QAction(tr("Learn spelling"), this);

#ifdef SPOTLIGHT_GOOGLE
	lookupSpotlightAction_ = new QAction(tr("Search in Spotlight"), this);
	lookupGoogleAction_ = new QAction(tr("Search in Google"), this);
#endif

	cutAction_ = new QAction(tr("Cut"), this);
	copyAction_ = new QAction(tr("&Copy"), this);
	pasteAction_ = new QAction(tr("&Paste"), this);
// #ifndef Q_WS_MAC
	cutAction_->setShortcuts(QKeySequence::Cut);
	copyAction_->setShortcuts(QKeySequence::Copy);
	pasteAction_->setShortcuts(QKeySequence::Paste);
// #endif
}

QMenu* GSpellCheckingTextEdit::createContextMenu(QContextMenuEvent* e, bool addTextEditingActions)
{
	QTextCursor cursor = parent_->cursorForPosition(e->pos());
	cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	parent_->setTextCursor(cursor);

	currentWord_ = cursor.selectedText();
	bool gotWord = !currentWord_.isEmpty();
	QMenu* menu = new QMenu();

	if (checkSpellingEnabled_ && !SpellChecker::instance()->isSpeltCorrectly(currentWord_, spellHighlighter_)) {
		const QStringList suggestions = SpellChecker::instance()->suggestions(currentWord_);
		if (suggestions.isEmpty()) {
			QAction* const noSuggestionsAction = menu->addAction(tr("No Guesses Found"));
			noSuggestionsAction->setEnabled(false);
		}
		else {
			foreach(const QString &suggestion, suggestions) {
				QAction* action = menu->addAction(suggestion);
				action->setProperty("is_suggestion", QVariant(true));
			}
		}

		menu->addSeparator();
		// menu->addAction(ignoreSpellingAction_);
		menu->addAction(learnSpellingAction_);
	}

#ifdef SPOTLIGHT_GOOGLE
	if (gotWord) {
		menu->addSeparator();
		menu->addAction(lookupSpotlightAction_);
		menu->addAction(lookupGoogleAction_);
	}
#endif

	if (addTextEditingActions) {
		menu->addSeparator();
		// menu->addAction(cutAction_);
		menu->addAction(copyAction_);
		menu->addAction(pasteAction_);
	}

	cutAction_->setEnabled(gotWord);
	copyAction_->setEnabled(gotWord);
	const bool gotClipboardContents = !QApplication::clipboard()->text().isEmpty();
	pasteAction_->setEnabled(gotClipboardContents);

	checkedTextCursor_ = cursor;
	connect(menu, SIGNAL(triggered(QAction*)), SLOT(suggestedWordSelected(QAction*)));
	return menu;
}

void GSpellCheckingTextEdit::contextMenuEvent(QContextMenuEvent* e)
{
	e->accept();
	QMenu* menu = createContextMenu(e, true);
	menu->exec(e->globalPos());
	delete menu;
}

void GSpellCheckingTextEdit::suggestedWordSelected(QAction* action)
{
	if (action == ignoreSpellingAction_) {
		// QtSpellCheckerBridge::ignoreSpelling(currentWord_, document());
		syntaxHighlighter_->rehighlight();
	}
	else if (action == learnSpellingAction_) {
		SpellChecker::instance()->learnSpelling(currentWord_);
		syntaxHighlighter_->rehighlight();
	}
#ifdef SPOTLIGHT_GOOGLE
	else if (action == lookupSpotlightAction_) {
		spotlight(currentWord);
	}
	else if (action == lookupGoogleAction_) {
		const QString searchString = "http://www.google.com/search?q=" + currentWord_;
		QDesktopServices::openUrl(QUrl(searchString));
	}
#endif
	else if (action == cutAction_) {
		QApplication::clipboard()->setText(currentWord_);
		checkedTextCursor_.deleteChar();
	}
	else if (action == copyAction_) {
		QApplication::clipboard()->setText(currentWord_);
	}
	else if (action == pasteAction_) {
		checkedTextCursor_.insertText(QApplication::clipboard()->text());
	}
	else {
		if (action->property("is_suggestion").toBool())
			checkedTextCursor_.insertText(action->text());
	}
}

CombinedSyntaxHighlighter* GSpellCheckingTextEdit::syntaxHighlighter() const
{
	return syntaxHighlighter_;
}

void GSpellCheckingTextEdit::setCheckSpelling(bool checkSpelling)
{
	if (checkSpellingEnabled_ != checkSpelling) {
		checkSpellingEnabled_ = checkSpelling;
		if (checkSpelling) {
			if (!spellHighlighter_)
				spellHighlighter_ = new YaSpellHighlighter(syntaxHighlighter_, parent_);
		}
		else {
			delete spellHighlighter_;
			spellHighlighter_ = 0;
		}
		syntaxHighlighter_->rehighlight();
	}
}

QAction* GSpellCheckingTextEdit::copyAction() const
{
	return copyAction_;
}

QAction* GSpellCheckingTextEdit::pasteAction() const
{
	return pasteAction_;
}
