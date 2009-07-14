/*
 * yaemptytextlineedit.h
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

#ifndef YAEMPTYTEXTLINEEDIT_H
#define YAEMPTYTEXTLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class YaEmptyTextLineEdit : public QWidget
{
	Q_OBJECT
public:
	YaEmptyTextLineEdit(QWidget* parent);
	~YaEmptyTextLineEdit();

	QString emptyText() const;
	void setEmptyText(const QString&);

	bool okButtonVisible() const;
	void setOkButtonVisible(bool visible);

	bool cancelButtonVisible() const;
	void setCancelButtonVisible(bool visible);

	bool disableFocusChanging() const;
	void setDisableFocusChanging(bool disableFocusChanging);

	QString text() const;
	void setText(const QString& text);
	void clear();
	void selectAll();
	void setFrame(bool frame);
	void setCursorPosition(int position);
	bool hasSelectedText() const;
	void copy();
	void paste();
	void setEchoMode(QLineEdit::EchoMode echoMode);
	void deselect();

	QLineEdit* internalLineEdit() const;
	void setLineEditGeometry(const QRect& r);

signals:
	void focusOut();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent* e);
	void showEvent(QShowEvent* e);
	void focusInEvent(QFocusEvent* e);
	bool eventFilter(QObject* object, QEvent* event);

private slots:
	void okButtonClicked();
	void cancelButtonClicked();

private:
	QLineEdit* lineEdit_;
	QToolButton* okButton_;
	QToolButton* cancelButton_;
	QString emptyText_;
	bool disableFocusChanging_;
};

#endif
