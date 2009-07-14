/*
 * visibletext.cpp
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

#include "visibletext.h"

#include <QFontMetrics>
#include <QStringList>

static int lastSpacePosition(const QString& text, int endPos)
{
	int last = -1;
	int offset = 0;
	int pos;
	while ((pos = text.indexOf(" ", offset)) != -1) {
		if (pos <= endPos) {
			last = pos;
			offset = pos + 1;
			continue;
		}

		break;
	}

	return last;
}

static int lastWordPosition(const QString& text, int endPos, bool lastLine)
{
	int pos = lastSpacePosition(text, endPos);
	if (pos != -1 && !lastLine) {
		return pos + 1;
	}

	return endPos;
}

static QString trimLine(const QString& txt, int width, const QFontMetrics& fontMetrics)
{
	return fontMetrics.elidedText(txt + QString::fromUtf8("…"), Qt::ElideRight, width);
}

QString visibleText(const QString& fullText, const QFontMetrics& fontMetrics, int width, int height, int numLines)
{
	QStringList lines;
	QString text = fullText;
	for (int lineNum = 1; (lineNum <= numLines) && !text.isEmpty(); lineNum++) {
		bool lastLine = lineNum >= numLines || ((lineNum + 1) * fontMetrics.height()) > height;
		QString line;
		for (int i = 2; i < text.length(); i++) {
			if (fontMetrics.width(text.left(i)) > width) {
				int pos = lastWordPosition(text, i - 1, lastLine);
				line = text.left(pos);
				text = text.mid(pos);
				if (lastLine)
					line = trimLine(line, width, fontMetrics);
				break;
			}
		}

		if (line.isEmpty()) {
			line = text;
			text = QString();
		}

		lines << line;

		if (lastLine)
			break;
	}

	return lines.join("\n");
}
