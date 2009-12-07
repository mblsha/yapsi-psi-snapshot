/*
 * yavisualutil.cpp - custom GUI routines
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

#include "yavisualutil.h"

#include <QFont>
#include <QColor>
#include <QPixmap>
#include <QTextDocument> // for Qt::escape
#include <QSize>
#include <QPainter>
#include <QIcon>
#include <QBitmap>
#include <QCoreApplication>
#include <QDir>
#include <QRegExp>
#include <QApplication>
#include <QDesktopWidget>

#include "utils.h"
#include "psioptions.h"
#include "psiiconset.h"
#include "psiaccount.h"
#include "avatars.h"
#include "psicontact.h"
#include "yacommon.h"
#include "vcardfactory.h"
#include "yawindow.h"
#include "yawindowtheme.h"
#include "borderrenderer.h"

#ifdef YAPSI_ACTIVEX_SERVER
#if QT_VERSION >= 0x040500
#define CUSTOM_SHADOW
#endif
#endif

class RoundedBitmapMaskCache : public QObject
{
public:
	static RoundedBitmapMaskCache* instance()
	{
		if (!instance_) {
			instance_ = new RoundedBitmapMaskCache();
		}
		return instance_;
	}

	bool supportsRadius(int radius)
	{
		return bitmaps_.contains(radius);
	}

	QBitmap* bitmapForRadius(int radius)
	{
		return bitmaps_[radius];
	}

private:
	RoundedBitmapMaskCache()
		: QObject(QCoreApplication::instance())
	{
		static QBitmap topLeftCorner(":/images/window/top_left_corner_mask_6px.bmp");

		QDir dir(":/images/window");
		QStringList filters;
		filters << "top_left_corner_mask*.bmp";
		QStringList bitmaps = dir.entryList(filters, QDir::NoFilter, QDir::Time);

		QRegExp rx("top_left_corner_mask_(\\d+)px");
		foreach(QString bitmapFileName, bitmaps) {
			Q_ASSERT(rx.indexIn(bitmapFileName) >= 0);
			int radius = rx.capturedTexts()[1].toInt();
			bitmaps_[radius] = new QBitmap(dir.absoluteFilePath(bitmapFileName));
		}

		Q_ASSERT(!bitmaps_.isEmpty());
	}

	~RoundedBitmapMaskCache()
	{
		qDeleteAll(bitmaps_);
		bitmaps_.clear();
	}

	static RoundedBitmapMaskCache* instance_;
	QHash<int, QBitmap*> bitmaps_;
};

RoundedBitmapMaskCache* RoundedBitmapMaskCache::instance_ = 0;

namespace Ya
{

QString VisualUtil::scaledAvatarPath(PsiAccount* account, const XMPP::Jid& jid, int toasterAvatarSize)
{
	Q_ASSERT(account);
	QString fileName = account->avatarFactory()->getCachedAvatarFileName(jid);
	if (fileName.isEmpty() || !QFile::exists(fileName)) {
		fileName = Ya::VisualUtil::noAvatarPixmapFileName(contactGender(account, jid));
	}

	QFileInfo fi(fileName);
	QString scaledSizeString = toasterAvatarSize == 50 ? QString() : QString::number(toasterAvatarSize);
	QString scaledAvatar = QString("%1_toaster%2.png")
	                       .arg(fi.baseName())
	                       .arg(scaledSizeString);
	QDir avatarsDir(account->avatarFactory()->getCacheDir());
	QString scaledAvatarPath = avatarsDir.absoluteFilePath(scaledAvatar);
	if (!QFile::exists(scaledAvatarPath)) {
		QImage image(fileName);
		image = image.scaled(QSize(toasterAvatarSize, toasterAvatarSize),
		                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
		image.save(scaledAvatarPath);
	}

	return scaledAvatarPath;
}

QString VisualUtil::contactName(PsiAccount* account, const XMPP::Jid& jid)
{
	PsiContact* contact = 0;
	if (account)
		contact = account->findContact(jid.bare());

	QString result = contact ? contact->name() : jid.bare();
	result = Ya::contactName(result, jid.bare());
	return result;
}

QString VisualUtil::contactGenderString(XMPP::VCard::Gender gender)
{
	if (gender == XMPP::VCard::Male)
		return "m";
	else if (gender == XMPP::VCard::Female)
		return "f";
	return "u";
}

XMPP::VCard::Gender VisualUtil::contactGender(PsiAccount* account, const XMPP::Jid& jid)
{
	PsiContact* contact = account->findContact(jid);
	XMPP::VCard::Gender gender = contact ? contact->gender() : XMPP::VCard::UnknownGender;
	if (!contact) {
		const VCard *vcard = VCardFactory::instance()->vcard(jid);
		if (vcard) {
			gender = vcard->gender();
		}
	}
	return gender;
}

QString Ya::VisualUtil::contactGenderString(PsiAccount* account, const XMPP::Jid& jid)
{
	return contactGenderString(contactGender(account, jid));
}

QString Ya::VisualUtil::contactAgeLocation(PsiAccount* account, const XMPP::Jid& jid, bool* stubTextWasUsed)
{
	Q_UNUSED(account);
	Q_ASSERT(stubTextWasUsed);
	*stubTextWasUsed = false;
	const VCard *vcard = VCardFactory::instance()->vcard(jid);

	QStringList info;
	if (vcard && vcard->age() > 0) {
		info << Ya::AgeFormatter::ageInYears(vcard->age());
	}

	if (vcard && !vcard->addressString().isEmpty()) {
		info << vcard->addressString();
	}

	QString text;
	QColor color = Qt::black;
	if (!info.isEmpty()) {
		text = info.join(", ");
	}
	else {
		if (vcard && vcard->gender() != XMPP::VCard::UnknownGender)
			text = tr("Age and locality are not specified");
		else
			text = tr("Gender, age and locality are not specified");
		*stubTextWasUsed = true;
	}

	return text + ".";
}

static QString fontStyle(const QFont& font)
{
	QStringList properties;
	properties << QString("font-family: %1").arg(font.family());
	if (font.bold())
		properties << "font-weight: bold";
	properties << QString("font-size: %1px").arg(font.pixelSize());
	return properties.join("; ");
}

QString VisualUtil::spanFor(const QColor& color, const QString& text)
{
	return QString("<span style='color: %1'>%2</span>").arg(color.name(), text);
}

QString VisualUtil::spanFor(const QColor& color, const QFont& font, const QString& text)
{
	return QString("<span style='color: %1; %2'>%3</span>").arg(color.name(), fontStyle(font), text);
}

QString VisualUtil::contactNameAndJid(QString name, QString jid, XMPP::Status::Type status)
{
	QString result;
	result += spanFor(statusColor(status, false),
	                  contactNameFont(RosterStyleSlim, status),
	                  Qt::escape(name));

	result += QString("&lt;%1&gt;").arg(contactJidAsRichText(jid));
	return result;
}

QString Ya::VisualUtil::contactJidAsRichText(QString jid)
{
	QString result = spanFor(statusColor(XMPP::Status::Online, false),
	                         contactNameFont(RosterStyleSlim, XMPP::Status::Offline),
	                         Qt::escape(jid));
	return result;
}

bool VisualUtil::contactIsAvailale(XMPP::Status::Type status)
{
	return status == XMPP::Status::Online ||
	       status == XMPP::Status::DND    ||
	       status == XMPP::Status::FFC;
}

QFont VisualUtil::contactNameFont(RosterStyle rosterStyle, XMPP::Status::Type status)
{
	QFont font("Arial");
	// font.setBold(contactIsAvailale(status));
	font.setPixelSize(contactNameFontSize(rosterStyle));
	return font;
}

QFont VisualUtil::contactStatusFont(RosterStyle rosterStyle)
{
	QFont font(rosterStyle == RosterStyleLarge ? "Arial" : "Tahoma");
	font.setPixelSize(contactStatusFontSize(rosterStyle));
	return font;
}

QColor VisualUtil::contactStatusColor(bool hovered)
{
	if (hovered) {
		return QColor(0x74,0x74,0x74);
	}
	return QColor(0x99,0x99,0x99);
}

QColor Ya::VisualUtil::rosterTabBorderColor()
{
	return contactStatusColor(false);
}

QColor Ya::VisualUtil::editAreaColor()
{
	return QColor(0xFF,0xFF,0xFF);
}

QColor Ya::VisualUtil::blueBackgroundColor()
{
	return QColor(0xCF,0xE3,0xFF);
}

QColor Ya::VisualUtil::yellowBackgroundColor()
{
	return QColor(0xFD, 0xEB, 0x9F);
}

QColor Ya::VisualUtil::tabHighlightColor()
{
	return QColor(0xAF,0xC0,0xD9);
}

QColor Ya::VisualUtil::highlightColor()
{
	return QColor(0XF3, 0XF8, 0XFF);
}

QColor Ya::VisualUtil::highlightBackgroundColor()
{
	return hoverHighlightColor();
}

QColor Ya::VisualUtil::hoverHighlightColor()
{
	return blueBackgroundColor();
}

QColor Ya::VisualUtil::downlightColor()
{
	return blueBackgroundColor();
}

QColor Ya::VisualUtil::deleteConfirmationBackgroundColor()
{
	return QColor(0x44, 0x66, 0xDD);
}

QColor Ya::VisualUtil::linkButtonColor()
{
	return QColor(0x00, 0x00, 0x80);
}

QColor VisualUtil::rosterGroupForeColor()
{
	return QColor(0x46, 0x46, 0x46);
}

QColor VisualUtil::rosterAccountForeColor()
{
	return QColor(0x73, 0x73, 0x73);
}

int VisualUtil::contactNameFontSize(RosterStyle rosterStyle)
{
	switch (rosterStyle) {
	case RosterStyleSlim:
	case RosterStyleNormal:
		return 12;
	case RosterStyleLarge:
		return 14;
	}
	Q_ASSERT(false);
	return 0;
}

int VisualUtil::contactStatusFontSize(RosterStyle rosterStyle)
{
	switch (rosterStyle) {
	case RosterStyleSlim:
	case RosterStyleNormal:
		return 10;
	case RosterStyleLarge:
		return 12;
	}
	Q_ASSERT(false);
	return 0;
}

QPixmap Ya::VisualUtil::statusPixmap(XMPP::Status::Type status)
{
	return PsiIconset::instance()->status(status).impix().pixmap();
}

#define ONLINE_COLOR  QColor(0x00,0x00,0x00)
#define AWAY_COLOR    QColor(0x28,0x78,0xd6)
#define OFFLINE_COLOR QColor(0xaa,0xaa,0xaa)
#define DND_COLOR     ONLINE_COLOR

QColor VisualUtil::statusColor(XMPP::Status::Type status, bool hovered)
{
	QColor result;
	switch (status) {
	case XMPP::Status::Offline:
#ifdef YAPSI
	case XMPP::Status::Blocked:
	case XMPP::Status::Reconnecting:
	case XMPP::Status::NotAuthorizedToSeeStatus:
#endif
		result = OFFLINE_COLOR;
		break;
	case XMPP::Status::XA:
	case XMPP::Status::Away:
		if (hovered) {
			return QColor(0x1d,0x58,0x9d);
		}
		result = AWAY_COLOR;
		break;
	case XMPP::Status::DND:
		result = DND_COLOR;
		break;
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
	case XMPP::Status::Invisible:
		result = ONLINE_COLOR;
		break;
	default:
		Q_ASSERT(false);
	}

	return result;
}

QColor VisualUtil::rosterToolTipStatusColor(XMPP::Status::Type status)
{
	QColor result;
	switch (status) {
	case XMPP::Status::Invisible:
	case XMPP::Status::Offline:
#ifdef YAPSI
	case XMPP::Status::Blocked:
	case XMPP::Status::Reconnecting:
	case XMPP::Status::NotAuthorizedToSeeStatus:
#endif
		result = Qt::black;
		break;
	case XMPP::Status::XA:
	case XMPP::Status::Away:
		result = Qt::gray;
		break;
	case XMPP::Status::DND:
		result = Qt::red;
		break;
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
		result = QColor(0x0C, 0x61, 0x00);
		break;
	default:
		Q_ASSERT(false);
	}

	return result;
}

QPixmap VisualUtil::rosterStatusPixmap(XMPP::Status::Type status)
{
	QPixmap result;
	switch (status) {
	case XMPP::Status::Offline:
		result = QPixmap(":iconsets/roster/default/icon-offline.png");
		break;
	case XMPP::Status::XA:
	case XMPP::Status::Away:
		result = QPixmap(":iconsets/roster/default/icon-away.png");
		break;
	case XMPP::Status::DND:
		result = QPixmap(":iconsets/roster/default/icon-busy.png");
		break;
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
	case XMPP::Status::Invisible:
		result = QPixmap(":iconsets/roster/default/icon-online.png");
		break;
#ifdef YAPSI
	case XMPP::Status::Blocked:
		result = QPixmap(":iconsets/roster/default/icon-lock.png");
		break;
	case XMPP::Status::Reconnecting:
		result = QPixmap(":iconsets/roster/default/icon-invisible.png");
		break;
	case XMPP::Status::NotAuthorizedToSeeStatus:
		result = QPixmap(":iconsets/roster/default/icon-warning.png");
		break;
#endif
	default:
		Q_ASSERT(false);
	}

	return result;
}

static void drawCorner(QPainter* p, int dx, int dy, int angle, const QBitmap* topLeftCorner)
{
	p->save();
	p->translate(dx, dy);
	QMatrix matrix = p->worldMatrix();
	matrix.rotate(angle);
	p->setWorldMatrix(matrix);
	p->drawPixmap(0, 0, *topLeftCorner);
	p->restore();
}

static QBitmap roundedBitmapMask(const QSize& size, int cornerRadius, Ya::VisualUtil::Borders borders)
{
	Q_ASSERT(RoundedBitmapMaskCache::instance()->supportsRadius(cornerRadius));
	QBitmap* topLeftCorner = RoundedBitmapMaskCache::instance()->bitmapForRadius(cornerRadius);
	Q_ASSERT(topLeftCorner);

	QBitmap bmp(size);
	if (size.isNull() || size.isEmpty())
		return bmp;
	QPainter p(&bmp);
	p.setBackgroundMode(Qt::OpaqueMode);

	p.fillRect(0, 0, size.width(), size.height(), Qt::color1);

	if (borders & Ya::VisualUtil::TopLeft) {
		drawCorner(&p, 0, 0, 0, topLeftCorner);
	}

	if (borders & Ya::VisualUtil::TopRight) {
		drawCorner(&p, size.width(), 0, 90, topLeftCorner);
	}

	if (borders & Ya::VisualUtil::BottomLeft) {
		drawCorner(&p, 0, size.height(), 270, topLeftCorner);
	}

	if (borders & Ya::VisualUtil::BottomRight) {
		drawCorner(&p, size.width(), size.height(), 180, topLeftCorner);
	}

	return bmp;
}

QRegion VisualUtil::roundedMask(const QSize& size, int cornerRadius, Borders borders)
{
	if (size.isEmpty()) {
		return QRegion();
	}

	if (RoundedBitmapMaskCache::instance()->supportsRadius(cornerRadius)) {
		return QRegion(roundedBitmapMask(size, cornerRadius, borders));
	}

	QRegion maskedRegion;
	maskedRegion += QRegion(cornerRadius, 0, size.width() - 2 * cornerRadius, size.height());
	maskedRegion += QRegion(0, cornerRadius, size.width(), size.height() - 2 * cornerRadius);

	if (borders & TopLeft)
		maskedRegion += QRegion(0, 0,
		                        2 * cornerRadius, 2 * cornerRadius, QRegion::Ellipse);
	else
		maskedRegion += QRegion(0, 0,
		                        cornerRadius, cornerRadius);

	if (borders & TopRight)
		maskedRegion += QRegion(size.width() - 2 * cornerRadius, 0,
		                        2 * cornerRadius, 2 * cornerRadius, QRegion::Ellipse);
	else
		maskedRegion += QRegion(size.width() - cornerRadius, 0,
		                        cornerRadius, cornerRadius);

	if (borders & BottomLeft)
		maskedRegion += QRegion(0, size.height() - 2 * cornerRadius,
		                        2 * cornerRadius, 2 * cornerRadius, QRegion::Ellipse);
	else
		maskedRegion += QRegion(0, size.height() - cornerRadius,
		                        cornerRadius, cornerRadius);

	if (borders & BottomRight)
		maskedRegion += QRegion(size.width() - 2 * cornerRadius, size.height() - 2 * cornerRadius,
		                        2 * cornerRadius, 2 * cornerRadius, QRegion::Ellipse);
	else
		maskedRegion += QRegion(size.width() - cornerRadius, size.height() - cornerRadius,
		                        cornerRadius, cornerRadius);

	return maskedRegion;
}

QRegion Ya::VisualUtil::bottomAACornersMask(const QRect& rect)
{
	QRegion mask(rect);
	mask -= QRegion(0, rect.height() - 1, 1, 1);
	mask -= QRegion(rect.width() - 1, rect.height() - 1, 1, 1);
	return mask;
}

int VisualUtil::rosterGroupCornerRadius()
{
	return 6;
}

int Ya::VisualUtil::belowScreenGeometry()
{
	QRegion allScreens;
	for (int i = 0; i < qApp->desktop()->numScreens(); ++i)
		allScreens += qApp->desktop()->screenGeometry(i);
	return allScreens.boundingRect().bottom();
}

QString Ya::VisualUtil::noAvatarPixmapFileName(XMPP::VCard::Gender gender)
{
	switch (gender) {
	case XMPP::VCard::UnknownGender:
		return ":/images/gender/default_avatar.png";
	case XMPP::VCard::Male:
		return ":/images/avatars/avatar17.png";
	case XMPP::VCard::Female:
		return ":/images/avatars/avatar18.png";
	}
	Q_ASSERT(false);
	return QString();
}

const QPixmap& VisualUtil::noAvatarPixmap(XMPP::VCard::Gender gender)
{
	static QPixmap pix_male;
	static QPixmap pix_female;
	static QPixmap pix_unknown;
	switch (gender) {
	case XMPP::VCard::UnknownGender:
		if (pix_unknown.isNull())
			pix_unknown = QPixmap(noAvatarPixmapFileName(gender));
		return pix_unknown;
	case XMPP::VCard::Male:
		if (pix_male.isNull())
			pix_male = QPixmap(noAvatarPixmapFileName(gender));
		return pix_male;
	case XMPP::VCard::Female:
		if (pix_female.isNull())
			pix_female = QPixmap(noAvatarPixmapFileName(gender));
		return pix_female;
	}
	Q_ASSERT(false);
	return QPixmap();
}

void VisualUtil::drawAvatar(QPainter* painter, const QRect& avatarRect, XMPP::Status::Type status, XMPP::VCard::Gender gender, const QIcon& avatar, bool useNoAvatarPixmap)
{
	QIcon::Mode mode = QIcon::Normal;
	painter->save();
	if (status == XMPP::Status::Offline ||
	    status == XMPP::Status::Blocked ||
	    status == XMPP::Status::NotAuthorizedToSeeStatus)
	{
		painter->setOpacity(0.5);
		mode = QIcon::Disabled;
	}

	QIcon icon = avatar;
	if (icon.isNull() /*&& useNoAvatarPixmap*/)
		icon = Ya::VisualUtil::noAvatarPixmap(gender);
	QRect rect = avatarRect;
	rect.setHeight(qMin(avatarRect.height(), icon.actualSize(avatarRect.size()).height()));
	icon.paint(painter, rect, Qt::AlignCenter, mode, QIcon::On);

	// painter->drawRect(rect.adjusted(0, 0, -1, -1));
	painter->restore();
}

// TODO: could alternatively use a small cache for different fadeOutSizes
// and translate the matrix to make sure that gradient is always painted
// at the right offset
void Ya::VisualUtil::drawTextFadeOut(QPainter* painter, const QRect& rect, const QColor& background, int fadeOutSize)
{
	QRect r(rect.right() - fadeOutSize + 1, rect.top(), fadeOutSize, rect.height());
	QLinearGradient linearGrad(
		QPointF(r.left(), r.top()), QPointF(r.right(), r.top())
	);
	QColor transparent(background);
	transparent.setAlpha(0);
	linearGrad.setColorAt(0, transparent);
	linearGrad.setColorAt(1, background);
	painter->fillRect(r, linearGrad);
}

void Ya::VisualUtil::drawTextDashedUnderline(QPainter* painter, const QString& text, const QRect& textRect, int flags)
{
	painter->setPen(Qt::DashLine);
	int w = qMin(painter->fontMetrics().width(text), textRect.width());
	int x = (textRect.width() - w) / 2;
	if (flags & Qt::AlignLeft) {
		x = textRect.left();
	}
	int y = textRect.top() + ((textRect.height() - painter->fontMetrics().height()) / 2) + painter->fontMetrics().ascent() + 1;
	Q_ASSERT(flags & Qt::AlignVCenter);

// #ifdef Q_WS_WIN
	y += 1;
// #endif
	painter->drawLine(x, y,
	                  x + w, y);
}

void Ya::VisualUtil::paintRosterBackground(QWidget* widget, QPainter* p)
{
	Q_ASSERT(widget);
	Q_ASSERT(widget->window());
	QPoint g = widget->mapToGlobal(widget->rect().topLeft()) - widget->window()->mapToGlobal(widget->window()->rect().topLeft());
	YaWindow* yaWindow = dynamic_cast<YaWindow*>(widget->window());
	QRect r = widget->rect().adjusted(-g.x() + yaWindow->additionalLeftMargin(), -g.y() + yaWindow->additionalTopMargin(), 0, 0);

	YaWindowTheme theme(YaWindowTheme::Roster);
	theme.paintBackground(p, r, yaWindow ? yaWindow->showAsActiveWindow() : widget->isActiveWindow());
}

int Ya::VisualUtil::windowShadowSize()
{
	return 9;
}

int Ya::VisualUtil::windowCornerRadius()
{
#ifdef Q_WS_MAC
	// http://trolltech.com/developer/task-tracker/index_html?method=entry&id=201941
	// currently use of pre-rendered round corners on Mac OS X is quite buggy
	// as it often results a semi-transparent corner image
	// return 10;
#endif
	return 6;
}

void Ya::VisualUtil::drawWindowTheme(QPainter* painter, const YaWindowTheme& theme, const QRect& rect, const QRect& contentsRect, bool showAsActiveWindow)
{
#ifdef CUSTOM_SHADOW
	if (theme.customFrame()) {
		// painter->fillRect(rect, Qt::white);

		QRect shadowRect(contentsRect);
		shadowRect.adjust(-windowShadowSize(), -windowShadowSize(), windowShadowSize(), windowShadowSize());

		BorderRenderer* border = BorderRenderer::rendererFor(":images/window/shadow");
		border->draw(painter, shadowRect.adjusted(0, 0, 1, 1));

		QRegion mask = Ya::VisualUtil::roundedMask(contentsRect.size(), windowCornerRadius(), Ya::VisualUtil::TopBorders);
		mask -= QRegion(0, contentsRect.height() - 1, 1, 1);
		mask -= QRegion(contentsRect.width() - 1, contentsRect.height() - 1, 1, 1);
		mask.translate(contentsRect.left(), contentsRect.top());

		painter->setClipRegion(mask);
	}
#endif
	theme.paintBackground(painter, contentsRect, showAsActiveWindow);
}

void Ya::VisualUtil::drawAACorners(QPainter* painter, const YaWindowTheme& theme, const QRect& rect, const QRect& contentsRect)
{
#ifdef CUSTOM_SHADOW
	if (!theme.customFrame())
#endif
		return;

	BorderRenderer* border = BorderRenderer::rendererFor(":images/window/aa-corner");
	border->draw(painter, contentsRect.adjusted(0, 0, 1, 1));
}

QFont VisualUtil::normalFont()
{
	QFont font("Arial");
	font.setPixelSize(12); // TODO: should be Arial 80%
	return font;
}

QFont VisualUtil::mediumFont()
{
	QFont font("Arial");
	font.setPixelSize(11); // TODO: should be Arial 75%
	return font;
}

QFont VisualUtil::smallFont()
{
	QFont font("Verdana");
	font.setPixelSize(10); // TODO: should be Verdana 70%
	return font;
}

const QIcon& VisualUtil::defaultWindowIcon()
{
	static QIcon pix;
	if (pix.isNull()) {
		pix.addFile(":iconsets/system/default/logo_16.png", QSize(16, 16));
		pix.addFile(":iconsets/system/default/logo_32.png", QSize(32, 32));
		pix.addFile(":iconsets/system/default/logo_48.png", QSize(48, 48));
	}
	Q_ASSERT(!pix.isNull());
	return pix;
}

const QIcon& Ya::VisualUtil::chatWindowIcon()
{
	static QIcon pix;
	if (pix.isNull()) {
		pix.addFile(":images/chat_windowicon.png", QSize(16, 16));
		pix.addFile(":images/chat_windowicon32.png", QSize(32, 32));
	}
	Q_ASSERT(!pix.isNull());
	return pix;
}

const QPixmap& VisualUtil::messageOverlayPixmap()
{
	static QPixmap pix;
	if (pix.isNull())
		pix = QPixmap(":images/envelope_overlay.png");
	Q_ASSERT(!pix.isNull());
	return pix;
}

QPixmap VisualUtil::createOverlayPixmap(const QPixmap& base, const QPixmap& overlay)
{
	QPixmap result(base);
	QPainter p(&result);
	p.drawPixmap(0, 0, overlay);
	return result;
}

QPixmap VisualUtil::dashBackgroundPixmap()
{
	static QPixmap pix;

	if (pix.isNull()) {
		pix = QPixmap(":images/group/background.png");
	}
	return pix;
}

QList<QPixmap> VisualUtil::closeTabButtonPixmaps()
{
	QList<QPixmap> result;
	QDir dir(":images/chat/closetab_button");
	foreach(QString file, dir.entryList()) {
		result << QPixmap(dir.absoluteFilePath(file));
	}
	return result;
}

};
