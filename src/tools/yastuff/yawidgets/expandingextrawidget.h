/*
 * expandingextrawidget.h - base class for cool expanding widgets
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

#ifndef EXPANDINGEXTRAWIDGET_H
#define EXPANDINGEXTRAWIDGET_H

#include <QWidget>

#include "customtimeline.h"

class YaSelfMoodButton;
class QMenu;

class ExpandingExtraWidget : public QWidget
{
	Q_OBJECT
public:
	ExpandingExtraWidget(QWidget* parent, QWidget* placeholderParent);
	~ExpandingExtraWidget();

	static int margin();

	bool isAnimating() const;
	virtual bool isExpanded() const = 0;

	virtual int normalHeight() const = 0;
	virtual int expandedHeight() const = 0;
	virtual int normalWidth() const = 0;
	virtual int expandedWidth() const = 0;

	// reimplemented
	virtual QSize sizeHint() const = 0;

signals:
	// Note: OverlayWidget relies on this signal's name
	void invalidateExtraPosition();

private slots:
	virtual void animate() = 0;
	virtual void animationFinished();

private:
	QWidget* placeholderParent_;
	CustomTimeLine<QSize>* expandCollapseTimeLine_;
	QList<YaSelfMoodButton*> buttons_;

protected:
	enum AnimationType {
		ExpandingAnimation = 0,
		CollapsingAnimation
	};
	// virtual void setState(State state) = 0;
	virtual void setExpandedState();
	virtual void setNormalState();
	virtual void cancelAction();
	virtual void focusOut();
	void startAnimation(AnimationType type, const QSize& toSize);
	virtual void stopAnimation();

	QWidget* placeholderParent() const;
	QSize currentAnimationFrame() const;

	int buttonsBottom() const;
	void updateButtonsPositions(const QRect& topButtonsPosition);
	void updateButtonsVisibility(bool visible);
	virtual void invalidateButtons();
	void createButtonFor(QAction* action);

	void updateMenuFromButtons(QMenu* menu);

	// reimplemented
	bool eventFilter(QObject* obj, QEvent* event);
};

#endif
