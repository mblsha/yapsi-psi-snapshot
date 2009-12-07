/*
 * yachatedit.cpp
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

#include "yachatedit.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOption>
#include <QPlastiqueStyle>
#include <QApplication>

#include "msgmle.h"
#include "yachatseparator.h"
#include "yachatsendbutton.h"
#include "psioptions.h"

//----------------------------------------------------------------------------
// YaChatEditFakeScrollBar
//----------------------------------------------------------------------------

class YaChatEditFakeScrollBar : public QScrollBar
{
	Q_OBJECT
public:
	YaChatEditFakeScrollBar(QWidget* parent)
		: couldBeVisible_(true)
	{
	}

	void setCouldBeVisible(bool couldBeVisible)
	{
		couldBeVisible_ = couldBeVisible;
		updateGeometry();
		updateStubScrollBarVisibility();
	}

	void setStubScrollBar(QScrollBar* stubScrollBar)
	{
		stubScrollBar_ = stubScrollBar;
		stubScrollBar_->hide();

		connect(this, SIGNAL(rangeChanged(int, int)), this, SLOT(updateStubRange()));
		connect(this, SIGNAL(valueChanged(int)), stubScrollBar_, SLOT(setValue(int)));
		connect(stubScrollBar_, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
	}

	// reimplemented
	bool event(QEvent* e)
	{
		bool result = QScrollBar::event(e);
		if (e->type() == QEvent::Show || e->type() == QEvent::Hide) {
			updateStubScrollBarVisibility();
		}
		return result;
	}

	void updateStubScrollBarVisibility()
	{
		if (stubScrollBar_) {
			stubScrollBar_->setVisible(!couldBeVisible_ && isVisible());
		}
	}

private slots:
	void updateStubRange()
	{
		stubScrollBar_->setRange(minimum(), maximum());
	}

public:
	// reimplemented
	QSize sizeHint() const
	{
		QSize sh = QScrollBar::sizeHint();
		if (!couldBeVisible_)
			sh.setWidth(0);
		return sh;
	}

	QSize minimumSizeHint() const
	{
		QSize sh = QScrollBar::minimumSizeHint();
		if (!couldBeVisible_)
			sh.setWidth(0);
		return sh;
	}

	void paintEvent(QPaintEvent* e)
	{
		if (couldBeVisible_) {
			QScrollBar::paintEvent(e);
		}
	}

private:
	bool couldBeVisible_;
	QPointer<QScrollBar> stubScrollBar_;
};

//----------------------------------------------------------------------------
// YaChatEdit
//----------------------------------------------------------------------------

static const QString sendButtonEnabledOptionPath = "options.ya.chat-window.send-button.enabled";

YaChatEdit::YaChatEdit(QWidget* parent)
	: QFrame(parent)
{
	setObjectName(QString::fromUtf8("bottomFrame"));
	QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Maximum);
	sizePolicy4.setHorizontalStretch(0);
	sizePolicy4.setVerticalStretch(0);
	sizePolicy4.setHeightForWidth(sizePolicy().hasHeightForWidth());
	setSizePolicy(sizePolicy4);
	setFrameShape(QFrame::NoFrame);
	setFrameShadow(QFrame::Raised);
	QVBoxLayout* vboxLayout6 = new QVBoxLayout(this);
	vboxLayout6->setSpacing(0);
	vboxLayout6->setContentsMargins(0, 0, 0, 0);
	vboxLayout6->setObjectName(QString::fromUtf8("vboxLayout6"));
	separator_ = new YaChatSeparator(this);
	separator_->setObjectName(QString::fromUtf8("separator"));

	vboxLayout6->addWidget(separator_);

	QHBoxLayout* horizontalLayout = new QHBoxLayout();
	horizontalLayout->setSpacing(0);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	mle_ = new LineEdit(this);
	mle_->setObjectName(QString::fromUtf8("mle"));
	mle_->setFrameShape(QFrame::NoFrame);

	horizontalLayout->addWidget(mle_);

	QVBoxLayout* vboxLayout7 = new QVBoxLayout();
	vboxLayout7->setSpacing(0);
	vboxLayout7->setObjectName(QString::fromUtf8("vboxLayout7"));
	QSpacerItem* verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

	vboxLayout7->addItem(verticalSpacer);

	sendButton_ = new YaChatSendButton(this);
	sendButton_->setObjectName(QString::fromUtf8("sendButton"));

	vboxLayout7->addWidget(sendButton_);

	horizontalLayout->addLayout(vboxLayout7);
	vboxLayout6->addLayout(horizontalLayout);

	QWidgetList contextMenuWidgets;
	contextMenuWidgets << sendButton_;
	contextMenuWidgets << this;
	chatEdit()->setContextMenuWidgets(contextMenuWidgets);

	YaChatEditFakeScrollBar* fakeScrollBar = new YaChatEditFakeScrollBar(0);
	chatEdit()->setVerticalScrollBar(fakeScrollBar);

	stubScrollBar_ = new QScrollBar(this);
	fakeScrollBar->setStubScrollBar(stubScrollBar_);

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
	optionChanged(sendButtonEnabledOptionPath);
}

YaChatEdit::~YaChatEdit()
{
}

ChatEdit* YaChatEdit::chatEdit() const
{
	return mle_;
}

YaChatSeparator* YaChatEdit::separator() const
{
	return separator_;
}

void YaChatEdit::setSendAction(QAction* sendAction)
{
	sendButton_->setAction(sendAction);
}

void YaChatEdit::optionChanged(const QString& option)
{
	if (option == sendButtonEnabledOptionPath) {
		setUpdatesEnabled(false);
		bool sendButtonEnabled = PsiOptions::instance()->getOption(sendButtonEnabledOptionPath).toBool();
		sendButton_->setVisible(sendButtonEnabled);

		YaChatEditFakeScrollBar* fakeScrollBar = dynamic_cast<YaChatEditFakeScrollBar*>(chatEdit()->verticalScrollBar());
		Q_ASSERT(fakeScrollBar);
		fakeScrollBar->setCouldBeVisible(!sendButtonEnabled);
		chatEdit()->setVerticalScrollBarPolicy(chatEdit()->verticalScrollBarPolicy());

		resized();
		setUpdatesEnabled(true);
	}
}

void YaChatEdit::resized()
{
	QRect stubScrollBarRect(QPoint(0, 0), stubScrollBar_->sizeHint());
	stubScrollBarRect.moveTop(separator_->frameGeometry().bottom());
	stubScrollBarRect.moveRight(rect().right());
	stubScrollBarRect.setBottom(sendButton_->frameGeometry().top());
	stubScrollBar_->setGeometry(stubScrollBarRect);
}

void YaChatEdit::resizeEvent(QResizeEvent* e)
{
	QFrame::resizeEvent(e);
	resized();
}

#include "yachatedit.moc"
