/*
 * yaautoreplacer.h - customizeable text auto-replacer
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

#include <QMap>
#include <QString>

class YaAutoreplace
{
public:
	YaAutoreplace(const QString& replace, int pos, int count);
	virtual ~YaAutoreplace();
public:
	QString replace();
	int pos();
	int count();
	bool isEmpty();
protected:
	QString replace_;
	int pos_;
	int count_;
};

class YaAutoreplacer
{
public:
	YaAutoreplacer();
	virtual ~YaAutoreplacer();
public:
	virtual bool isOk();

	virtual void addAutoreplace(const QString& key, const QString& value);
	virtual bool removeAutoreplace(const QString& key);
	void clearAutoreplaces();

	virtual YaAutoreplace process(const QString& s);
	virtual QMap<QChar, bool> monitoredInput();
protected:
	bool load();
	bool save();
protected:
	bool isOk_;
	QMap<QString, QString> map_;
};
