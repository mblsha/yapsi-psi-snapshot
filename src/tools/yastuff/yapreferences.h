/*
 * yapreferences.h - preferences window
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

#ifndef YAPREFERENCES_H
#define YAPREFERENCES_H

#include "yawindow.h"
#include "advwidget.h"

#include <QMap>

#include "ui_yapreferences.h"
#include "yawindowtheme.h"

class PsiCon;
class PsiAccount;
class PsiContact;
class QPushButton;
class YaManageAccounts;

class YaPreferences : public YaWindow
{
	Q_OBJECT
public:
	YaPreferences();
	~YaPreferences();

	void setController(PsiCon* controller);

	void restore();
	void save();

	void toggle();
	void forceVisible();

	void activate();
	bool shouldBeVisible();

	QString getPreferencesXml() const;

	// reimplemented
	virtual const YaWindowTheme& theme() const;

protected:
	// reimplemented
	bool eventFilter(QObject *, QEvent *);
	void keyPressEvent(QKeyEvent* event);

public slots:
	void openPreferences();
	void openAccounts();

private slots:
	void clearMessageHistory();
	void clearMessageHistoryConfirmation(const QString& id, bool confirmed);
	void changeChatFontFamily(const QFont &);
	void changeChatFontSize(const QString &);
	void getChatPreferences();
	void showLocalHistory();

protected slots:
	void accept();
	void smthSet();

private:
	PsiCon* controller_;
	Ui::YaPreferences ui_;
	QFont chatFont_;
	YaWindowTheme theme_;
	YaManageAccounts* accountsPage_;
	QMap<QString, QWidget*> preferenceKeyMapping_;

	enum ClearMessageHistoryType {
		ClearHistory_Local_All = 0,
		ClearHistory_Local,
		ClearHistory_Remote
	};

	void clearMessageHistory(YaPreferences::ClearMessageHistoryType type, PsiAccount* account);

	enum Page {
		Page_Preferences = 0,
		Page_Accounts
	};

	enum {
		PreferenceMappingData = Qt::UserRole + 1
	};

	void setCurrentPage(Page page);
	void setChangedConnectionsEnabled(bool changedConnectionsEnabled);
	void convertHistory(PsiContact* contact);

private slots:
	void applyPreferences(const QString& xml);
	void applyImmediatePreferences(const QString& xml);
	void confirmationDelete(YaPreferences::ClearMessageHistoryType type, PsiAccount* account);
	void edb_finished();
};

#endif
