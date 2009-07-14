/*
 * yachatcontactstatus.cpp - contact status widget
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

#include "yachatcontactstatus.h"

#include <QPainter>
#include <QTextDocument> // for Qt::escape()

#include "xmpp_status.h"
#include "yacommon.h"
#include "textutil.h"
#include "yavisualutil.h"
#include "yastyle.h"
#include "textutil.h"
#include "yaeditorcontextmenu.h"
#include "yaselfmood.h"

class YaChatContactStatusExtraLabel : public QLabel
{
	Q_OBJECT
public:
	YaChatContactStatusExtraLabel(QWidget* parent)
		: QLabel(parent)
	{
	}

protected:
	// reimplemented
	void paintEvent(QPaintEvent* e)
	{
		QLabel::paintEvent(e);

		if (rect().width() < sizeHint().width()) {
			QPainter p(this);
			Ya::VisualUtil::drawTextFadeOut(&p, rect(), Ya::VisualUtil::blueBackgroundColor());
		}
	}

	// reimplemented
	void contextMenuEvent(QContextMenuEvent* e)
	{
		YaEditorContextMenu menu(this);
		menu.exec(e, this);
	}
};

//----------------------------------------------------------------------------
// YaChatContactStatusExtra
//----------------------------------------------------------------------------

YaChatContactStatusExtra::YaChatContactStatusExtra(QWidget *parent)
	: QWidget(parent)
	, status_(XMPP::Status::Offline)
	, gender_(XMPP::VCard::UnknownGender)
{
	icon_ = new QLabel(this);
	icon_->setFixedSize(16, 16);

	label_ = new YaChatContactStatusExtraLabel(this);
	label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
	label_->setCursor(Qt::IBeamCursor);
	// TODO: probably YaChatContactStatusExtraLabel shouldn't steal focus, but then
	// it'll have to display no shortcuts in the context menu too
	// label_->setFocusPolicy(Qt::NoFocus);
	label_->setWordWrap(false);
	QPalette pal = label_->palette();
	pal = YaStyle::useNativeTextSelectionColors(pal);
	label_->setPalette(pal);
}

XMPP::Status::Type YaChatContactStatusExtra::status() const
{
	return status_;
}

void YaChatContactStatusExtra::setStatus(XMPP::Status::Type status, const QString& _msg, XMPP::VCard::Gender gender)
{
	status_ = status;
	gender_ = gender;
	icon_->setPixmap(Ya::VisualUtil::rosterStatusPixmap(status));
	icon_->setVisible(status_ != XMPP::Status::Online);

	QString msg = TextUtil::plain2richSimple(_msg.simplified());
	// if (msg.isEmpty())
	// 	msg = trUtf8("", "The mood is empty.");

	QString txt;
	if (!statusText(status).isEmpty()) {
		if (msg.isEmpty())
			txt = statusText(status);
		else
			txt = trUtf8("<b>%1</b> - %2").arg(statusText(status)).arg(msg);
	}
	else {
		txt = msg;
	}

	labelText_ = txt;
	label_->setText(QString("<qt>%1</qt>").arg(txt).simplified());

	QFont f = Ya::VisualUtil::normalFont();
	// f.setStyleStrategy(QFont::PreferAntialias);
	f.setPixelSize(10);
	f.setBold(true);
	// f.setItalic(true);
	label_->setFont(f);
	emit invalidateExtraPosition();
}

QString YaChatContactStatusExtra::statusText(XMPP::Status::Type status) const
{
	if (showStatusDescription(status) && !Ya::statusDescription(status).isEmpty()) {
		return trUtf8("<font color=\"%1\">%2</font>").arg(Ya::statusColor(status).name(), Ya::statusDescription(status, gender_));
	}
	return QString();
}

bool YaChatContactStatusExtra::showStatusDescription(XMPP::Status::Type status) const
{
	return status != XMPP::Status::Online;
}

bool YaChatContactStatusExtra::shouldBeVisible() const
{
	return !labelText_.isEmpty() || showStatusDescription(status_);
}

static int marginW = 5;
static int leftMargin = 22;
static int rightMargin = 10;
static int topMargin = 10;
static int bottomMargin = 10;

QSize YaChatContactStatusExtra::sizeHint() const
{
	QSize sh = label_->sizeHint();
	if (icon_->isVisible()) {
		sh.setWidth(sh.width() + icon_->width());
	}

	sh.setWidth(sh.width() + leftMargin + rightMargin);
	sh.setHeight(sh.height() + topMargin + bottomMargin);
	return sh;
}

void YaChatContactStatusExtra::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	// p.fillRect(rect(), Qt::red);

	BorderRenderer* border = BorderRenderer::rendererFor(":images/balloon/mood");
	border->setBorderRectCalculator(new YaSelfMoodBorderRectCalculator(border));
	border->draw(&p, rect());
}

void YaChatContactStatusExtra::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);
	QRect r(rect());
	r.setTopLeft(r.topLeft() + QPoint(leftMargin, topMargin));
	r.setBottomRight(r.bottomRight() - QPoint(rightMargin, bottomMargin));

	if (icon_->isVisible()) {
		QRect iconRect(r);
		iconRect.setWidth(icon_->width());
		iconRect.setHeight(icon_->height());
		iconRect.moveTop(iconRect.top() + (r.height() - iconRect.height()) / 2);
		icon_->setGeometry(iconRect);
		r.setLeft(iconRect.right() + 0);
	}
	label_->setGeometry(r);
}

//----------------------------------------------------------------------------
// YaChatContactStatus
//----------------------------------------------------------------------------

YaChatContactStatus::YaChatContactStatus(QWidget *parent)
	: OverlayWidget<QFrame, YaChatContactStatusExtra>(parent,
		new YaChatContactStatusExtra(parent->parentWidget()))
{
}

QRect YaChatContactStatus::extraGeometry() const
{
	extra()->setVisible(extraShouldBeVisible());

	QSize sh = extra()->sizeHint();
	QRect result(globalRect().x() - 18,
	             globalRect().y() - sh.height() - 1,
	             sh.width(),
	             qMax(sh.height(), height()));

	if (result.right() > globalRect().right()) {
		result.setRight(globalRect().right());
	}

	return result;
}

bool YaChatContactStatus::extraShouldBeVisible() const
{
	return extra()->shouldBeVisible();
}

QSize YaChatContactStatus::sizeHint() const
{
	return minimumSizeHint();
}

QSize YaChatContactStatus::minimumSizeHint() const
{
	return QSize(1, 1);
}

void YaChatContactStatus::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	// p.fillRect(rect(), Qt::green);
}

#include "yachatcontactstatus.moc"
