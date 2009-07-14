/*
 * typographyhighlighter.cpp - automatically inserts em-dashes and double-angle quotation marks
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

#include "typographyhighlighter.h"

#include <QString>
#include <QTextCursor>
#include <QTextEdit>
#include <QKeyEvent>

#include "psioptions.h"

static const QString enableTypographyOptionPath = "options.ya.typography.enable";

TypographyHighlighter::TypographyHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit)
	: SyntaxHighlighter(parent, textEdit)
	, autoreplacer_(new YaAutoreplacer())
{
	// TODO: needed_ needs to get updated when YaAutoreplacer is changed
	needed_ = autoreplacer_->monitoredInput();
}

TypographyHighlighter::~TypographyHighlighter()
{
	delete autoreplacer_;
}

bool TypographyHighlighter::justAppliedTypography() const
{
	return textEdit()->property("just-applied-typography").toBool();
}

void TypographyHighlighter::setJustAppliedTypography(bool justAppliedTypography) const
{
	textEdit()->setProperty("just-applied-typography", QVariant(justAppliedTypography));
}

bool TypographyHighlighter::keyPressEvent(QKeyEvent* e)
{
	if (!PsiOptions::instance()->getOption(enableTypographyOptionPath).toBool()) {
		setJustAppliedTypography(false);
		return false;
	}

	// TODO: Check for presence of QTextBlockFormat (noTypographyFormat),
	// and exit in such case

	switch (e->key()) {
	case Qt::Key_Backspace: {
		if (justAppliedTypography()) {
			textEdit()->setUpdatesEnabled(false);
			setJustAppliedTypography(false);
			textEdit()->undo();
			textEdit()->setUpdatesEnabled(true);
			return true;
		}
		break;
	}
	case Qt::Key_Space: {
		QTextCursor cursor = textEdit()->textCursor();
		int index = cursor.position();
		cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 3);
		QRegExp r("([,:.\\!\\?\\s^])(-{1,2})$");
		bool doReplace = r.indexIn(cursor.selectedText()) != -1;
		cursor.setPosition(index, QTextCursor::KeepAnchor);

		if (doReplace) {
			textEdit()->setUpdatesEnabled(false);
			insertTypedText(e);
			cursor = textEdit()->textCursor();

			cursor.beginEditBlock();
			cursor.setPosition(index - r.cap(2).length(), QTextCursor::KeepAnchor);
			cursor.insertText(QString::fromUtf8("—") + QString(" "));
			cursor.endEditBlock();

			textEdit()->setTextCursor(cursor);
			textEdit()->setUpdatesEnabled(true);
			setJustAppliedTypography(true);
			return true;
		}
		break;
	}
	case Qt::Key_QuoteDbl: { // "
		QTextCursor cursor = textEdit()->textCursor();
		int index = cursor.position();
		cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
		QString q, prec = cursor.selectedText();
		if (prec.isEmpty() || prec.contains(QRegExp("[\\s:,^]"))) {
			q = QString::fromUtf8("«");
		}
		else if (prec.contains(QRegExp("[^\\s:,^]"))) {
			q = QString::fromUtf8("»");
		}
		cursor.setPosition(index, QTextCursor::KeepAnchor);

		if (!q.isNull()) {
			textEdit()->setUpdatesEnabled(false);
			insertTypedText(e);
			cursor = textEdit()->textCursor();

			cursor.beginEditBlock();
			cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
			cursor.insertText(q);

			cursor.endEditBlock();
			textEdit()->setTextCursor(cursor);
			textEdit()->setUpdatesEnabled(true);
			setJustAppliedTypography(true);
			return true;
		}
		break;
	}
	default:
		if (autoreplacer_->isOk() && needed_.contains(e->key())) {
			insertTypedText(e);
			QTextCursor cursor = textEdit()->textCursor();
			cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
			YaAutoreplace y = autoreplacer_->process(cursor.selectedText());
			if (!y.isEmpty()) {
				textEdit()->setUpdatesEnabled(false);
				cursor.beginEditBlock();
				cursor.setPosition(y.pos(), QTextCursor::MoveAnchor);
				cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, y.count());
				cursor.insertText(y.replace());	
				cursor.endEditBlock();
				textEdit()->setTextCursor(cursor);
				textEdit()->setUpdatesEnabled(true);
				setJustAppliedTypography(true);
			}
			return true;
		}
		setJustAppliedTypography(false);
	}

	return false;
}

bool TypographyHighlighter::highlightBlock(const QString &)
{
	return false;
}
