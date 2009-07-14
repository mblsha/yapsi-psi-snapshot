/*
 * yatoolbox.h
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

#ifndef YATOOLBOX_H
#define YATOOLBOX_H

#include "baseanimatedstackedwidget.h"

class QVBoxLayout;

class YaToolBoxPage;
class YaToolBoxPageButton;
class YaToolBoxAnimation;

class YaToolBox : public BaseAnimatedStackedWidget
{
	Q_OBJECT
public:
	YaToolBox(QWidget* parent);
	~YaToolBox();

	void addPage(YaToolBoxPageButton* button, QWidget* widget);

	void setCurrentPage(YaToolBoxPageButton* button);
	QLayout* layout() const;

	int currentIndex() const;
	int count() const;
	YaToolBoxPageButton* button(int index) const;

private slots:
	void setCurrentPage();
	void toggleCurrentPage();

protected:
	// reimplemented
	virtual QWidget* normalPage() const;
	virtual BaseAnimatedStackedWidgetAnimation* createAnimationPage() const;
	// virtual void init();
	virtual void setCurrentState(QWidget* widget);
	virtual void setNewState();
	virtual void setCurrentWidget_internal(QWidget* widget);
	virtual void activateNormalPageLayout();

private:
	QWidget* normalPage_;
	QVBoxLayout* layout_;
	QList<YaToolBoxPage*> pages_;
};

#endif
