/*
 * spellcheckingtextedit.h
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

#ifndef SPELLCHECKINGTEXTEDIT_H
#define SPELLCHECKINGTEXTEDIT_H

#include <QTextEdit>

class CombinedSyntaxHighlighter;
class SyntaxHighlighter;

class GSpellCheckingTextEdit : public QObject
{
	Q_OBJECT
public:
	GSpellCheckingTextEdit(QWidget* parent);

	QMenu* createContextMenu(QContextMenuEvent* e, bool addTextEditingActions);
	void contextMenuEvent(QContextMenuEvent* e);

	CombinedSyntaxHighlighter* syntaxHighlighter() const;
	void setCheckSpelling(bool checkSpelling);

	QAction* copyAction() const;
	QAction* pasteAction() const;

protected slots:
	void suggestedWordSelected(QAction* action);

private:
	QTextEdit* parent_;

	bool checkSpellingEnabled_;
	QString currentWord_;
	CombinedSyntaxHighlighter* syntaxHighlighter_;
	SyntaxHighlighter* spellHighlighter_;
	QTextCursor checkedTextCursor_;

	QAction* ignoreSpellingAction_;
	QAction* learnSpellingAction_;
	QAction* lookupSpotlightAction_;
	QAction* lookupGoogleAction_;
	QAction* cutAction_;
	QAction* copyAction_;
	QAction* pasteAction_;
};

template <class BaseClass>
class BaseSpellCheckingTextEdit : public BaseClass
{
public:
	BaseSpellCheckingTextEdit(QWidget* parent = 0)
		: BaseClass(parent)
	{
		g_ = new GSpellCheckingTextEdit(this);
	}

	void setCheckSpelling(bool checkSpelling)
	{
		if (g_) {
			g_->setCheckSpelling(checkSpelling);
		}
	}

protected:
	// reimplemented
	void contextMenuEvent(QContextMenuEvent* e)
	{
		if (g_) {
			g_->contextMenuEvent(e);
		}
	}

	GSpellCheckingTextEdit* g() const
	{
		return g_;
	}

private:
	GSpellCheckingTextEdit* g_;
};

class SpellCheckingTextEdit : public BaseSpellCheckingTextEdit<QTextEdit>
{
	Q_OBJECT
public:
	SpellCheckingTextEdit(QWidget* parent = 0)
		: BaseSpellCheckingTextEdit<QTextEdit>(parent)
	{}
};

#endif
