/*
 * yaeventnotifier.h - event notifier widget
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

#ifndef YAEVENTNOTIFIER_H
#define YAEVENTNOTIFIER_H

#include <QWidget>
#include <QPointer>

#include "yainformer.h"

class PsiCon;
class PsiContactList;
class PsiEvent;
class YaEventNotifierFrame;
class YaEventNotifier;

class YaEventNotifierInformer : public YaInformer
{
	Q_OBJECT
public:
	YaEventNotifierInformer(QObject* parent);
	~YaEventNotifierInformer();

	YaEventNotifier* notifier() const;

	// reimplemented
	virtual QWidget* widget();
	virtual bool isPersistent() const;
	virtual QString jid() const;
	virtual QIcon icon() const;
	virtual bool isOnline() const;
	virtual QString value() const;
	virtual int desiredHeight() const;

private slots:
	void notifierVisibilityChanged();

private:
	QPointer<YaEventNotifier> notifier_;
};

class YaEventNotifier : public QWidget
{
	Q_OBJECT
public:
	YaEventNotifier(QWidget* parent);
	~YaEventNotifier();

	void setController(PsiCon* psi);

	int numUnreadEvents() const;

signals:
	void visibilityChanged();

protected:
	// reimplemented
	void showEvent(QShowEvent*);

private slots:
	void update();

	void skip();
	void skipAll();
	void read();

private:
	int currentEventId() const;
	PsiEvent* currentEvent() const;
	void updateSkippedList();
	QList<int> unskippedIds() const;
	bool shouldBeVisible() const;

	YaEventNotifierFrame* eventNotifierFrame_;
	PsiCon* psi_;
	QList<int> skipped_ids_;

	friend class YaEventNotifierInformer;
};

#endif
