/*
 * msgmle.h - subclass of PsiTextView to handle various hotkeys
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

#ifndef MSGMLE_H
#define MSGMLE_H

#include <QTextEdit>

#include "psitextview.h"
#include "spellcheckingtextedit.h"

class ChatEdit;
class QEvent;
class QKeyEvent;
class QResizeEvent;
class QTimer;
class SpellHighlighter;

class ChatView : public PsiTextView
{
	Q_OBJECT
public:
	ChatView(QWidget* parent);
	~ChatView();

	void setDialog(QWidget* dialog);

	// reimplemented
	QSize sizeHint() const;

	void appendText(const QString &text);
	bool handleCopyEvent(QObject *object, QEvent *event, ChatEdit *chatEdit);

	QString formatTimeStamp(const QDateTime &time);

protected:
	// override the tab/esc behavior
	bool focusNextPrevChild(bool next);
	void keyPressEvent(QKeyEvent *);

protected slots:
	void autoCopy();

private:
	QWidget* dialog_;
};

class ChatEdit : public BaseSpellCheckingTextEdit<QTextEdit>
{
	Q_OBJECT

public:
	ChatEdit(QWidget* parent);
	~ChatEdit();

	void setDialog(QWidget* dialog);
	void setSendAction(QAction* sendAction);

	// reimplemented
	QSize sizeHint() const;

#ifdef YAPSI
	void setTypographyAction(QAction* typographyAction);
	void setEmoticonsAction(QAction* emoticonsAction);
	void setCheckSpellingAction(QAction* checkSpellingAction);
	void setSendButtonEnabledAction(QAction* sendButtonEnabledAction);
#endif

	QWidgetList contextMenuWidgets() const;
	void setContextMenuWidgets(QWidgetList contextMenuWidgets);

public slots:
	void clear();

protected slots:
	void optionChanged(const QString& option);

protected:
	// override the tab/esc behavior
	bool focusNextPrevChild(bool next);
	void keyPressEvent(QKeyEvent *);
	bool event(QEvent * event);
	void contextMenuEvent(QContextMenuEvent *e);
	bool eventFilter(QObject* o, QEvent* e);

private:
	QWidget	*dialog_;
	QAction* sendAction_;
	bool check_spelling_;
	SpellHighlighter* spellhighlighter_;
	QWidgetList contextMenuWidgets_;

#ifdef YAPSI
	QAction* typographyAction_;
	QAction* emoticonsAction_;
	QAction* checkSpellingAction_;
	QAction* sendButtonEnabledAction_;

	void updateMargins();
#endif
};


class LineEdit : public ChatEdit
{
	Q_OBJECT
public:
	LineEdit(QWidget* parent);
	~LineEdit();

	// reimplemented
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

protected:
	// reimplemented
	void resizeEvent(QResizeEvent*);

private slots:
	void recalculateSize();
	void updateScrollBar();

private:
	QSize oldSizeHint_;
};

#endif
