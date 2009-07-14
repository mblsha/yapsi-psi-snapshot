/*
 * yaremoveconfirmationmessagebox.cpp - generic confirmation of destructive action
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

#include "yaremoveconfirmationmessagebox.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>

#include "yastyle.h"

#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#include "yapreferences.h"
#endif

//----------------------------------------------------------------------------
// YaRemoveConfirmationMessageBoxManager
//----------------------------------------------------------------------------

YaRemoveConfirmationMessageBoxManager* YaRemoveConfirmationMessageBoxManager::instance_ = 0;
int YaRemoveConfirmationMessageBoxManager::onlineId_ = 0;

YaRemoveConfirmationMessageBoxManager* YaRemoveConfirmationMessageBoxManager::instance()
{
	if (!instance_) {
		instance_ = new YaRemoveConfirmationMessageBoxManager();
	}
	return instance_;
}

void YaRemoveConfirmationMessageBoxManager::processData(const QString& id, const QList<DataCallback> callbacks, const QString& title, const QString& informativeText, QWidget* parent, const QStringList& actionNames)
{
	Data data;
	data.onlineId = ++onlineId_;
	data.id = id;
	data.callbacks = callbacks;
	data.title = title;
	data.informativeText = informativeText;
	data.buttons = actionNames;
	data.parent = parent;

	// FIXME: duplicates mustn't be allowed
	foreach(Data d, data_) {
		Q_ASSERT(d.id != id);
		if (d.id == id)
			return;
	}

	data_ << data;
#ifndef YAPSI_ACTIVEX_SERVER
	QTimer::singleShot(0, this, SLOT(update()));
#else
	QString parentString = "0";
	if (parent) {
		parentString = QString::number((int)parent->window()->winId());

		if (dynamic_cast<YaPreferences*>(parent->window())) {
			parentString = "settings";
		}
	}

	YaOnlineHelper::instance()->messageBox(parentString,
	                                       QString::number(data.onlineId),
	                                       YaRemoveConfirmationMessageBox::tr("Ya.Online"),
	                                       YaRemoveConfirmationMessageBox::processInformativeText(data.informativeText),
	                                       data.buttons,
	                                       QMessageBox::Warning);
#endif
}

void YaRemoveConfirmationMessageBoxManager::removeConfirmation(const QString& id, QObject* obj, const char* slot, const QString& title, const QString& informativeText, QWidget* parent, const QString& destructiveActionName)
{
	QStringList buttons;
	if (!destructiveActionName.isEmpty())
		buttons << destructiveActionName;
	else
		buttons << YaRemoveConfirmationMessageBox::tr("Delete");
	buttons << YaRemoveConfirmationMessageBox::tr("Cancel");

	QList<DataCallback> callbacks;
	callbacks << DataCallback(obj, slot);

	processData(id, callbacks, title, informativeText, parent, buttons);
}

void YaRemoveConfirmationMessageBoxManager::removeConfirmation(const QString& id, QObject* obj1, const char* action1slot, QObject* obj2, const char* action2slot, const QString& title, const QString& informativeText, QWidget* parent, const QString& action1name, const QString& action2name)
{
	QStringList buttons;
	buttons << action1name;
	buttons << action2name;
	buttons << YaRemoveConfirmationMessageBox::tr("Cancel");

	QList<DataCallback> callbacks;
	callbacks << DataCallback(obj1, action1slot);
	callbacks << DataCallback(obj2, action2slot);

	processData(id, callbacks, title, informativeText, parent, buttons);
}

void YaRemoveConfirmationMessageBoxManager::update()
{
#ifndef YAPSI_ACTIVEX_SERVER
	while (!data_.isEmpty()) {
		Data data = data_.takeFirst();

		Q_ASSERT(data.buttons.count() >= 2 && data.buttons.count() <= 3);
		YaRemoveConfirmationMessageBox msgBox(data.title, data.informativeText, data.parent);

		QStringList buttons = data.buttons;
		buttons.takeLast(); // Cancel
		Q_ASSERT(!buttons.isEmpty());
		msgBox.setDestructiveActionName(buttons.takeFirst());
		if (!buttons.isEmpty()) {
			msgBox.setComplimentaryActionName(buttons.takeFirst());
		}

		msgBox.doExec();

		QList<bool> callbackData;
		for (int i = 0; i < data.callbacks.count(); ++i) {
			if (i == 0)
				callbackData << msgBox.removeAction();
			else if (i == 1)
				callbackData << msgBox.complimentaryAction();
			else
				callbackData << false;
		}

		for (int i = 0; i < data.callbacks.count(); ++i) {
			QMetaObject::invokeMethod(data.callbacks[i].obj, data.callbacks[i].slot, Qt::DirectConnection,
			                           QGenericReturnArgument(),
			                           Q_ARG(QString, data.id),
			                           Q_ARG(bool, callbackData[i]));
		}
	}
#endif
}

#ifdef YAPSI_ACTIVEX_SERVER
void YaRemoveConfirmationMessageBoxManager::onlineCallback(const QString& id, int button)
{
	if (id.isEmpty())
		return;
	int intId = id.toInt();
	Q_ASSERT(intId > 0);
	if (intId <= 0)
		return;

	for (int i = 0; i < data_.count(); ++i) {
		if (data_[i].onlineId == intId) {
			Data data = data_[i];

			for (int j = 0; j < data.callbacks.count(); ++j) {
				QMetaObject::invokeMethod(data.callbacks[j].obj, data.callbacks[j].slot, Qt::DirectConnection,
				                           QGenericReturnArgument(),
				                           Q_ARG(QString, data.id),
				                           Q_ARG(bool, (button - 1) == j));
			}
			data_.removeAt(i);
			break;
		}
	}
}
#endif

YaRemoveConfirmationMessageBoxManager::YaRemoveConfirmationMessageBoxManager()
	: QObject(QCoreApplication::instance())
{
#ifdef YAPSI_ACTIVEX_SERVER
	connect(YaOnlineHelper::instance(), SIGNAL(messageBoxClosed(const QString&, int)), this, SLOT(onlineCallback(const QString&, int)));
#endif
}

YaRemoveConfirmationMessageBoxManager::~YaRemoveConfirmationMessageBoxManager()
{
}

//----------------------------------------------------------------------------
// YaRemoveConfirmationMessageBox
//----------------------------------------------------------------------------

#ifdef Q_WS_WIN
// #define USE_WINDOWS_NATIVE_MSGBOX
#endif

#ifdef USE_WINDOWS_NATIVE_MSGBOX
#include <windows.h>

static QString okButtonCaption;
static QString cancelButtonCaption;
static bool activated;

// hook technique from http://www.catch22.net/tuts/msgbox.asp
static HHOOK hMsgBoxHook;

static int textWidth(HWND button, const QString& text)
{
	RECT textRect;
	textRect.left = textRect.top = textRect.right = textRect.bottom = 0;
	DrawText(GetDC(button), text.utf16(), text.length(), &textRect, DT_CALCRECT);
	return textRect.right - textRect.left;
}

LRESULT CALLBACK CBTProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_ACTIVATE) {
		if (activated)
			return 0;
		activated = true;
		HWND hChildWnd = (HWND)wParam;

		HWND okButton = GetDlgItem(hChildWnd, IDOK);
		HWND cancelButton = GetDlgItem(hChildWnd, IDCANCEL);
		Q_ASSERT(okButton);
		Q_ASSERT(cancelButton);

		SetDlgItemText(hChildWnd, IDOK, okButtonCaption.utf16());
		SetDlgItemText(hChildWnd, IDCANCEL, cancelButtonCaption.utf16());

		RECT parentRect, okRect, cancelRect;
		GetWindowRect(GetParent(okButton), &parentRect);
		GetWindowRect(okButton, &okRect);
		GetWindowRect(cancelButton, &cancelRect);

		int parentWidth = parentRect.right - parentRect.left;

		int minMargin = 14;
		int spacing = cancelRect.left - okRect.right;

		int buttonWidth = qMax(::textWidth(okButton, okButtonCaption), ::textWidth(cancelButton, cancelButtonCaption));
		int maxButtonWidth = (parentWidth - minMargin*2 - spacing) / 2;
		buttonWidth = qMin(buttonWidth, maxButtonWidth);

		WINDOWPLACEMENT wp;
		GetWindowPlacement(okButton, &wp);
		wp.rcNormalPosition.left  = (parentWidth / 2) - (spacing / 2) - buttonWidth - 3;
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + buttonWidth;
		SetWindowPlacement(okButton, &wp);

		GetWindowPlacement(cancelButton, &wp);
		wp.rcNormalPosition.left  = (parentWidth / 2) + (spacing / 2) - 3;
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + buttonWidth;
		SetWindowPlacement(cancelButton, &wp);
	}
	else {
		CallNextHookEx(hMsgBoxHook, nCode, wParam, lParam);
	}
	return 0;
}

int MsgBoxEx(HWND hwnd, LPCTSTR szText, LPCTSTR szCaption, UINT uType)
{
	int retval;

	activated = false;
	hMsgBoxHook = SetWindowsHookEx(
	                  WH_CBT,
	                  CBTProc,
	                  NULL,
	                  GetCurrentThreadId()
	              );

	retval = MessageBox(hwnd, szText, szCaption, uType);

	UnhookWindowsHookEx(hMsgBoxHook);

	return retval;
}
#endif

YaRemoveConfirmationMessageBox::YaRemoveConfirmationMessageBox(const QString& title, const QString& informativeText, QWidget* parent)
	: QMessageBox()
	, removeButton_(0)
	, complimentaryButton_(0)
	, cancelButton_(0)
{
	setStyle(YaStyle::defaultStyle());

	setWindowTitle(tr("Ya.Online"));
	setText(title);
	setInformativeText(informativeText);

	setIcon(QMessageBox::Warning);
	int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
	QIcon tmpIcon= style()->standardIcon(QStyle::SP_MessageBoxWarning);
	if (!tmpIcon.isNull())
		setIconPixmap(tmpIcon.pixmap(iconSize, iconSize));

	// doesn't work with borderless top-level windows on Mac OS X
	// QWidget* window = parent->window();
	// msgBox.setParent(window);
	// msgBox.setWindowFlags(Qt::Sheet);
	Q_UNUSED(parent);
}

void YaRemoveConfirmationMessageBox::setDestructiveActionName(const QString& destructiveAction)
{
	Q_ASSERT(!removeButton_);
	Q_ASSERT(!cancelButton_);
	removeButton_ = addButton(destructiveAction, QMessageBox::AcceptRole /*QMessageBox::DestructiveRole*/);
	cancelButton_ = addButton(QMessageBox::Cancel);
	setDefaultButton(removeButton_);
}

void YaRemoveConfirmationMessageBox::setComplimentaryActionName(const QString& complimentaryAction)
{
	Q_ASSERT(removeButton_);
	Q_ASSERT(cancelButton_);
	Q_ASSERT(!complimentaryButton_);
	complimentaryButton_ = addButton(complimentaryAction, QMessageBox::AcceptRole);
}

QString YaRemoveConfirmationMessageBox::processInformativeText(const QString& informativeText)
{
	QString text = informativeText;
	text.replace("<br>", "\n");
	QRegExp rx("<.+>");
	rx.setMinimal(true);
	text.replace(rx, "");
	return text;
}

void YaRemoveConfirmationMessageBox::doExec()
{
	if (!removeButton_) {
		setDestructiveActionName(tr("Delete"));
	}

	setText(processInformativeText(informativeText()));
	setInformativeText(QString());

#ifdef USE_WINDOWS_NATIVE_MSGBOX
	okButtonCaption = removeButton_->text();
	cancelButtonCaption = cancelButton_->text();

	int msgboxID = MsgBoxEx(
	                   NULL,
	                   text.utf16(),
	                   windowTitle().utf16(),
	                   MB_ICONWARNING | MB_OKCANCEL | MB_APPLMODAL
	               );

	return msgboxID == IDOK;
#else
	Q_ASSERT(removeButton_);
	Q_ASSERT(cancelButton_);
	YaStyle::makeMeNativeLooking(this);
	exec();
#endif
}

bool YaRemoveConfirmationMessageBox::removeAction() const
{
	return clickedButton() == removeButton_;
}

bool YaRemoveConfirmationMessageBox::complimentaryAction() const
{
	return clickedButton() == complimentaryButton_;
}
