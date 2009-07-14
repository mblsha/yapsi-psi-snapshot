/*
 * yachatviewdelegate.h - painting of custom chatlog items
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

#ifndef YACHATVIEWDELEGATE_H
#define YACHATVIEWDELEGATE_H

#include <QItemDelegate>

struct DelegateMessageData;
class QTextDocument;
class YaChatView;

#include "yachatviewmodel.h"

class YaChatViewDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	YaChatViewDelegate(QAbstractItemView* parent);
	virtual ~YaChatViewDelegate();

	int width() const;
	void setWidth(int width);

	QColor textColor() const;
	void setTextColor(const QColor& textColor);

	void indexAboutToBeRemoved(const QModelIndex& index);
	void setParentFont(const QFont& parentFont);

	const QString& selfNick() const;
	const QString& contactNick() const;
	void nicksChanged(QString selfNick, QString contactNick);

	const QString& sendShortcutText() const;
	void setSendShortcutText(const QString& sendShortcutText);

	enum SelectedText {
		SelectedTextOnly = 0,
		SelectedTextWithHeader
	};

	QString selectedText(const QModelIndex& index, SelectedText selectedText) const;
	QString selectedHtml(const QModelIndex& index, SelectedText selectedText) const;
	bool isHoveringText(const QModelIndex& index, const QPoint& pos) const;
	bool isHoveringTime(const QModelIndex& index, const QPoint& pos) const;
	bool isHoveringEmoticon(const QModelIndex& index, const QPoint& pos) const;
	bool isHoveringDeliveryConfirmation(const QModelIndex& index, const QPoint& pos) const;
	QString messageEmoticonTextUnder(const QModelIndex& index, const QPoint& pos) const;
	QString messageEmoticonNameUnder(const QModelIndex& index, const QPoint& pos) const;

	// reimplemented
	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void sendControlEvent(const QModelIndex& index, QMouseEvent* e, const QPointF& coordinateOffset, QWidget* contextWidget);

signals:
	void updateRequest();
	void linkHovered(QString);
	void linkActivated(QString);

protected:
	void ensureDocumentLayouted(QTextDocument* doc, int documentWidth) const;

	DelegateMessageData ensureDelegateMessageData(const QModelIndex& index) const;
	void saveDelegateMessageData(const DelegateMessageData& data, const QModelIndex& index) const;
	void ensureMessageLayout(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	QPoint messageTextControlPosition(const QModelIndex& index) const;
	QString messageSenderName(bool incoming) const;
	QString messageSenderNameAsRichText(bool incoming) const;

	void drawMessageName(QPainter* painter, QRect& nameRect, bool incoming) const;
	void drawMessage(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawDateHeader(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	QSize messageSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	QSize dateHeaderSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void sendControlEventMessage(const QModelIndex& index, QMouseEvent* e, const QPointF& coordinateOffset, QWidget* contextWidget);
	QString selectedMessageText(const QModelIndex& index, SelectedText selectedText) const;
	QString selectedMessageHtml(const QModelIndex& index, SelectedText selectedText) const;

	YaChatViewModel::Type type(const QModelIndex& index) const;
	YaChatViewModel::SpooledType spooledType(const QModelIndex& index) const;

private slots:
	void updateRequest(QRectF);

private:
	int width_;

	QString selfNick_;
	QString contactNick_;
	QString sendShortcutText_;

	QColor lineColor_;
	QColor timeColor_;
	QColor nameColor_;
	QColor nameFirstLetterColor_;
	QColor contentsColor_;
	QPoint mousePressPos_;
	QColor textColor_;

	QFont nameFont_;
	QFontMetrics* nameFontMetrics_;
	QFont timeFont_;
	QFontMetrics* timeFontMetrics_;
	QFont contentsFont_;
	QFontMetrics* contentsFontMetrics_;
	QFont dateHeaderFont_;
	QFontMetrics* dateHeaderFontMetrics_;
	QFont noticeFont_;
	QFontMetrics* noticeFontMetrics_;

	bool useNoticeFont(const QModelIndex& index) const;
	QAbstractItemView* parentWidget() const;
};

#endif
