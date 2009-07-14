/*
 * yainformer.cpp
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

#include "yainformer.h"

#include <QVBoxLayout>
#include <QLabel>

#include "yainformerbutton.h"
#include "contactlistmodel.h"
#include "xmpp_status.h"
#include "psievent.h"
#include "yaeventnotifierframe.h"
#include "shortcutmanager.h"

//----------------------------------------------------------------------------
// DummyShortcutWidget - this class exists for the sole purpose of activating
// shortcut actions when status bar is hidden
//----------------------------------------------------------------------------

class DummyShortcutWidget : public QWidget
{
	Q_OBJECT
public:
	DummyShortcutWidget(const QString& shortcut, QWidget* button)
	{
		ShortcutManager::connect(shortcut, this, SIGNAL(activated()));
		connect(this, SIGNAL(activated()), button, SLOT(click()));
	}

signals:
	void activated();
};

//----------------------------------------------------------------------------
// YaStatusEvent
//----------------------------------------------------------------------------

class YaStatusEvent : public PsiEvent
{
	Q_OBJECT
public:
	YaStatusEvent(const QModelIndex& index)
		: PsiEvent(static_cast<const ContactListModel*>(index.model())->account(index))
	{
		jid_ = index.data(ContactListModel::JidRole).toString();
		description_ = index.data(ContactListModel::StatusTextRole).toString();
	}
	~YaStatusEvent() { }

	XMPP::Jid jid() const;
	void setJid(const XMPP::Jid &);

	// reimplemented
	int type() const { return Message; }
	XMPP::Jid from() const { return jid_; }
	void setFrom(const XMPP::Jid &) { }
	virtual QString description() const { return description_; }

private:
	XMPP::Jid jid_;
	QString description_;
};

//----------------------------------------------------------------------------
// YaInformer
//----------------------------------------------------------------------------

YaInformer::YaInformer(const QModelIndex& index, QObject* parent)
	: QObject(parent)
	, model_(0)
	, button_(0)
	, shortcutWidget_(0)
{
	model_ = index.model();
	index_ = index;
	jid_ = index_.data(ContactListModel::JidRole).toString();

	if (model_) {
		connect(model_, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
		        this, SLOT(dataChanged(QModelIndex, QModelIndex)));
		connect(model_, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
		        this, SLOT(invalidateIndex()));
		connect(model_, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
		        this, SLOT(invalidateIndex()));
		connect(model_, SIGNAL(modelAboutToBeReset()),
		        this, SLOT(invalidateIndex()));
	}

	widget_ = new QWidget();
	QVBoxLayout* vbox = new QVBoxLayout(widget_);
	vbox->setMargin(0);
	vbox->setSpacing(0);
	eventNotifierFrame_ = new YaEventNotifierFrame(widget_);
	eventNotifierFrame_->setTosterMode(true);
	connect(eventNotifierFrame_, SIGNAL(closeClicked()), SIGNAL(deactivated()));
	vbox->addWidget(eventNotifierFrame_);

	button_ = new YaInformerButton(this);
	connect(button_, SIGNAL(clicked(bool)), SLOT(informerButtonClicked(bool)));

	dataChanged(index, index);
}

YaInformer::~YaInformer()
{
	if (!widget_.isNull())
		delete widget_;
	if (!button_.isNull())
		delete button_;
}

void YaInformer::invalidateIndex()
{
	index_ = QModelIndex();
}

bool YaInformer::handlesIndex(const QModelIndex& index) const
{
	return index.data(ContactListModel::JidRole).toString() == jid();
}

void YaInformer::ensureIndex() const
{
	if (index_.isValid() || !model_)
		return;

	for (int row = 0; row < model_->rowCount(QModelIndex()); ++row) {
		QModelIndex index = model_->index(row, 0, QModelIndex());
		if (handlesIndex(index)) {
			index_ = index;
			break;
		}
	}
}

bool YaInformer::isValid() const
{
	ensureIndex();
	return index_.isValid();
}

void YaInformer::updateEventNotifierFrame()
{
	ensureIndex();
	YaStatusEvent* event = new YaStatusEvent(index_);
	eventNotifierFrame_->setEvent(-1, event);
	delete event;
}

void YaInformer::updateButton()
{
	button_->dataChanged();
}

void YaInformer::dataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
	ensureIndex();
	if (topLeft.parent() != index_.parent())
		return;

	if (index_.row() >= topLeft.row() && index_.row() <= bottomRight.row()) {
		updateEventNotifierFrame();
		updateButton();
	}
}

YaInformer* YaInformer::create(const QModelIndex& index, QObject* parent)
{
	return new YaInformer(index, parent);
}

YaInformerButton* YaInformer::button() const
{
	return button_;
}

QWidget* YaInformer::widget()
{
	return widget_;
}

QWidget* YaInformer::shortcutWidget() const
{
	return shortcutWidget_;
}

bool YaInformer::isPersistent() const
{
	return jid() == "mail.ya.ru";
}

QBrush YaInformer::backgroundBrush() const
{
	return eventNotifierFrame_->backgroundBrush();
}

QString YaInformer::jid() const
{
	return jid_;
}

QIcon YaInformer::icon() const
{
	ensureIndex();
	QIcon result;
	QVariant iconData = index_.data(ContactListModel::PictureRole);
	if (iconData.isValid()) {
		if (iconData.type() == QVariant::Icon) {
			result = qvariant_cast<QIcon>(iconData);
		}
	}

	return result;
}

bool YaInformer::isOnline() const
{
	ensureIndex();
	return static_cast<XMPP::Status::Type>(index_.data(ContactListModel::StatusTypeRole).toInt()) != XMPP::Status::Offline;
}

QString YaInformer::value() const
{
	ensureIndex();
	return "";
}

void YaInformer::informerButtonClicked(bool checked)
{
	if (checked)
		emit activated();
	else
		emit deactivated();
}

void YaInformer::setActivateShortcut(const QString& shortcut)
{
	shortcutWidget_ = new DummyShortcutWidget(shortcut, button_);
}

void YaInformer::toggle()
{
	setVisible(!isVisible());
}

bool YaInformer::isVisible() const
{
	return button_->isChecked();
}

void YaInformer::setVisible(bool visible)
{
	if (visible)
		emit activated();
	else
		emit deactivated();
}

void YaInformer::hide()
{
	setVisible(false);
}

int YaInformer::desiredHeight() const
{
	return 150;
}

#include "yainformer.moc"
