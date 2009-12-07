/*
 * yandexspeller.cpp
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

#include "yandexspeller.h"

#include <QHttp>
#include <QUrl>
#include <QVariant>
#include <QDomDocument>
#include <QTimer>
#include <QMutableListIterator>
#include <QSettings>

#ifdef YAPSI
#include "psioptions.h"
static const QString customDictionaryOptionPath = "options.ui.spell-check.custom-dictionary";
#include "applicationinfo.h"
#endif

static const int MAX_WORDS_PER_REQUEST = 100;
#define USE_HTTPS

YandexSpeller::YandexSpeller()
	: SpellChecker()
	, http_(0)
	, httpRequestId_(-1)
{
	http_ = new QHttp(this);
	connect(http_, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));

	spellingSuggestions_.setMaxCost(100000);

	processRequestsTimer_ = new QTimer(this);
	processRequestsTimer_->setSingleShot(true);
	processRequestsTimer_->setInterval(500);
	connect(processRequestsTimer_, SIGNAL(timeout()), SLOT(processRequests()));

	saveCustomDictionaryTimer_ = new QTimer(this);
	saveCustomDictionaryTimer_->setSingleShot(true);
	saveCustomDictionaryTimer_->setInterval(1000);
	connect(saveCustomDictionaryTimer_, SIGNAL(timeout()), SLOT(saveCustomDictionary()));

	customDictionary_.clear();
	QStringList stringList;
#ifdef YAPSI
	stringList = PsiOptions::instance()->getOption(customDictionaryOptionPath).toStringList();
#else
	QSettings settings("Yandex", "QtSpeller");
	stringList = settings.value("customDictionary", QStringList()).toStringList();
#endif
	foreach(const QString& word, stringList) {
		customDictionary_[word] = true;
	}
}

YandexSpeller::~YandexSpeller()
{
#ifndef YAPSI
	saveCustomDictionary();
#endif
}

void YandexSpeller::saveCustomDictionary()
{
	QStringList words;
	QHashIterator<QString, bool> it(customDictionary_);
	while (it.hasNext()) {
		it.next();
		words << it.key();
	}
#ifdef YAPSI
	PsiOptions::instance()->setOption(customDictionaryOptionPath, words);
#else
	QSettings settings("Yandex", "QtSpeller");
	settings.setValue("customDictionary", words);
#endif
}

bool YandexSpeller::available() const
{
	return true;
}

bool YandexSpeller::writable() const
{
	return true;
}

QList<QString> YandexSpeller::suggestions(const QString& word)
{
	if (spellingSuggestions_.contains(word)) {
		QStringList* list = spellingSuggestions_[word];
		if (list) {
			return *list;
		}
	}
	return QList<QString>();
}

bool YandexSpeller::isSpeltCorrectly(const QString& word, SyntaxHighlighter* highlighter)
{
	return customDictionary_.contains(word) ||
	       !spellingSuggestions_.contains(word);
}

bool YandexSpeller::learnSpelling(const QString& word)
{
	customDictionary_[word] = true;
	saveCustomDictionaryTimer_->start();
	return true;
}

QList<QtTextRange> YandexSpeller::spellingErrorIndexes(const QString& text, SyntaxHighlighter* highlighter, int cursorPositionInCurrentBlock, bool* needRehighlight, int blockCount)
{
	Q_UNUSED(blockCount);
	Q_ASSERT(needRehighlight);

	QList<QtTextRange> ranges;

	QStringList wordsToCheck;

	foreach(const QtTextRange& range, splitTextIntoWords(text, needRehighlight, cursorPositionInCurrentBlock)) {
		QString word = text.mid(range.index, range.length);
		if (spellingSuggestions_.contains(word)) {
			if (!customDictionary_.contains(word) && spellingSuggestions_[word]) {
				*needRehighlight = true;
				ranges << range;
			}
		}
		else {
			*needRehighlight = true;
			if (wordsToCheck.length() < MAX_WORDS_PER_REQUEST) {
				wordsToCheck << word;
			}
		}
	}

	if (!wordsToCheck.isEmpty()) {
		QMutableListIterator<SpellingRequest> it(requestQueue_);
		while (it.hasNext()) {
			SpellingRequest r = it.next();
			if (r.highlighter == highlighter) {
				it.remove();
			}
		}

		requestQueue_ << SpellingRequest(wordsToCheck, highlighter);
		processRequestsTimer_->start();
		*needRehighlight = true;
	}

	return ranges;
}

void YandexSpeller::processRequests()
{
	if (!currentRequest_.isNull()) {
		processRequestsTimer_->start();
		return;
	}

	if (!requestQueue_.isEmpty()) {
		currentRequest_ = requestQueue_.takeFirst();
		QString scheme = "http";
#ifdef USE_HTTPS
		scheme = "https";
#endif
		const int BY_WORDS = 0x100;
		int options = BY_WORDS;
		httpRequestId_ = httpGet(http_, QString("%3://speller.yandex.net/services/spellservice/checkText?options=%1&text=%2")
		                                       .arg(options)
		                                       .arg(currentRequest_.words.join(" "))
		                                       .arg(scheme));
	}
}

// copied from afisha-cinema/httphelpers.cpp
int YandexSpeller::httpGet(QHttp* http, const QString& urlString) const
{
	QUrl url(urlString, QUrl::TolerantMode);
	// Q_ASSERT(url.hasQuery());

	QString query = url.encodedQuery();
	query.replace("?", "&"); // FIXME: Bug in Qt 4.4.2?

	QString fullUri = url.path();
	if (!query.isEmpty()) {
		fullUri += "?" + query;
	}

	QHttpRequestHeader header("GET", fullUri);
#ifdef YAPSI
	header.setValue("User-Agent", QString("Ya.Online %1").arg(ApplicationInfo::version()));
#else
	header.setValue("User-Agent", "QtSpeller");
#endif
	header.setValue("Host", url.port() == -1 ? url.host() : QString("%1:%2").arg(url.host(), url.port()));
	header.setValue("Accept-Language", "en-us");
	header.setValue("Accept", "*/*");

	QByteArray bytes;

	int contentLength = bytes.length();
	header.setContentLength(contentLength);

	int defaultPort = 80;
#ifdef USE_HTTPS
	defaultPort = 443;
#endif
	http->setHost(url.host(),
#ifdef USE_HTTPS
		QHttp::ConnectionModeHttps,
#else
		QHttp::ConnectionModeHttp,
#endif
		url.port() == -1 ? defaultPort : url.port());
	return http->request(header, bytes);
}

void YandexSpeller::httpRequestFinished(int requestId, bool error)
{
	if (error) {
		qWarning("YandexSpeller::httpRequestFinished(): error");
		currentRequest_ = SpellingRequest();
		return;
	}

	if (requestId != httpRequestId_) {
		return;
	}

	QByteArray data = http_->readAll();
	QString text = QString::fromUtf8(data);

	QDomDocument doc;
	if (doc.setContent(data)) {
		QDomElement root = doc.documentElement();
		if (root.tagName() == "SpellResult") {
			QStringList correctWords = currentRequest_.words;
			for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement e = n.toElement();
				if (e.isNull() || e.tagName() != "error")
					continue;

				QString word;
				QStringList* suggestions = new QStringList();
				for (QDomNode n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
					QDomElement e2 = n2.toElement();
					if (e2.isNull())
						continue;
					if (e2.tagName() == "word")
						word = e2.text();
					if (e2.tagName() == "s")
						suggestions->append(e2.text());
				}
				correctWords.removeAll(word);
				spellingSuggestions_.insert(word, suggestions);
			}

			foreach(const QString& word, correctWords) {
				spellingSuggestions_.insert(word, 0);
			}

			if (currentRequest_.highlighter) {
				currentRequest_.highlighter->rehighlight();
			}
		}
	}

	http_->close();
	currentRequest_ = SpellingRequest();
}
