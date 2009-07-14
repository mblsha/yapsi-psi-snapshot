/*
 * trimmablemultilinelabel.h
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

#ifndef TRIMMABLEMULTILINELABEL_H
#define TRIMMABLEMULTILINELABEL_H

#include <QFrame>

class QTextEdit;
class QFont;

class TrimmableMultilineLabel : public QFrame
{
	Q_OBJECT
public:
	TrimmableMultilineLabel(QWidget* parent);
	~TrimmableMultilineLabel();

	void doSetFont(const QFont& font);

	QString text() const;
	void setText(const QString& text);

	int minNumLines() const;
	void setMinNumLines(int minNumLines);

	int maxNumLines() const;
	void setMaxNumLines(int maxNumLines);

	void setDrawTextFadeOut(bool drawTextFadeOut, QColor backgroundColor);

	Qt::TextInteractionFlags textInteractionFlags() const;
	void setTextInteractionFlags(Qt::TextInteractionFlags);

	// reimplemented
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

	// dummy
	QString html() const { return text(); }
	void setHtml(const QString& html) { setText(html); }
	bool wordWrap() const { return true; }
	void setWordWrap(bool) {}

protected:
	// reimplemented
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);
	bool eventFilter(QObject* obj, QEvent* e);

private:
	QString text_;
	int minNumLines_;
	int maxNumLines_;
	QTextEdit* textEdit_;
	QSize sizeHint_;
	bool drawTextFadeOut_;
	QColor backgroundColor_;

	void relayout();
	QFontMetrics actualFontMetrics() const;
};

#endif
