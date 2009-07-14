/*
 * yasplitter.cpp
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

#include "yasplitter.h"

#include <QMouseEvent>
#include <QCoreApplication>
#include <QPushButton>

#include "overlaywidget.h"

class YaSplitterHandleExtra : public QPushButton
{
	Q_OBJECT
public:
	YaSplitterHandleExtra(QWidget *h, QWidget *parent)
		: QPushButton(parent)
		, handle_(h)
	{
#ifndef YAPSI_NO_STYLESHEETS
		setStyleSheet(
		"YaSplitterHandleExtra {"
		"	border-image: url(:/images/splitter/normal.png) 8px 30px 8px 30px repeat;"
		"	border-width: 8px 30px 8px 30px;"
		"	height: 30px;"
		"	width: 50px;"
		"	margin-bottom: 5px;"
		"}"
                ""
		"YaSplitterHandleExtra:hover,"
		"YaSplitterHandleExtra:pressed {"
		"	border-image: url(:/images/splitter/hover.png) 8px 30px 8px 30px repeat;"
		"	border-width: 8px 30px 8px 30px;"
		"	height: 30px;"
		"	width: 50px;"
		"	margin-bottom: 5px;"
		"}"
		);
#endif

		setFocusPolicy(Qt::NoFocus);
		setCursor(Qt::SplitVCursor);
		QPalette pal = palette();
		for (int i = 0; i < QPalette::NColorRoles; ++i) {
			QColor col = pal.color((QPalette::ColorRole)i);
			col.setAlpha(200);
			pal.setColor((QPalette::ColorRole)i, col);
		}
		setPalette(pal);
	}

	void mousePressEvent(QMouseEvent *e)
	{
		e->accept();
		QCoreApplication::sendEvent(handle_, e);
	}

	void mouseMoveEvent(QMouseEvent *e)
	{
		e->accept();
		QCoreApplication::sendEvent(handle_, e);
	}

	void mouseReleaseEvent(QMouseEvent *e)
	{
		e->accept();
		QCoreApplication::sendEvent(handle_, e);
	}

	QSize sizeHint() const
	{
		return QSize(129, 24);
	}

private:
	QWidget *handle_;
};

class YaSplitterHandleBase : public QSplitterHandle
{
public:
	YaSplitterHandleBase(QWidget* parent)
		: QSplitterHandle(Qt::Vertical, static_cast<QSplitter*>(parent))
	{
	}
};

class YaSplitterHandle : public OverlayWidget<YaSplitterHandleBase, YaSplitterHandleExtra>
{
	Q_OBJECT
public:
	YaSplitterHandle(QSplitter *parent)
		: OverlayWidget<YaSplitterHandleBase, YaSplitterHandleExtra>(parent, new YaSplitterHandleExtra(this, parent))
	{
	}

protected:
	// reimplemented
	QRect extraGeometry() const
	{
		const QSize sh = extra()->sizeHint();
		QRect r = globalRect();
		return QRect(r.x() + (width() - sh.width()) / 2,
		             r.y() + 1 + (height() - sh.height()) / 2,
		             sh.width(), sh.height());
	}

	// reimplemented
	void paintEvent(QPaintEvent* e)
	{
		YaSplitterHandleBase::paintEvent(e);
	}
};

YaSplitter::YaSplitter(QWidget* parent)
	: QSplitter(parent)
{
}

QSize YaSplitter::minimumSizeHint() const
{
	QSize sh = QSplitter::minimumSizeHint();
	for (int i = 1; i < count(); i++)
		if (handle(i))
			sh.setHeight(sh.height() + handle(i)->height());
	return sh;
}

QSplitterHandle* YaSplitter::createHandle()
{
	return new YaSplitterHandle(this);
}

#include "yasplitter.moc"
