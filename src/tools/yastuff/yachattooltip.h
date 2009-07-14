/*
 * yachattooltip.h - custom contact tooltip for chat dialog
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

#ifndef YACHATTOOLTIP_H
#define YACHATTOOLTIP_H

#include "yarostertooltip.h"

class YaChatToolTipPosition : public ToolTipPosition
{
public:
	YaChatToolTipPosition(QRect rect, const QWidget* parentWidget)
		: ToolTipPosition(rect.topLeft(), parentWidget)
		, globalRect(rect)
	{}

	virtual QPoint calculateTipPosition(const QWidget* label, const QSize& size) const;

private:
	QRect globalRect;
};

class YaChatToolTip : public YaRosterToolTip
{
public:
	static YaChatToolTip* instance();

	// reimplemented
	virtual bool isVisible() const;

protected:
	YaChatToolTip() {}

private:
	// reimplemented
	virtual ToolTipPosition* createTipPosition(const QPoint& cursorPos, const QWidget* parentWidget);
	PsiTipLabel* createTipLabel(const QString& text, QWidget* parent);
	void updateTipLabelProperties(PsiTipLabel* label);

	static YaChatToolTip* instance_;
};

#endif
