/*
 * yachatseparator.h
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

#ifndef YACHATSEPARATOR_H
#define YACHATSEPARATOR_H

#include <QWidget>
#include <QToolBar>
#include <QPushButton>

#include "overlaywidget.h"

class QTimeLine;
class QToolButton;

class Iconset;
class PsiIcon;
class ChatEdit;
class QPushButton;
class YaChatSeparator;
class YaCloseButton;
class QPainter;

class YaChatSeparatorPushButton : public QPushButton
{
	Q_OBJECT
public:
	YaChatSeparatorPushButton(const QString& text, QWidget* parent);

	// reimplemented
	QSize sizeHint() const;

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

	virtual void drawForeground(QPainter* p);
};

class YaChatSeparatorTypoButton : public YaChatSeparatorPushButton
{
	Q_OBJECT
public:
	YaChatSeparatorTypoButton(QWidget* parent);

	bool typoEnabled();
	void setTypoEnabled(bool enabled);

	// reimplemented
	QSize sizeHint() const;

protected:
	// reimplemented
	virtual void drawForeground(QPainter* p);

private:
	bool typoEnabled_;
};

class YaChatEmoticonToolBar : public QToolBar
{
	Q_OBJECT
public:
	YaChatEmoticonToolBar(QWidget* parent);
	~YaChatEmoticonToolBar();

	void setIconset(const Iconset &iconset);

signals:
	void textSelected(QString text);

private slots:
	void actionActivated();

private:
	QPointer<QStyle> style_;
};

class YaChatSeparatorExtra : public QWidget
{
	Q_OBJECT
public:
	YaChatSeparatorExtra(QWidget* parent, YaChatSeparator* overlayParent);

	void setIconset(const Iconset &iconset) { toolBar_->setIconset(iconset); }

	void setExpanded(bool expanded);
	bool isExpanded() const;

	void updateButtons();

	// reimplemented
	QSize sizeHint() const;

	void setTypographyEnabled(bool enabled);

signals:
	void textSelected(QString text);

	void addContact();
	void authContact();
	void typographyToggled();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);
	bool eventFilter(QObject* obj, QEvent* e);

signals:
	void invalidateExtraPosition();
	void expanding();
	void collapsing();

private slots:
	void collapse();
	void expand();
	void animate();

private:
	enum State {
		Normal = 0,
		Expanding,
		Expanded,
		Collapsing
	};

	QPushButton* addButton_;
	QPushButton* authButton_;
	YaChatSeparatorTypoButton* typographyButton_;

	State state_;
	YaChatSeparator* overlayParent_;
	QTimeLine* expandCollapseTimeLine_;
	YaCloseButton* closeButton_;
	YaChatEmoticonToolBar* toolBar_;
	QPixmap pixmap_;
	QPixmap background_;

	void setState(State state);
	void startAnimation(int toWidth);

	void updateChildWidgets();

	QRect pixmapRect() const;
	int normalWidth() const;
	int expandedWidth() const;
};

class YaChatSeparator : public OverlayWidget<QWidget, YaChatSeparatorExtra>
{
	Q_OBJECT
public:
	YaChatSeparator(QWidget* parent);
	~YaChatSeparator();

	void setIconset(const Iconset &iconset) { extra()->setIconset(iconset); }
	void setChatWidgets(ChatEdit* chatEdit, QWidget* chatView);
	void updateChatEditHeight();

	bool showAddButton() const;
	void setShowAddButton(bool showAddButton);

	bool showAuthButton() const;
	void setShowAuthButton(bool showAuthButton);

	// reimplemented
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

signals:
	void textSelected(QString text);

	void addContact();
	void authContact();

private slots:
	void expanding();
	void collapsing();
	void optionChanged(const QString&);
	void typographyToggled();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	bool eventFilter(QObject* obj, QEvent* e);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);

private:
	// reimplemented
	QRect extraGeometry() const;

private:
	QPixmap pixmap_;
	bool showAddButton_;
	bool showAuthButton_;

	QPointer<ChatEdit> chatEdit_;
	QPointer<QWidget> chatView_;
	QPoint mousePressPosition_;
	bool resizingChatEdit_;
	int chatEditStartHeight_;

	int numLines() const;
	int chatEditMaximumHeight() const;
	int chatEditHeight(int numLines) const;
};

#endif
