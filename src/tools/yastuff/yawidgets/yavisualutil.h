/*
 * yavisualutil.h - custom GUI routines
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

#ifndef YAVISUALUTIL_H
#define YAVISUALUTIL_H

#include "xmpp_status.h"
#include "xmpp_vcard.h"

class QFont;
class QColor;
class QPixmap;
class QRegion;
class QSize;
class QPainter;
class QIcon;
class QRect;

class PsiAccount;
class YaWindowTheme;

namespace Ya
{

class VisualUtil : public QObject
{
	Q_OBJECT
public:
	static QString contactGender(XMPP::VCard::Gender gender);
	static QString scaledAvatarPath(PsiAccount* account, const XMPP::Jid& jid, int toasterAvatarSize = 50);
	static QString contactName(PsiAccount* account, const XMPP::Jid& jid);
	static QString contactGender(PsiAccount* account, const XMPP::Jid& jid);
	static QString contactAgeLocation(PsiAccount* account, const XMPP::Jid& jid, bool* stubTextWasUsed);

	enum RosterStyle {
		RosterStyleSlim = 0,
		RosterStyleNormal,
		RosterStyleLarge
	};

	static QFont normalFont();
	static QFont mediumFont();
	static QFont smallFont();

	static QString spanFor(const QColor& color, const QString& text);
	static QString spanFor(const QColor& color, const QFont& font, const QString& text);

	static QString contactNameAndJid(QString name, QString jid, XMPP::Status::Type status);
	static QString contactJidAsRichText(QString jid);

	static QFont contactNameFont(RosterStyle rosterStyle, XMPP::Status::Type status);
	static QFont contactStatusFont(RosterStyle rosterStyle);
	static QColor contactStatusColor(bool hovered);

	static QColor editAreaColor();
	static QColor rosterTabBorderColor();
	static QColor blueBackgroundColor();
	static QColor yellowBackgroundColor();
	static QColor tabHighlightColor();
	static QColor highlightColor();
	static QColor highlightBackgroundColor();
	static QColor hoverHighlightColor();
	static QColor downlightColor();
	static QColor deleteConfirmationBackgroundColor();
	static QColor linkButtonColor();

	static QColor rosterGroupForeColor();
	static QColor rosterAccountForeColor();

	static int rosterGroupCornerRadius();

	static int belowScreenGeometry();

	static QPixmap statusPixmap(XMPP::Status::Type status);
	static QColor statusColor(XMPP::Status::Type status, bool hovered);
	static QColor rosterToolTipStatusColor(XMPP::Status::Type status);
	static QPixmap rosterStatusPixmap(XMPP::Status::Type status);

	enum Borders {
		NoBorders   = 0,
		TopLeft     = 1 << 0,
		TopRight    = 1 << 1,
		BottomLeft  = 1 << 2,
		BottomRight = 1 << 3,

		TopBorders    = TopLeft | TopRight,
		BottomBorders = BottomLeft | BottomRight,
		LeftBorders   = TopLeft | BottomLeft,
		RightBorders  = TopRight | BottomRight,
		AllBorders    = TopBorders | BottomBorders
	};

	static QRegion roundedMask(const QSize& size, int cornerRadius, Borders borders);
	static QRegion bottomAACornersMask(const QRect& rect);

	static QString noAvatarPixmapFileName(XMPP::VCard::Gender gender);
	static const QPixmap& noAvatarPixmap(XMPP::VCard::Gender gender);
	static void drawAvatar(QPainter* painter, const QRect& rect, XMPP::Status::Type status, XMPP::VCard::Gender gender, const QIcon& avatar, bool useNoAvatarPixmap = true);
	static void drawTextFadeOut(QPainter* painter, const QRect& rect, const QColor& background, int fadeOutSize = 30);
	static void drawTextDashedUnderline(QPainter* painter, const QString& text, const QRect& textRect, int flags);

	static void paintRosterBackground(QWidget* widget, QPainter* p);
	static int windowShadowSize();
	static int windowCornerRadius();
	static void drawWindowTheme(QPainter* painter, const YaWindowTheme& theme, const QRect& rect, const QRect& contentsRect, bool showAsActiveWindow);
	static void drawAACorners(QPainter* painter, const YaWindowTheme& theme, const QRect& rect, const QRect& contentsRect);

	static const QIcon& defaultWindowIcon();
	static const QIcon& chatWindowIcon();
	static const QPixmap& messageOverlayPixmap();
	static QPixmap createOverlayPixmap(const QPixmap& base, const QPixmap& overlay);
	static QPixmap dashBackgroundPixmap();

private:
	static bool contactIsAvailale(XMPP::Status::Type status);
	static int contactNameFontSize(RosterStyle rosterStyle);
	static int contactStatusFontSize(RosterStyle rosterStyle);
};

};

#endif
