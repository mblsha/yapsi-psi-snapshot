/*
 * yaemptytextlineedit.cpp
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

#include "yaemptytextlineedit.h"

#include <QPainter>
#include <QVariant>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QApplication>

#include "yaeditorcontextmenu.h"
#include "yaokbutton.h"
#include "yaclosebutton.h"

YaEmptyTextLineEdit::YaEmptyTextLineEdit(QWidget* parent)
	: QWidget(parent)
	, okButton_(0)
	, cancelButton_(0)
	, disableFocusChanging_(false)
{
	setFocusPolicy(Qt::StrongFocus);

	QHBoxLayout* hbox = new QHBoxLayout(this);
	hbox->setSpacing(3);
	hbox->setMargin(0);

	lineEdit_ = new QLineEdit(this);
	lineEdit_->installEventFilter(this);
	hbox->addWidget(lineEdit_);

	okButton_ = new YaOkButton(this);
	okButton_->setFocusPolicy(Qt::NoFocus);
	connect(okButton_, SIGNAL(clicked()), SLOT(okButtonClicked()));
	okButton_->show();

	cancelButton_ = new YaCloseButton(this);
	cancelButton_->setFocusPolicy(Qt::NoFocus);
	connect(cancelButton_, SIGNAL(clicked()), SLOT(cancelButtonClicked()));
	cancelButton_->hide();

	// hbox->addSpacing(4);
	hbox->addWidget(okButton_);
	hbox->addWidget(cancelButton_);
}

YaEmptyTextLineEdit::~YaEmptyTextLineEdit()
{
}

QString YaEmptyTextLineEdit::emptyText() const
{
	return emptyText_;
}

void YaEmptyTextLineEdit::setEmptyText(const QString& emptyText)
{
	emptyText_ = emptyText;
	update();
}

void YaEmptyTextLineEdit::paintEvent(QPaintEvent* e)
{
	// QWidget::paintEvent(e);
	// if (text().isEmpty() && !emptyText().isEmpty()) {
		// QPainter p(this);
		// p.fillRect(rect(), Qt::red);
	// 	p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
	// 	p.drawText(rect().adjusted(10, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, emptyText());
	// }
}

void YaEmptyTextLineEdit::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);
}

void YaEmptyTextLineEdit::showEvent(QShowEvent* e)
{
	QWidget::showEvent(e);
}

void YaEmptyTextLineEdit::focusInEvent(QFocusEvent* e)
{
	lineEdit_->setFocus();
}

void YaEmptyTextLineEdit::okButtonClicked()
{
	QKeyEvent ke(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
	QCoreApplication::instance()->sendEvent(this, &ke);
}

void YaEmptyTextLineEdit::cancelButtonClicked()
{
	QKeyEvent ke(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
	QCoreApplication::instance()->sendEvent(this, &ke);
}

bool YaEmptyTextLineEdit::okButtonVisible() const
{
	return okButton_->isVisible();
}

void YaEmptyTextLineEdit::setOkButtonVisible(bool visible)
{
	okButton_->setVisible(visible);
}

bool YaEmptyTextLineEdit::cancelButtonVisible() const
{
	return cancelButton_->isVisible();
}

void YaEmptyTextLineEdit::setCancelButtonVisible(bool visible)
{
	cancelButton_->setVisible(visible);
}

QString YaEmptyTextLineEdit::text() const
{
	return lineEdit_->text();
}

void YaEmptyTextLineEdit::setText(const QString& text)
{
	lineEdit_->setText(text);
}

void YaEmptyTextLineEdit::clear()
{
	lineEdit_->clear();
}

void YaEmptyTextLineEdit::selectAll()
{
	lineEdit_->selectAll();
}

void YaEmptyTextLineEdit::setFrame(bool frame)
{
	lineEdit_->setFrame(frame);
}

void YaEmptyTextLineEdit::setCursorPosition(int position)
{
	lineEdit_->setCursorPosition(position);
}

bool YaEmptyTextLineEdit::hasSelectedText() const
{
	return lineEdit_->hasSelectedText();
}

void YaEmptyTextLineEdit::copy()
{
	lineEdit_->copy();
}

void YaEmptyTextLineEdit::paste()
{
	lineEdit_->paste();
}

void YaEmptyTextLineEdit::setEchoMode(QLineEdit::EchoMode echoMode)
{
	lineEdit_->setEchoMode(echoMode);
}

void YaEmptyTextLineEdit::deselect()
{
	lineEdit_->deselect();
}

QLineEdit* YaEmptyTextLineEdit::internalLineEdit() const
{
	return lineEdit_;
}

void YaEmptyTextLineEdit::setLineEditGeometry(const QRect& r)
{
	lineEdit_->setFixedHeight(r.height());
}

bool YaEmptyTextLineEdit::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Tab ||
		    keyEvent->key() == Qt::Key_Backtab)
		{
			if (disableFocusChanging_) {
				return true;
			}
		}
	}

	if (object == lineEdit_ && event->type() == QEvent::ContextMenu) {
		QContextMenuEvent* e = dynamic_cast<QContextMenuEvent*>(event);
		YaEditorContextMenu menu(lineEdit_);
		menu.exec(e, lineEdit_);
		return true;
	}

	// moved from YaContactListViewDelegate::eventFilter()
	// make sure data is not committed
	if (event->type() == QEvent::FocusOut) {
		if (!lineEdit_->isActiveWindow() || (QApplication::focusWidget() != lineEdit_)) {
			QWidget *w = QApplication::focusWidget();
			while (w) { // don't worry about focus changes internally in the editor
				if (w == this) {
					return false;
				}
				w = w->parentWidget();
			}
// #ifndef QT_NO_DRAGANDDROP
// 			// The window may lose focus during an drag operation.
// 			// i.e when dragging involves the taskbar on Windows.
// 			if (QDragManager::self() && QDragManager::self()->object != 0)
// 				return false;
// #endif
			// Opening a modal dialog will start a new eventloop
			// that will process the deleteLater event.
			if (QApplication::activeModalWidget() && !QApplication::activeModalWidget()->isAncestorOf(lineEdit_)) {
				return false;
			}
			emit focusOut();
		}
		return false;
	}

	return QWidget::eventFilter(object, event);
}

bool YaEmptyTextLineEdit::disableFocusChanging() const
{
	return disableFocusChanging_;
}

void YaEmptyTextLineEdit::setDisableFocusChanging(bool disableFocusChanging)
{
	disableFocusChanging_ = disableFocusChanging;
}
