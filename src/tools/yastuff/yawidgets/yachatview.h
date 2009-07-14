/*
 * yachatview.h - custom chatlog widget
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

#ifndef YACHATVIEW_H
#define YACHATVIEW_H

#include <QListView>

class YaChatViewDelegate;
class QTextEdit;
class QDateTime;

class YaChatView : public QListView
{
	Q_OBJECT
public:
	YaChatView(QWidget* parent);
	~YaChatView();

	void setDialog(QWidget* dialog);
	bool handleCopyEvent(QObject* object, QEvent* event, QTextEdit* chatEdit);
	QString formatTimeStamp(const QDateTime& timeStamp) const;
	void nicksChanged(QString selfNick, QString contactNick);

	QColor textColor() const;
	void setTextColor(const QColor& textColor);

	QString selectedText();
	QString selectedHtml();
	bool hasSelectedText() const;

	// reimplemented
	void setModel(QAbstractItemModel* model);
	void doItemsLayout();

	// reimplemented
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	bool composingEventVisible() const;
	void setComposingEventVisible(bool visible);

signals:
	// TODO: TBD
	void selectionChanged();

public slots:
	void scrollToBottom();
	void scrollToTop();

protected:
	// reimplemented
	void changeEvent(QEvent* e);
	void contextMenuEvent(QContextMenuEvent* e);
	void resizeEvent(QResizeEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
	void rowsInserted(const QModelIndex& parent, int start, int end);
	void updateGeometries();
	bool event(QEvent* event);
	void paintEvent(QPaintEvent* e);
	void scrollContentsBy(int dx, int dy);

	void handleMousePress(QMouseEvent* e);
	void sendControlEvent(QMouseEvent* e);
	void sendControlEvent(const QModelIndex& index, QMouseEvent* e, const QRect& indexRect);

	void deselectIndex(const QModelIndex& index);
	void selectIndex(const QModelIndex& index, const QPoint& startPoint, const QPoint& endPoint);

private slots:
	void updateRequest();
	void linkHovered(QString);
	void linkActivated(QString);
	void updateCursor();
	void copy();
	void copyAsHtml();
	void selectAll();
	void autoScroll();
	void modelReset();
	void composingEventTimerTimeout();
	void optionChanged(const QString& option);

private:
	// reimplemented
	QRect visualRect(const QModelIndex& index) const;

	QModelIndex index(int row) const;
	YaChatViewDelegate* delegate() const;
	QRect rect(const QModelIndex& index) const;
	void clearSelectedIndices();
	void updateDelegateWidth();
	void fullSelectionPoints(const QModelIndex& index, QPoint* sp, QPoint* ep) const;

	bool autoScrollToBottom() const;
	bool autoScrollToTop() const;

	QRect composingEventRect() const;
	QRect faderRect();

	QWidget* dialog_;
	QAction* copyAction_;
	QAction* copyAsHtmlAction_;
	QAction* selectAllAction_;
	QTimer* autoScrollTimer_;
	QList<QModelIndex> selectedIndices_;
	QModelIndex selectedIndex_;
	QPoint mousePressPosition_;
	QPoint mousePressGlobalPosition_;
	QPoint mouseMoveLastGlobalPosition_;
	int autoScrollCount_;
	bool doubleClicked_;
	bool hoveringLink_;
	bool hoveringEmoticon_;
	bool hoveringText_;
	QString link_;
	bool composingEventVisible_;
	int composingEventPixmapIndex_;
	QTimer* composingEventTimer_;
};

#endif
