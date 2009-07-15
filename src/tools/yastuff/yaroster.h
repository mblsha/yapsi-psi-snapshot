/*
 * yaroster.h - widget that handles contact-list
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

#ifndef YAROSTER_H
#define YAROSTER_H

#include <QFrame>
#include <QPointer>
#include <QToolButton>
#include <QPair>

class QStackedWidget;
class PsiContactList;
class ContactListModel;
class YaToolBox;
class YaContactListView;
class YaFilteredContactListView;
class YaToolBoxPageButton;
class YaRosterTabButton;
class QLineEdit;
class QAbstractItemModel;
class QSortFilterProxyModel;
class YaLoginPage;
class AddContactListView;
class YaRosterContactsTabButton;
class PsiAccount;
class PsiContact;
class QMimeData;
class AnimatedStackedWidget;
class YaContactListModel;
class YaCommonNoContactsPage;
class YaErrorPage;
class YaConnectingPage;
class YaAddContactPage;
class YaContactCounter;
class YaCommonNoContactsPage;

#include "xmpp_jid.h"
#include "yacontactlistview.h"

class YaRoster;

class YaRosterToolButton : public QToolButton
{
public:
	YaRosterToolButton(QWidget* parent);

	void setUnderMouse(bool underMouse);
	void setCompactSize(const QSize& size);

	// reimplemented
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	virtual bool pressable() const;

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void changeEvent(QEvent* e);

	virtual QIcon::Mode iconMode() const;
	virtual bool textVisible() const;

private:
	bool underMouse_;
	QSize compactSize_;
	QString toolTip_;
};

class YaRosterAddContactToolButton : public YaRosterToolButton
{
public:
	YaRosterAddContactToolButton(QWidget* parent, YaRoster* yaRoster);

	// reimplemented
	virtual bool pressable() const;

protected:
	// reimplemented
	virtual QIcon::Mode iconMode() const;

private:
	QPointer<YaRoster> yaRoster_;
};

class YaRosterTab : public QObject
{
	Q_OBJECT
public:
	YaRosterTab(const QString& name, bool createToolButtons, YaRoster* parent);
	~YaRosterTab();

	virtual void init();

	virtual QList<QWidget*> proxyableWidgets() const;

	YaToolBoxPageButton* pageButton() const;
	QWidget* widget() const;
	YaRoster* yaRoster() const;

	virtual void setModel(QAbstractItemModel* model);
	virtual void setModelForFilter(QAbstractItemModel* model);
	virtual void setAvatarMode(YaContactListView::AvatarMode avatarMode);

	YaContactListView* contactListView() const;
	QStackedWidget* contactListViewStackedWidget() const;

signals:
	void addContact();

protected:
	QStackedWidget* stackedWidget() const;
	PsiContactList* contactList() const;

private slots:
	void updateContactListViewStackedWidget();
	void showOfflineContacts();
	void reconnect();
	void login();

private:
	QPointer<YaRosterTabButton> pageButton_;
	QPointer<QStackedWidget> stackedWidget_;
	QPointer<QStackedWidget> contactListViewStackedWidget_;
	QPointer<YaCommonNoContactsPage> noContactsPage_;
	QPointer<YaErrorPage> errorPage_;
	QPointer<YaConnectingPage> connectingPage_;
	QPointer<YaCommonNoContactsPage> offlinePage_;
	QPointer<YaCommonNoContactsPage> noVisibleContactsPage_;
	YaContactListView* contactListView_;
	YaContactCounter* contactCounter_;
};

class YaRosterContactsTab : public YaRosterTab
{
	Q_OBJECT
public:
	YaRosterContactsTab(const QString& name, bool createToolButtons, YaRoster* parent);

	YaAddContactPage* addContactListView() const;
	bool filterModeEnabled() const;

	// reimplemented
	void init();
	void setModel(QAbstractItemModel* model);
	void setModelForFilter(QAbstractItemModel* model);
	void setAvatarMode(YaContactListView::AvatarMode avatarMode);
	YaRosterContactsTabButton* contactsPageButton() const;
	QList<QWidget*> proxyableWidgets() const;

	YaFilteredContactListView* filteredListView() const;

signals:
	void filterModeChanged(bool enabled);

private slots:
	void watchForJid();
	void vcardFinished();

	void okButtonClicked();
	void cancelButtonClicked();

public slots:
	void addContact(const XMPP::Jid& jid, PsiAccount* account, const QStringList& groups, const QString& desiredName);
	void addButtonClicked();
	void setAddContactModeEnabled(bool enabled);
	void addContactTextEntered(const QString& text);

	void filterEditTextChanged(const QString&);
	void quitFilteringMode();

	void updateFilterMode();

	void setFilterModeEnabled(bool enabled);
	void addContactCancelled();

protected:
	bool eventFilter(QObject* obj, QEvent* e);

private:
	bool filterModeEnabled_;
	YaAddContactPage* addContactListView_;
	YaFilteredContactListView* filteredListView_;
	QSortFilterProxyModel* proxyModel_;
	QTimer* watchTimer_;
	QPointer<PsiAccount> watchedAccount_;
	XMPP::Jid watchedJid_;

	QPair<PsiAccount*, XMPP::Jid> addContactHelper(const QString& text, PsiAccount* account, const QStringList& groups, const QString& desiredName);
	void contactAddedHelper(PsiAccount* account, const XMPP::Jid& jid);
};

class YaRoster : public QFrame
{
	Q_OBJECT
public:
	YaRoster(QWidget* parent);
	~YaRoster();

	QList<QWidget*> proxyableWidgets() const;

	void setContactList(PsiContactList* contactList);
	void setContactListViewportMenu(QMenu* menu);

	bool selfWidgetsShouldBeVisible();

	void doSetFocus();

	bool haveAvailableAccounts() const;
	PsiContactList* contactList() const;
	YaContactListModel* contactListModel() const;

	void bringToFront(QWidget* widget) const;

signals:
	void updateSelfWidgetsVisibility();
	void enableAddGroupAction(bool);
	void availableAccountsChanged(bool);
	void accountActivityChanged();

public slots:
	void setAvatarMode(YaContactListView::AvatarMode avatarMode);
	void toggleFilterContacts();
	void addGroup();
	void addContact();

private slots:
	void updateCurrentPage();
	void optionChanged(const QString& option);
	void removeContact(PsiContact* contact, QMimeData* selection);
	void renameContact(PsiContact* contact, QMimeData* selection);
	void blockContact(PsiContact* contact, QMimeData* contactSelection);
	void unblockContact(PsiContact* contact, QMimeData* contactSelection);
	void removeContactConfirmation(const QString& id, bool confirmed);
	void removeContactFullyConfirmation(const QString& id, bool confirmed);
	void blockContactConfirmationHelper(const QString& id, bool confirmed, bool doBlock);
	void blockContactConfirmation(const QString& id, bool confirmed);
	void unblockContactConfirmation(const QString& id, bool confirmed);
	void removeContactConfirmation(QMimeData* contactSelection);
	void removeSelection(QMimeData* selection);
	void addSelection(QMimeData* selection);
	void accountCountChanged();
	void scrollToContact(PsiContact* contact);

	void showAgentsChanged(bool);
	void showHiddenChanged(bool);
	void showSelfChanged(bool);
	bool showOfflineChanged(bool);

protected:
	// reimplemented
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);
	bool eventFilter(QObject* obj, QEvent* e);
	void contextMenuEvent(QContextMenuEvent* e);

	YaContactListModel* modelForDeleteOperations() const;
	QList<YaRosterTab*> rosterTabs() const;

private:
	YaContactListView::AvatarMode avatarMode_;
	QStackedWidget* stackedWidget_;
	YaToolBox* rosterPage_;
	YaLoginPage* loginPage_;

	YaRosterContactsTab* contacts_;

	ContactListModel* filterModel_;
	ContactListModel* informersModel_;
	ContactListModel* friendListModel_;
	ContactListModel* contactListModel_;

	QPointer<PsiContactList> contactList_;
	bool haveAvailableAccounts_;
};

#endif
