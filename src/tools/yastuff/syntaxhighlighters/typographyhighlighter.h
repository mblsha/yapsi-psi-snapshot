/*
 * typographyhighlighter.h - automatically inserts em-dashes and double-angle quotation marks
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

#ifndef TYPOGRAPHYHIGHLIGHTER_H
#define TYPOGRAPHYHIGHLIGHTER_H

#include "syntaxhighlighter.h"

class QTextEdit;
class QKeyEvent;

#include <QRegExp>
#include <QMap>
#include <QTextCharFormat>

#include "yaautoreplacer.h"

class TypographyHighlighter : public SyntaxHighlighter
{
	Q_OBJECT
public:
	TypographyHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit);
	~TypographyHighlighter();

protected:
	virtual bool highlightBlock(const QString &text);
	virtual bool keyPressEvent(QKeyEvent* e);

protected:
	YaAutoreplacer* autoreplacer_;
	QMap<QChar, bool> needed_;

	bool justAppliedTypography() const;
	void setJustAppliedTypography(bool justAppliedTypography) const;
};

#endif
