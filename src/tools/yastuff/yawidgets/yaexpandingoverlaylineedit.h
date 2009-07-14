/*
 * yaexpandingoverlaylineedit.h
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

#ifndef YAEXPANDINGOVERLAYLINEEDIT_H
#define YAEXPANDINGOVERLAYLINEEDIT_H

#include "yaemptytextlineedit.h"

class QTimer;
class QTimeLine;

class YaExpandingOverlayLineEdit : public YaEmptyTextLineEdit
{
	Q_OBJECT
public:
	YaExpandingOverlayLineEdit(QWidget* parent);
	~YaExpandingOverlayLineEdit();

	QWidget* controller() const;
	void setController(QWidget* controller);

	QList<QWidget*> grounding() const;
	void addToGrounding(QWidget* widget);

public slots:
	// reimplemented
	void setVisible(bool visible);

signals:
	void enteredText(const QString& text);
	void cancelled();

protected:
	// reimplemented
	bool eventFilter(QObject* obj, QEvent* e);

private slots:
	void animate();
	void groundingChanged();
	void recalculateGeometry();

private:
	enum State {
		Normal = 0,
		Expanding,
		Expanded,
		Collapsing
	};
	State state_;

	QRect animatedRect() const;

	void setState(State state);
	void startAnimation(int toWidth);

	QTimeLine* expandCollapseTimeLine_;
	QTimer* updateGeometryTimer_;
	QWidget* controller_;
	QList<QWidget*> grounding_;
	QRect groundingGeometry_;
};

#endif
