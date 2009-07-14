/*
 * msgmle.cpp - subclass of PsiTextView to handle various hotkeys
 * Copyright (C) 2001-2003  Justin Karneges, Michail Pishchagin
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

#include "msgmle.h"

#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QLayout>
#include <QMenu>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTimer>
#include <QDateTime>

#include "shortcutmanager.h"
#include "spellhighlighter.h"
#include "spellchecker.h"
#include "psioptions.h"

#ifdef YAPSI
#include "typographyhighlighter.h"
#ifdef YAPSI_DEV
#include "cpphighlighter.h"
#endif
#include "wikihighlighter.h"
#include "listhighlighter.h"
#include "quotationhighlighter.h"
#include "yaspellhighlighter.h"
#include "combinedsyntaxhighlighter.h"
#endif

#ifdef YAPSI
#include <QTextFrameFormat>
#include <QTextFrame>
#include <QTextDocument>
#endif

//----------------------------------------------------------------------------
// ChatView
//----------------------------------------------------------------------------
ChatView::ChatView(QWidget *parent)
	: PsiTextView(parent)
	, dialog_(0)
{
	setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

	setReadOnly(true);
	setUndoRedoEnabled(false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifndef Q_WS_X11	// linux has this feature built-in
	connect(this, SIGNAL(selectionChanged()), SLOT(autoCopy()));
	connect(this, SIGNAL(cursorPositionChanged()), SLOT(autoCopy()));
#endif

// FIXME
#ifdef YAPSI
	CombinedSyntaxHighlighter* hl = new CombinedSyntaxHighlighter(this);
	new QuotationHighlighter(hl, this);
	new WikiHighlighter(hl, this);
#ifdef YAPSI_DEV
	new CppHighlighter(hl, this);
#endif
#endif
}

ChatView::~ChatView()
{
}

void ChatView::setDialog(QWidget* dialog)
{
	dialog_ = dialog;
}

QSize ChatView::sizeHint() const
{
	return minimumSizeHint();
}

bool ChatView::focusNextPrevChild(bool next)
{
	return QWidget::focusNextPrevChild(next);
}

void ChatView::keyPressEvent(QKeyEvent *e)
{
/*	if(e->key() == Qt::Key_Escape)
		e->ignore(); 
#ifdef Q_WS_MAC
	else if(e->key() == Qt::Key_W && e->modifiers() & Qt::ControlModifier)
		e->ignore();
	else
#endif
	else if(e->key() == Qt::Key_Return && ((e->modifiers() & Qt::ControlModifier) || (e->modifiers() & Qt::AltModifier)) )
		e->ignore();
	else if(e->key() == Qt::Key_H && (e->modifiers() & Qt::ControlModifier))
		e->ignore();
	else if(e->key() == Qt::Key_I && (e->modifiers() & Qt::ControlModifier))
		e->ignore(); */
	/*else*/ if(e->key() == Qt::Key_M && (e->modifiers() & Qt::ControlModifier) && !isReadOnly()) // newline 
		insert("\n");
/*	else if(e->key() == Qt::Key_U && (e->modifiers() & Qt::ControlModifier) && !isReadOnly())
		clear(); */
	else if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && ((e->modifiers() & Qt::ControlModifier) || (e->modifiers() & Qt::AltModifier))) {
		e->ignore();
	}
	else {
		PsiTextView::keyPressEvent(e);
	}
}

/**
 * Copies any selected text to the clipboard
 * if autoCopy is enabled and ChatView is in read-only mode.
 */
void ChatView::autoCopy()
{
	if (isReadOnly() && PsiOptions::instance()->getOption("options.ui.automatically-copy-selected-text").toBool()) {
		copy();
	}
}

/**
 * Handle KeyPress events that happen in ChatEdit widget. This is used
 * to 'fix' the copy shortcut.
 * \param object object that should receive the event
 * \param event received event
 * \param chatEdit pointer to the dialog's ChatEdit widget that receives user input
 */
bool ChatView::handleCopyEvent(QObject *object, QEvent *event, ChatEdit *chatEdit)
{
	if (object == chatEdit && event->type() == QEvent::KeyPress) {
		QKeyEvent *e = (QKeyEvent *)event;
		if ((e->key() == Qt::Key_C && (e->modifiers() & Qt::ControlModifier)) ||
		    (e->key() == Qt::Key_Insert && (e->modifiers() & Qt::ControlModifier)))
		{
			if (!chatEdit->textCursor().hasSelection() &&
			     this->textCursor().hasSelection()) 
			{
				this->copy();
				return true;
			}
		}
	}
	
	return false;
}

void ChatView::appendText(const QString &text)
{
	bool doScrollToBottom = atBottom();
	
	// prevent scrolling back to selected text when 
	// restoring selection
	int scrollbarValue = verticalScrollBar()->value();
	
	PsiTextView::appendText(text);
	
	if (doScrollToBottom)
		scrollToBottom();
	else
		verticalScrollBar()->setValue(scrollbarValue);
}

/**
 * \brief Common function for ChatDlg and GCMainDlg. FIXME: Extract common
 * chat window from both dialogs and move this function to that class.
 */
QString ChatView::formatTimeStamp(const QDateTime &time)
{
	// TODO: provide an option for user to customize
	// time stamp format
	return QString().sprintf("%02d:%02d", time.time().hour(), time.time().minute());
}

//----------------------------------------------------------------------------
// ChatEdit
//----------------------------------------------------------------------------
ChatEdit::ChatEdit(QWidget *parent)
	: QTextEdit(parent)
	, dialog_(0)
	, sendAction_(0)
	, check_spelling_(false)
	, spellhighlighter_(0)
#ifdef YAPSI
	, copyAction_(0)
	, pasteAction_(0)
	, typographyAction_(0)
	, emoticonsAction_(0)
#endif
{
	setWordWrapMode(QTextOption::WordWrap);
	setAcceptRichText(false);

	setReadOnly(false);
	setUndoRedoEnabled(true);

	setMinimumHeight(48);

// FIXME
#ifdef YAPSI
	CombinedSyntaxHighlighter* hl = new CombinedSyntaxHighlighter(this);
	new TypographyHighlighter(hl, this);
	// new QuotationHighlighter(hl, this);
	// new WikiHighlighter(hl, this);
	if (PsiOptions::instance()->getOption("options.ui.spell-check.enabled").toBool()) {
		new YaSpellHighlighter(hl, this);
	}
#ifdef YAPSI_DEV
	new CppHighlighter(hl, this);
#endif
#endif

	previous_position_ = 0;
#ifndef YAPSI
	setCheckSpelling(checkSpellingGloballyEnabled());
#endif
	connect(PsiOptions::instance(),SIGNAL(optionChanged(const QString&)),SLOT(optionsChanged()));
	optionsChanged();

#ifdef YAPSI
	copyAction_ = new QAction(tr("&Copy"), this);
	copyAction_->setShortcut(QKeySequence::Copy);
	connect(copyAction_, SIGNAL(triggered()), SLOT(copy()));
	pasteAction_ = new QAction(tr("&Paste"), this);
	pasteAction_->setShortcut(QKeySequence::Paste);
	connect(pasteAction_, SIGNAL(triggered()), SLOT(paste()));

	updateMargins();
#endif
}

ChatEdit::~ChatEdit()
{
	delete spellhighlighter_;
}

void ChatEdit::clear()
{
	QTextEdit::clear();
#ifdef YAPSI
	updateMargins();
#endif
}

#ifdef YAPSI
void ChatEdit::updateMargins()
{
	QTextFrameFormat frameFormat = document()->rootFrame()->frameFormat();
	frameFormat.setLeftMargin(10);
	frameFormat.setRightMargin(10);
	document()->rootFrame()->setFrameFormat(frameFormat);
}
#endif

void ChatEdit::setDialog(QWidget* dialog)
{
	dialog_ = dialog;
}

void ChatEdit::setSendAction(QAction* sendAction)
{
	sendAction_ = sendAction;
}

QSize ChatEdit::sizeHint() const
{
	return minimumSizeHint();
}

bool ChatEdit::checkSpellingGloballyEnabled()
{
	return (SpellChecker::instance()->available() && PsiOptions::instance()->getOption("options.ui.spell-check.enabled").toBool());
}

void ChatEdit::setCheckSpelling(bool b)
{
	check_spelling_ = b;
	if (check_spelling_) {
		if (!spellhighlighter_)
			spellhighlighter_ = new SpellHighlighter(document());
	}
	else {
		delete spellhighlighter_;
		spellhighlighter_ = 0;
	}
}

bool ChatEdit::focusNextPrevChild(bool next)
{
	return QWidget::focusNextPrevChild(next);
}

// Qt text controls are quite greedy to grab key events.
// disable that.
bool ChatEdit::event(QEvent * event) {
	if (event->type() == QEvent::ShortcutOverride) {
		return false;
	}
	return QTextEdit::event(event);
}

void ChatEdit::keyPressEvent(QKeyEvent *e)
{
/*	if(e->key() == Qt::Key_Escape || (e->key() == Qt::Key_W && e->modifiers() & Qt::ControlModifier))
		e->ignore();
	else if(e->key() == Qt::Key_Return && 
	       ((e->modifiers() & Qt::ControlModifier) 
#ifndef Q_WS_MAC
	       || (e->modifiers() & Qt::AltModifier) 
#endif
	       ))
		e->ignore();
	else if(e->key() == Qt::Key_M && (e->modifiers() & Qt::ControlModifier)) // newline
		insert("\n");
	else if(e->key() == Qt::Key_H && (e->modifiers() & Qt::ControlModifier)) // history
		e->ignore();
	else  if(e->key() == Qt::Key_S && (e->modifiers() & Qt::AltModifier))
		e->ignore();
	else*/ if(e->key() == Qt::Key_U && (e->modifiers() & Qt::ControlModifier))
		clear();
/*	else if((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && !((e->modifiers() & Qt::ShiftModifier) || (e->modifiers() & Qt::AltModifier)) && option.chatSoftReturn)
		e->ignore();
	else if((e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) && (e->modifiers() & Qt::ShiftModifier))
		e->ignore();
	else if((e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) && (e->modifiers() & Qt::ControlModifier))
		e->ignore(); */
#ifdef Q_WS_MAC
	else if (e->key() == Qt::Key_QuoteLeft && e->modifiers() == Qt::ControlModifier) {
		e->ignore();
	}
#endif
#ifdef YAPSI
	else if (e->key() == Qt::Key_Z && e->modifiers() == Qt::ControlModifier) {
		// Work-around for disappearing margins when undo in empty ChatEdit is performed
		QTextEdit::keyPressEvent(e);
		updateMargins();
	}
#endif
	else
	{
		QTextEdit::keyPressEvent(e);
	}
}

/**
 * Work around Qt bug, that QTextEdit doesn't accept() the 
 * event, so it could result in another context menu popping
 * out after the first one.
 */
void ChatEdit::contextMenuEvent(QContextMenuEvent *e) 
{
	last_click_ = e->pos();
	if (check_spelling_ && textCursor().selectedText().isEmpty() && SpellChecker::instance()->available()) {
		// Check if the word under the cursor is misspelled
		QTextCursor tc = cursorForPosition(last_click_);
		tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
		tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
		QString selected_word = tc.selectedText();
		if (!selected_word.isEmpty() && !SpellChecker::instance()->isCorrect(selected_word)) {
			QList<QString> suggestions = SpellChecker::instance()->suggestions(selected_word);
			if (!suggestions.isEmpty() || SpellChecker::instance()->writable()) {
				QMenu spell_menu;
				if (!suggestions.isEmpty()) {
					foreach(QString suggestion, suggestions) {
						QAction* act_suggestion = spell_menu.addAction(suggestion);
						connect(act_suggestion,SIGNAL(triggered()),SLOT(applySuggestion()));
					}
					spell_menu.addSeparator();
				}
				if (SpellChecker::instance()->writable()) {
					QAction* act_add = spell_menu.addAction(tr("Add to dictionary"));
					connect(act_add,SIGNAL(triggered()),SLOT(addToDictionary()));
				}
				spell_menu.exec(QCursor::pos());
				e->accept();
				return;
			}
		}
	}

	Q_ASSERT(sendAction_);
	QMenu menu;
	sendAction_->setShortcuts(ShortcutManager::instance()->shortcuts("chat.send"));
	menu.addAction(sendAction_);
#ifndef YAPSI
	menu.addSeparator();
	QMenu *standardMenu = createStandardContextMenu();
	foreach(QAction* action, standardMenu->actions()) {
		menu.addAction(action);
	}
#else
	copyAction_->setEnabled(textCursor().hasSelection());
	menu.addAction(copyAction_);
	menu.addAction(pasteAction_);
	if (typographyAction_ && emoticonsAction_) {
		menu.addSeparator();
		menu.addAction(typographyAction_);
		menu.addAction(emoticonsAction_);
	}
#endif
	menu.exec(e->globalPos());
#ifndef YAPSI
	delete standardMenu;
#endif
	e->accept();
}

/*!
 * \brief handles a click on a suggestion
 * \param the action is just the container which holds the suggestion.
 * 
 * This method is called by the framework whenever a user clicked on the child popupmenu
 * to select a suggestion for a missspelled word. It exchanges the missspelled word with the
 * suggestion which is the text of the QAction parameter.
 */
void ChatEdit::applySuggestion()
{
	QAction* act_suggestion = (QAction*) sender();
	int current_position = textCursor().position();
	
	// Replace the word
	QTextCursor	tc = cursorForPosition(last_click_);
	tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
	tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	int old_length = tc.position() - tc.anchor();
	tc.insertText(act_suggestion->text());
	tc.clearSelection();

	// Put the cursor where it belongs
	int new_length = act_suggestion->text().length();
	tc.setPosition(current_position - old_length + new_length);
	setTextCursor(tc);
}

/*!
 * \brief handles a click on the add2dict action of the parent popupmenu
 * \param Never used bool parameter
 * 
 * The method sets the cursor to the last mouseclick position and looks for the word which is placed there.
 * This word is than added to the dictionary of aspell.
 */
void ChatEdit::addToDictionary()
{
	QTextCursor	tc = cursorForPosition(last_click_);
	int current_position = textCursor().position();

	// Get the selected word
	tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
	tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	SpellChecker::instance()->add(tc.selectedText());
	
	// Put the cursor where it belongs
	tc.clearSelection();
	tc.setPosition(current_position);
	setTextCursor(tc);
}

/**
 * \todo Make spellChecker the sibling of highlighters family and uncomment the line(s) below
 */
void ChatEdit::optionsChanged()
{
#ifndef YAPSI
	setCheckSpelling(checkSpellingGloballyEnabled());
#endif
}

#ifdef YAPSI
void ChatEdit::setTypographyAction(QAction* typographyAction)
{
	typographyAction_ = typographyAction;
}

void ChatEdit::setEmoticonsAction(QAction* emoticonsAction)
{
	emoticonsAction_ = emoticonsAction;
}
#endif

//----------------------------------------------------------------------------
// LineEdit
//----------------------------------------------------------------------------
LineEdit::LineEdit( QWidget *parent)
	: ChatEdit(parent)
{
	setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // no need for horizontal scrollbar with this
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setMinimumHeight(0);

	connect(this, SIGNAL(textChanged()), SLOT(recalculateSize()));
}

LineEdit::~LineEdit()
{
}

QSize LineEdit::minimumSizeHint() const
{
	QSize sh = QTextEdit::minimumSizeHint();
	sh.setHeight(fontMetrics().height() + 1);
	sh += QSize(0, QFrame::lineWidth() * 2);
	return sh;
}

QSize LineEdit::sizeHint() const
{
	QSize sh = QTextEdit::sizeHint();
	sh.setHeight(int(document()->documentLayout()->documentSize().height()));
	sh += QSize(0, QFrame::lineWidth() * 2);
#ifndef YAPSI
	((QTextEdit*)this)->setMaximumHeight(sh.height());
#endif
	return sh;
}

void LineEdit::resizeEvent(QResizeEvent* e)
{
	ChatEdit::resizeEvent(e);
	QTimer::singleShot(0, this, SLOT(updateScrollBar()));
}

void LineEdit::recalculateSize()
{
	QSize sizeHint = this->sizeHint();
	if (sizeHint == oldSizeHint_)
		return;

	oldSizeHint_ = sizeHint;
	updateGeometry();
	QTimer::singleShot(100, this, SLOT(updateScrollBar()));
}

void LineEdit::updateScrollBar()
{
	setVerticalScrollBarPolicy(sizeHint().height() > height() ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
	ensureCursorVisible();
}
