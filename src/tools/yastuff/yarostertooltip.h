/*
 * yarostertooltip.h - per-contact tooltips
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

#ifndef YAROSTERTOOLTIP_H
#define YAROSTERTOOLTIP_H

#include "psitooltip.h"

class PsiContact;
class YaRosterTipLabel;
class QMimeData;
class YaChatToolTip;
class PsiAccount;

#include "xmpp_jid.h"

class YaRosterToolTipPosition : public ToolTipPosition
{
public:
	YaRosterToolTipPosition(QRect rect, const QWidget* parentWidget)
		: ToolTipPosition(rect.topLeft(), parentWidget)
		, globalRect(rect)
	{}

	virtual QPoint calculateTipPosition(const QWidget* label, const QSize& size) const;

protected:
	QRect globalRect;

	virtual QPoint initialPoint(const QWidget* label) const;
};

class YaRosterToolTip : public PsiToolTip
{
	Q_OBJECT
public:
	QRect currentRect() const;

	void hide();
	void showText(QRect rect, PsiContact* contact, const QWidget* w, QMimeData* contactSelection);
	static YaRosterToolTip* instance();

	// reimplemented
	virtual bool isVisible() const;

	static QString forceTipLabelUpdateString();

signals:
	void removeContact(PsiContact* contact, QMimeData* contactSelection);
	void renameContact(PsiContact* contact, QMimeData* contactSelection);
	void blockContact(PsiContact* contact, QMimeData* contactSelection);
	void unblockContact(PsiContact* contact, QMimeData* contactSelection);
	void addContact(const XMPP::Jid& jid, PsiAccount* account, const QStringList& groups, const QString& desiredName);
	void toolTipEntered(PsiContact* contact, QMimeData* contactSelection);
	void toolTipHidden(PsiContact* contact, QMimeData* contactSelection);

protected:
	YaRosterToolTip();

	static YaRosterToolTip* instance_;
	QRect lastRect_;
	PsiContact* lastContact_;
	QWidget* lastWidget_;
	QMimeData* contactSelection_;

	void registerTipLabel(YaRosterTipLabel* tipLabel);

protected:
	// reimplemented
	virtual ToolTipPosition* createTipPosition(const QPoint& cursorPos, const QWidget* parentWidget);
	virtual PsiTipLabel* createTipLabel(const QString& text, QWidget* parent);
	virtual bool moveAndUpdateTipLabel(PsiTipLabel* label, const QString& text);
	virtual void updateTipLabel(PsiTipLabel* label, const QString& text, const ToolTipPosition* pos);
	virtual bool preventAnimation() const;
	virtual bool preventShow();

	virtual void updateTipLabelProperties(PsiTipLabel* label);
	void updateLabel(YaRosterTipLabel* label, const QString& text);
	bool mouseOverRect() const;

	friend class YaChatToolTip;
	friend class YaChatDlg;
	friend class PsiAccount;
	friend class PsiContact;
	friend class YaAddContactHelper;
};

#endif
