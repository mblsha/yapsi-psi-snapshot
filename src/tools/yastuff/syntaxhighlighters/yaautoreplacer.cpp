/*
 * yaautoreplacer.cpp - customizeable text auto-replacer
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

#include "yaautoreplacer.h"
#include "psioptions.h"

static const QString autoReplaceOptionPath = "options.ya.auto-replace";

//----------------------------------------------------------------------------
// YaAutoreplace
//----------------------------------------------------------------------------

YaAutoreplace::YaAutoreplace(const QString& replace, int pos, int count)
	: replace_(replace)
	, pos_(pos)
	, count_(count)
{
};

YaAutoreplace::~YaAutoreplace()
{
};

QString YaAutoreplace::replace()
{
	return replace_;
}

int YaAutoreplace::pos()
{
	return pos_;
}

int YaAutoreplace::count()
{
	return count_;
}

bool YaAutoreplace::isEmpty()
{
	return count_ <= 0;
}

//----------------------------------------------------------------------------
// YaAutoreplacer
//----------------------------------------------------------------------------

YaAutoreplacer::YaAutoreplacer()
{
	isOk_ = load();
}

YaAutoreplacer::~YaAutoreplacer()
{}

bool YaAutoreplacer::isOk()
{
	return isOk_ && !map_.isEmpty();
}

void YaAutoreplacer::addAutoreplace(const QString& key, const QString& value)
{
	map_[key] = value;
}

bool YaAutoreplacer::removeAutoreplace(const QString& key)
{
	return map_.remove(key);
}

void YaAutoreplacer::clearAutoreplaces()
{
	map_.clear();
}

YaAutoreplace YaAutoreplacer::process(const QString& s)
{
	QString result;
	QMapIterator<QString, QString> i(map_);
	while (i.hasNext()) {
		i.next();
		if (s.endsWith(i.key())) {
			return YaAutoreplace(i.value(), s.length() - i.key().length(), i.key().length());
		}
	}
	return YaAutoreplace(QString(), -1, 0);
}

QMap<QChar, bool> YaAutoreplacer::monitoredInput()
{
	QMap<QChar, bool> result;
	QMapIterator<QString, QString> i(map_);
	while (i.hasNext()) {
		i.next();
		result[i.key().at(i.key().length() -1)] = true;
	}
	return result;
}

bool YaAutoreplacer::load()
{
	map_.clear();
	QList<QVariant> list = PsiOptions::instance()->getOption(autoReplaceOptionPath).toList();
	foreach(QVariant pairsVariant, list) {
		QList<QVariant> pairs = pairsVariant.toList();
		if (pairs.count() >= 2) {
			QString key = pairs.takeFirst().toString().trimmed();
			foreach(QVariant valueVariant, pairs) {
				QString value = valueVariant.toString().trimmed();
				if (!key.isEmpty() && !value.isEmpty()) {
					map_[value] = key;
				}
			}
		}
	}
	return !map_.isEmpty();
}

bool YaAutoreplacer::save()
{
	QVariantList pairs;

	// TODO: build the reverse map
	QMap<QString, QString>::iterator i = map_.begin();
	for ( ; i != map_.end(); ++i) {
		QVariantList pair;
		pair << i.value();
		pair << i.key();
		pairs << pair;
	}

	PsiOptions::instance()->setOption(autoReplaceOptionPath, pairs);
	return false;
}
