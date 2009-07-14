/*
 * yatoolboxpage.h
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

#ifndef YATOOLBOXPAGE_H
#define YATOOLBOXPAGE_H

#include <QFrame>

class YaToolBoxPage;

class YaToolBoxPageButton : public QFrame
{
	Q_OBJECT
public:
	YaToolBoxPageButton();

	YaToolBoxPage* page() const;

	bool isCurrentPage() const;
	//! This function is invoked by YaToolBox in order to provide
	//! some state information
	virtual void setIsCurrentPage(bool isCurrentPage);

	//! This function is called when YaToolBoxPage changes its size
	virtual void updateMask();

signals:
	//! This signal is emitted when pageButton is clicked in order to make
	//! its page the current one
	void setCurrentPage();

	//! This signal is emitted when pageButton is clicked in order to toggle
	//! page visibility
	void toggleCurrentPage();

private:
	bool isCurrentPage_;
};

class YaToolBoxPage : public QFrame
{
public:
	YaToolBoxPage(QWidget* parent, YaToolBoxPageButton* button, QWidget* widget);

	bool isCurrentPage() const;
	void setCurrentPage(bool current);

	void resizeAndUpdateLayout(QSize size);

	YaToolBoxPageButton* button() const { return button_; }

	// reimplemented
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

protected:
	// reimplemented
	void resizeEvent(QResizeEvent*);

private:
	bool isCurrentPage_;
	YaToolBoxPageButton* button_;
	QWidget* widget_;
};

#endif
