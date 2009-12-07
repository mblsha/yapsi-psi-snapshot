/*
 * yachatedit.h
 * Copyright (C) 2009  Yandex LLC (Michail Pishchagin)
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

#ifndef YACHATEDIT_H
#define YACHATEDIT_H

#include <QFrame>
#include <QPointer>

class LineEdit;
class ChatEdit;
class YaChatSendButton;
class YaChatSeparator;
class QAction;
class QScrollBar;

class YaChatEdit : public QFrame
{
	Q_OBJECT
public:
	YaChatEdit(QWidget* parent);
	~YaChatEdit();

	ChatEdit* chatEdit() const;
	YaChatSeparator* separator() const;

	void setSendAction(QAction* sendAction);

protected:
	// reimplemented
	void resizeEvent(QResizeEvent* e);

private slots:
	void optionChanged(const QString& option);
	void resized();

private:
	LineEdit* mle_;
	YaChatSendButton* sendButton_;
	YaChatSeparator* separator_;
	QScrollBar* stubScrollBar_;
};

#endif
