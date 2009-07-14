/*
 * psirichtext.h - helper functions to handle Icons in QTextDocuments
 * Copyright (C) 2006  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QTextDocument>
#include <QTextCursor>

class TextIconFormat : public QTextCharFormat
{
public:
	TextIconFormat(const QString &iconName, const QString &text);

	enum Property {
		IconName = QTextFormat::UserProperty + 1,
		IconText = QTextFormat::UserProperty + 2,
		DrawIcon = QTextFormat::UserProperty + 3
	};

	static const int IconFormatType;
};

class PsiRichText
{
public:
	static void install(QTextDocument *doc);
	static void ensureTextLayouted(QTextDocument *doc, int documentWidth, Qt::Alignment align = Qt::AlignLeft, Qt::LayoutDirection layoutDirection = Qt::LeftToRight, bool textWordWrap = true);
	static void setText(QTextDocument *doc, const QString &text);
	static void insertIcon(QTextCursor &cursor, const QString &iconName, const QString &iconText);
	static void appendText(QTextDocument *doc, QTextCursor &cursor, const QString &text);
	static QString convertToPlainText(const QTextDocument *doc);
	static QTextCursor textIconCursorAt(QTextDocument* doc, const QPoint& pos);
	static QString iconTextAt(QTextDocument* doc, const QPoint& pos);
	static QString iconNameAt(QTextDocument* doc, const QPoint& pos);
};
