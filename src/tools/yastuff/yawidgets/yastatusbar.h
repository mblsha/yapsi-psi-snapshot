/*
 * yastatusbar.h
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

#ifndef YASTATUSBAR_H
#define YASTATUSBAR_H

#include <QFrame>

class QAbstractItemModel;
class QVBoxLayout;
class QHBoxLayout;
class QPaintEvent;
class QSizeGrip;
class QStackedWidget;

class PsiCon;
class YaInformer;

#include <QModelIndex>
#include "yabusywidget.h"

class YaStatusBar : virtual public QFrame
{
	Q_OBJECT
public:
	YaStatusBar(QWidget* parent);
	~YaStatusBar();

	void setController(PsiCon* controller);
	void setModel(QAbstractItemModel* model);

	void addInformer(YaInformer* informer);

	void setShouldBeVisible(bool shouldBeVisible);

	// reimplemented
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	virtual void paintEvent(QPaintEvent*);

private slots:
	void modelDestroyed();
	void dataChanged(QModelIndex, QModelIndex);
	void rowsInserted(QModelIndex, int, int);
	void rowsRemoved(QModelIndex, int, int);

	void reset();
	void doItemsLayout();

	void informerActivated();
	void informerDeactivated();

	void updateBusyWidget();

	void optionChanged(const QString& option);

protected:
	// reimplemented
	void resizeEvent(QResizeEvent*);

protected:
	PsiCon* controller_;
	QAbstractItemModel* model_;
	QStackedWidget* stack_;
	QVBoxLayout* topLayout_;
	QLayout* layout_;
	QSizeGrip* sizeGrip_;
	YaBusyWidget* busyWidget_;
	QWidget* actualStatusBar_;
	bool layoutDirty_;
	bool shouldBeVisible_;
	QList<YaInformer*> informers_;
	QList<YaInformer*> persistentInformers_;

	QList<YaInformer*> allInformers() const;

	void removeInvalidInformers();
	void addInformerHelper(YaInformer* informer);
	void addInformerButton(YaInformer* informer);

	void updateVisibility();
};

#endif
