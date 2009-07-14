/*
 * yarostertiplabel.h - widget that shows contact tooltips
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

#ifndef YAROSTERTIPLABEL_H
#define YAROSTERTIPLABEL_H

#include "psitiplabel.h"

#include "ui_yarostertiplabel.h"

class PsiContact;
class ToolTipPosition;
class LastActivityTask;
class QMimeData;
class QMenu;

namespace XMPP {
	class VCard;
	class JT_VCard;
};

using namespace XMPP;

class YaRosterTipLabelLeaveTimer : public QObject
{
	Q_OBJECT
public:
	YaRosterTipLabelLeaveTimer(QWidget* parent);

	QWidget* listView() const;
	void setListView(QWidget* listView);

	// reimplemented
	bool processEvent(QObject* obj, QEvent* e, bool* result);

	bool isEnabledWithoutConsideringPermanentlyDisabledState() const;
	bool isEnabled() const;
	void setEnabled(bool enabled);

	bool permanentlyDisabled() const;
	void setPermanentlyDisabled(bool permanentlyDisabled);

signals:
	void hideTip();

protected:
	// reimplemented
	void timerEvent(QTimerEvent* e);

private:
	QWidget* parent_;
	QWidget* listView_;
	QBasicTimer leaveTimer_;
	bool enabled_;
	bool permanentlyDisabled_;

	void startLeaveTimer();
	void stopLeaveTimer();
};

class YaRosterTipLabel : public PsiTipLabel
{
	Q_OBJECT
public:
	YaRosterTipLabel(QWidget* parent, bool isHelperTipLabel = false);
	~YaRosterTipLabel();

	void setTipPosition(ToolTipPosition* tipPosition);
	void setContact(PsiContact* contact);
	void setContactSelection(QMimeData* contactSelection);
	void setListView(QWidget* listView);
	bool leaveTimerPermanentlyDisabled() const;
	void setLeaveTimerPermanentlyDisabled(bool permanentlyDisabled);

	// reimplemented
	QSize sizeHint() const;
	bool eventFilter(QObject *, QEvent *);
	void setText(const QString& text);

signals:
	void removeContact(PsiContact* contact, QMimeData* contactSelection);
	void renameContact(PsiContact* contact, QMimeData* contactSelection);
	void blockContact(PsiContact* contact, QMimeData* contactSelection);
	void unblockContact(PsiContact* contact, QMimeData* contactSelection);
	void addContact(const XMPP::Jid& jid, PsiAccount*, const QStringList& groups, const QString& desiredName);
	void toolTipEntered(PsiContact* contact, QMimeData* contactSelection);
	void toolTipHidden(PsiContact* contact, QMimeData* contactSelection);

protected:
	// reimplemented
	void enterEvent(QEvent*);
	void paintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent*);
	void showEvent(QShowEvent* event);

	// used in YaChatTipLabel
	QWidget* widget() const { return leaveTimer_->listView(); }

public slots:
	void updateSize(bool force = false);

private slots:
	void openChat();
	void openHistory();
	void openYaProfile();
	void openPhotos();
	void composeEmail();
	void addContact();
	void deleteContact();
	void renameContact();
	void authContact();
	void blockContact();
	void copyJid();

	void vcardFinished();
	void lastAvailableFinished();

	void applyNewGeometry();

	void updateContact(const Jid& jid);
	void updateActionButtons();

protected:
	Ui::YaRosterTipLabel ui_;

	virtual void dataInvalidated();
	bool useMaleMsg() const;

	// reimplemented
	void initUi();

protected slots:
	// reimplemented
	virtual void hideTip();

	void closeClicked();
	void hideMenuAndTip();

private:
	ToolTipPosition* tipPosition_;
	PsiContact* contact_;
	QMimeData* contactSelection_;
	YaProfile* profile_;
	bool initialized_;
	QPointer<LastActivityTask> lastActivityTask_;
	QPointer<XMPP::JT_VCard> vcardTask_;
	QRect newGeometry_;
	QTimer* deleteTimer_;
	QTimer* updateGeometryTimer_;
	QPointer<YaRosterTipLabelLeaveTimer> leaveTimer_;
	QString statusText_;
	QString fullStatusTextHtml_;
	QString fullStatusTextHtmlSansStatus_;
	QAction* copyJidAction_;
	QAction* composeEmailAction_;
	QMenu* jidMenu_;
	bool statusTextInvisible_;
	bool statusTextEscape_;

	bool selfContact() const;
	void invalidateData();
	QFont labelFont() const;
	void updateFont();
	void setVCard(XMPP::VCard vcard);
	void setStatusText(QString text, bool invisible = false, bool escape = true);
	void setLastAvailable(const QDateTime& dateTime, bool invisible);

	QColor backgroundColor() const;
	QString statusTypeDescription(XMPP::Status::Type type) const;

	int firstButtonTop() const;

	// reimplemented
	void startHideTimer();
};

#endif
