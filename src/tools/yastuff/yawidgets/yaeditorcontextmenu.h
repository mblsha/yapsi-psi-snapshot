/*
 * yaeditorcontextmenu.h - custom context menu for text editor widgets
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

#ifndef YAEDITORCONTEXTMENU_H
#define YAEDITORCONTEXTMENU_H

#include <QObject>

class QAction;
class QWidget;
class QTextEdit;
class QLabel;
class QLineEdit;
class QContextMenuEvent;
class QTextControl;
class QMimeData;

class YaEditorContextMenu : public QObject
{
	Q_OBJECT;
public:
	YaEditorContextMenu(QObject* parent);

	void exec(QContextMenuEvent* e, QTextEdit* textEdit);
	void exec(QContextMenuEvent* e, QLineEdit* lineEdit);
	void exec(QContextMenuEvent* e, QLabel* label);

private slots:
	void copy();
	void selectAll();
	void paste();

private:
	QWidget* widget_;
	QAction* copyAction_;
	QAction* selectAllAction_;
	QAction* pasteAction_;

	void exec(QContextMenuEvent* e);

	bool supportsEditing() const;
	bool hasSelection() const;
	bool canPaste() const;
	bool canInsertFromMimeData(const QMimeData* source) const;

	QTextEdit* textEdit() const;
	QLineEdit* lineEdit() const;
	QTextControl* textControl() const;
};

#endif
