/*
 * yaselfavatarlabel.h - self avatar widget
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

#ifndef YASELFAVATARLABEL_H
#define YASELFAVATARLABEL_H

#include "yaavatarlabel.h"

class PsiAccount;
class PsiContactList;
class YaSelfAvatarLabelMenu;
class PsiContact;

#include "busywidget.h"

class YaSelfAvatarLabel : public YaAvatarLabel
{
	Q_OBJECT
public:
	YaSelfAvatarLabel(QWidget* parent);
	virtual ~YaSelfAvatarLabel();

	enum Mode {
		SelectAvatar = 0,
		OpenProfile
	};

	Mode mode() const;
	void setMode(Mode mode);

	PsiContact* selfContact() const;
	void setSelfContact(PsiContact* contact);

	class AvatarData {
	public:
		AvatarData();
		AvatarData(QString fileName);

		void initFromData(const QByteArray& data);

		bool isNull() const;

		QByteArray data() const;

		const QImage& image() const;
		const QString& hash() const;

	private:
		QImage image_;
		QByteArray data_;
		QString hash_;
	};

protected:
	// reimplemented
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void paintAvatar(QPainter* painter);
	virtual XMPP::VCard::Gender gender() const;
	bool eventFilter(QObject* obj, QEvent* e);
	void contextMenuEvent(QContextMenuEvent* e);

private slots:
	void avatarSelected(YaSelfAvatarLabel::AvatarData);
	void choosePixmap();
	void accountCountChanged();
	void accountActivityChanged();
	void setVCardFinished();

public:
	virtual void setContactList(const PsiContactList* contactList);
	virtual const PsiContactList* contactList() const;
	QList<PsiAccount*> accounts() const;

protected:
	AvatarData avatarData() const;
	QPixmap avatarPixmap() const;

	// reimplemented abstracts
	virtual QIcon avatar() const;

private:
	Mode mode_;
	YaSelfAvatarLabelMenu* menu_;
	QPointer<PsiContactList> contactList_;
	int uploadingVCardsCount_;
	BusyWidget* busyWidget_;
	PsiContact* selfContact_;

	bool isConnected() const;
	void updateBusyWidget();
};

#endif
