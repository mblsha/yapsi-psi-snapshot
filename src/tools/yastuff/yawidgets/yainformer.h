/*
 * yainformer.h
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

#ifndef YAINFORMER_H
#define YAINFORMER_H

#include <QObject>
#include <QModelIndex>
#include <QPointer>

class YaEventNotifierFrame;
class YaInformerButton;
class QLabel;

class YaInformer : public QObject
{
	Q_OBJECT
public:
	static YaInformer* create(const QModelIndex& index, QObject* parent);
	~YaInformer();

	bool handlesIndex(const QModelIndex& index) const;
	bool isValid() const;

	YaInformerButton* button() const;
	virtual QWidget* widget();
	QWidget* shortcutWidget() const;

	virtual bool isPersistent() const;

	virtual int desiredHeight() const;

	virtual QBrush backgroundBrush() const;
	virtual QString jid() const;
	virtual QIcon icon() const;
	virtual bool isOnline() const;
	virtual QString value() const;

	bool isVisible() const;

public slots:
	void toggle();
	void setVisible(bool visible);
	void hide();

signals:
	void activated();
	void deactivated();

private slots:
	void dataChanged(QModelIndex topLeft, QModelIndex bottomRight);
	void invalidateIndex();

protected slots:
	virtual void informerButtonClicked(bool checked);

protected:
	YaInformer(const QModelIndex& index, QObject* parent);

	void updateEventNotifierFrame();
	void updateButton();

	void setActivateShortcut(const QString& shortcut);

private:
	const QAbstractItemModel* model_;
	QString jid_;
	mutable QModelIndex index_;
	QPointer<YaInformerButton> button_;
	QPointer<QWidget> widget_;
	YaEventNotifierFrame* eventNotifierFrame_;
	QWidget* shortcutWidget_;

	void ensureIndex() const;
};

#endif
