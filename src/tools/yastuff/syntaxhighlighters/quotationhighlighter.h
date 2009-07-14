/*
 * listhighlighter.h - highlight email-style quotations
 * Copyright (C) 2008  Yandex LLC (Alexei Matiouchkine)
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

#ifndef QUOTATIONHIGHLIGHTER_H
#define QUOTATIONHIGHLIGHTER_H

#include "syntaxhighlighter.h"

class QuotationHighlighter : public SyntaxHighlighter
{
	Q_OBJECT
public:
	QuotationHighlighter(CombinedSyntaxHighlighter* parent, QTextEdit* textEdit);
	QuotationHighlighter(CombinedSyntaxHighlighter* parent, QTextDocument* textDocument);

protected:
	virtual bool highlightBlock(const QString &text);
};

#endif
