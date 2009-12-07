/*
 * yandexspeller.h 
 * Copyright (C) 2009  Michail Pishchagin
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

#ifndef YANDEXSPELLER_H
#define YANDEXSPELLER_H

#include "spellchecker.h"

#include <QStringList>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QCache>
#include "syntaxhighlighter.h"

class QHttp;

class YandexSpeller : public SpellChecker
{
	Q_OBJECT
public:
	YandexSpeller();
	~YandexSpeller();

	// reimplemented
	virtual bool available() const;
	virtual bool writable() const;
	virtual QList<QString> suggestions(const QString&);
	virtual bool isSpeltCorrectly(const QString&, SyntaxHighlighter* highlighter);
	virtual bool learnSpelling(const QString&);
	virtual QList<QtTextRange> spellingErrorIndexes(const QString& text, SyntaxHighlighter* highlighter, int cursorPositionInCurrentBlock, bool* needRehighlight, int blockCount);

private slots:
	void processRequests();
	void httpRequestFinished(int requestId, bool error);
	void saveCustomDictionary();

private:
	int httpGet(QHttp* http, const QString& urlString) const;

	struct SpellingRequest {
		SpellingRequest() {}
		SpellingRequest(const QStringList& _words, SyntaxHighlighter* _highlighter)
		: words(_words), highlighter(_highlighter) {}

		bool isNull() {
			return words.isEmpty() || highlighter.isNull();
		}

		QStringList words;
		QPointer<SyntaxHighlighter> highlighter;
	};

	QCache<QString, QStringList> spellingSuggestions_;
	QList<SpellingRequest> requestQueue_;
	QHash<QString, bool> customDictionary_;

	QTimer* processRequestsTimer_;
	QTimer* saveCustomDictionaryTimer_;
	QHttp* http_;
	int httpRequestId_;
	SpellingRequest currentRequest_;
};

#endif