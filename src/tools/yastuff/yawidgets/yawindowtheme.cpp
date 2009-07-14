/*
 * yawindowtheme.cpp
 * Copyright (C) 2008-2009  Yandex LLC (Michail Pishchagin)
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

#include "yawindowtheme.h"

#include <QDir>
#include <QPainter>
#include <QDomDocument>
#include <QDomElement>

#include "psioptions.h"
#include "yavisualutil.h"
#include "pixmaputil.h"

static const QString customFrameOptionPath = "options.ya.custom-frame";
static const QString chatBackgroundOptionPath = "options.ya.chat-background";
static const QString officeBackgroundOptionPath = "options.ya.office-background";

struct YaWindowThemeButton {
	YaWindowThemeButton(const QString& _name, QPixmap* _pix)
		: pix(_pix)
		, name(_name)
	{}
	QPixmap* pix;
	QString name;
};

YaWindowTheme::Theme::Theme(const QDomElement& e)
{
	Q_ASSERT(e.hasAttribute("name"));
	name_ = e.attribute("name");
	Q_ASSERT(e.hasAttribute("background"));
	fileName_ = YaWindowTheme::dir().absoluteFilePath(e.attribute("background"));
	Q_ASSERT(!fileName_.isEmpty());
	top_ = QPixmap(fileName_);
	Q_ASSERT(!top_.isNull());

	onlineExpansion_ = QPixmap(YaWindowTheme::onlineExpansionDir().absoluteFilePath(e.attribute("background")));
	Q_ASSERT(!onlineExpansion_.isNull());

	if (e.hasAttribute("relief")) {
		relief_ = QPixmap(YaWindowTheme::dir().absoluteFilePath("relief/" + e.attribute("relief")));
		Q_ASSERT(!relief_.isNull());
	}

	QPixmap tmp = top_.copy(0, top_.height() - 1,
	                        top_.width(), 1);
	QImage tmpImg = tmp.toImage();
	bottomColor_ = tmpImg.pixel(0, 0);

	tabBackgroundColor_ = e.attribute("tabBackgroundColor");
	tabBlinkColor_ = e.attribute("tabBlinkColor");
	rosterLogoColor_ = e.attribute("rosterLogoColor");
	nickColor_ = e.attribute("nickColor");
	profileTextColor_ = e.attribute("profileTextColor");
	profileBackground_ = name_ == "academic" ?
	                     QPixmap(":images/chat/profile_background_academic.png") :
	                     QPixmap(":images/chat/profile_background.png");
	profileBackgroundColor_ = name_ == "academic" ?
	                          QColor("#E2E5E7") :
	                          QColor("#FFFFFF");

	QString rosterLogoBackgroundFileName(QString(":images/window/background/logo/%1.png").arg(name_));
	if (QFile::exists(rosterLogoBackgroundFileName)) {
		rosterLogoBackground_ = QPixmap(rosterLogoBackgroundFileName);
	}

	QDir buttonDir(QString(":images/window/buttons/%1").arg(name_));
	QDir buttonDirNormal(QString(":images/window/buttons/%1").arg("normal"));
	Q_ASSERT(buttonDirNormal.exists());
	if (!buttonDir.exists()) {
		buttonDir = buttonDirNormal;
	}

	QList<YaWindowThemeButton> buttons;
	buttons << YaWindowThemeButton("close", closeButton_);
	buttons << YaWindowThemeButton("maximize", maximizeButton_);
	buttons << YaWindowThemeButton("minimize", minimizeButton_);
	buttons << YaWindowThemeButton("gear", gearButton_);

	foreach(const YaWindowThemeButton& b, buttons) {
		float opacity = 0.65;
		if (name_ == "academic") {
			buttonDirNormal = buttonDir;
			opacity = 0.36;
		}

		b.pix[buttonPixmapIndex(false)] = QPixmap(QDir(buttonDirNormal).absoluteFilePath(b.name));
		b.pix[buttonPixmapIndex(true)] = QPixmap(QDir(buttonDir).absoluteFilePath(b.name));

		if (buttonDir == buttonDirNormal) {
			QPixmap pix = PixmapUtil::createTransparentPixmap(b.pix[buttonPixmapIndex(true)].size());
			QPainter p(&pix);
			p.setOpacity(opacity);
			p.drawPixmap(0, 0, b.pix[buttonPixmapIndex(true)]);
			b.pix[buttonPixmapIndex(false)] = pix;
		}
	}

	if (name_ == "academic") {
		preferencesTabTextActive_ = QColor(0x20, 0x20, 0x20);
		preferencesTabTextInactive_ = QColor(0x96, 0x96, 0x96);
	}
	else {
		preferencesTabTextActive_ = QColor(0x42, 0x42, 0x42);
		preferencesTabTextInactive_ = Qt::white;
	}
}

int YaWindowTheme::Theme::buttonPixmapIndex(bool hovered) const
{
	return hovered ? 1 : 0;
}

QVector<YaWindowTheme::Theme> YaWindowTheme::themes_;
int YaWindowTheme::randomTheme_ = -1;
int YaWindowTheme::defaultTheme_ = -1;
int YaWindowTheme::progressDialogTheme_ = -1;

YaWindowTheme::YaWindowTheme()
	: customFrame_(true)
	, currentTheme_(0)
{
	init(Random);
}

YaWindowTheme::YaWindowTheme(Mode mode)
	: customFrame_(true)
	, currentTheme_(0)
{
	init(mode);
}

QDir YaWindowTheme::dir()
{
	return QDir(":images/window/background");
}

QDir YaWindowTheme::onlineExpansionDir()
{
	return QDir(":images/window/online_expansion");
}

QString YaWindowTheme::randomTheme()
{
	ensureThemes();
	QStringList entries;
	foreach(const YaWindowTheme::Theme& theme, themes_) {
		entries << theme.name();
	}
	return entries[rand() % entries.count()];
}

void YaWindowTheme::init(Mode mode)
{
	mode_ = mode;
	theme_ = -1;

	if (mode == Random) {
		init(randomTheme());
	}
	else {
		init("glamour");
	}
	Q_ASSERT(theme_ >= 0);

	optionChanged(customFrameOptionPath);
}

void YaWindowTheme::ensureThemes()
{
	if (!themes_.isEmpty())
		return;

	QFile file(":images/window/themes.xml");
	if (!file.open(QIODevice::ReadOnly)) {
		Q_ASSERT(false);
		return;
	}
	QDomDocument doc;
	if (!doc.setContent(&file)) {
		Q_ASSERT(false);
		return;
	}

	QDomElement root = doc.documentElement();
	for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;

		Theme theme(e);
		themes_.append(theme);
	}

	for (int i = 0; i < themes_.count(); ++i) {
		if (themes_[i].name() == "ice") {
			defaultTheme_ = i;
		}

		if (themes_[i].name() == "academic") {
			progressDialogTheme_ = i;
		}
	}

	randomTheme_ = rand() % themes_.count();
	Q_ASSERT(themes_.count() > 0);
	Q_ASSERT(randomTheme_ != -1);
	Q_ASSERT(defaultTheme_ != -1);
	Q_ASSERT(progressDialogTheme_ != -1);

	{
		bool found = false;
		QString chatTheme = PsiOptions::instance()->getOption(chatBackgroundOptionPath).toString();
		foreach(const Theme& b, themes_) {
			if (b.name() == chatTheme) {
				found = true;
				break;
			}
		}

		if (!found) {
			bool officeTheme = PsiOptions::instance()->getOption(officeBackgroundOptionPath).toBool();
			PsiOptions::instance()->setOption(chatBackgroundOptionPath,
			                                  officeTheme ? "ice" :
			                                  "hawaii");
		}
	}
}

void YaWindowTheme::init(const QString& name)
{
	ensureThemes();
	Q_ASSERT(themes_.count());
	for (int i = 0; i < themes_.count(); ++i) {
		if (themes_[i].name() == name) {
			theme_ = i;
			break;
		}
	}
	Q_ASSERT(theme_ >= 0);
}

const YaWindowTheme::Theme& YaWindowTheme::theme() const
{
	currentTheme_ = &themes_[theme_];

	QString chatTheme = PsiOptions::instance()->getOption("options.ya.chat-background").toString();
	if (mode_ == ProgressDialog) {
		currentTheme_ = &themes_[progressDialogTheme_];
	}
	else if (chatTheme != "random") {
		bool found = false;
		foreach(const Theme& b, themes_) {
			if (b.name() == chatTheme) {
				found = true;
				currentTheme_ = &b;
				break;
			}
		}

		if (!found) {
			currentTheme_ = &themes_[defaultTheme_];
		}
	}
	else {
		currentTheme_ = &themes_[randomTheme_];
	}

	return *currentTheme_;
}

void YaWindowTheme::paintBackground(QPainter* p, const QRect& rect, bool isActiveWindow) const
{
	QRect pixmapRect(rect);
	pixmapRect.setHeight(theme().top().height());
	if (rect.bottom() < pixmapRect.bottom()) {
		pixmapRect.setBottom(rect.bottom());
	}

	p->save();
	if (!isActiveWindow) {
		p->fillRect(rect, Qt::white);
		p->setOpacity(0.75);
	}

	QRect plainRect(rect);
	plainRect.setTop(pixmapRect.bottom() + 1);

	p->drawTiledPixmap(pixmapRect, theme().top());
	if (plainRect.height() > 0) {
		p->fillRect(plainRect, theme().bottomColor());
	}

	if (!theme().relief().isNull()) {
		QRect reliefRect(pixmapRect);
		reliefRect.setHeight(theme().relief().height());
		p->drawTiledPixmap(reliefRect, theme().relief());
	}

	p->restore();
}

QString YaWindowTheme::name() const
{
	return currentTheme_ ? currentTheme_->name() : QString();
}

QStringList YaWindowTheme::funnyThemes()
{
	QStringList result;

	foreach(const Theme& b, themes_) {
		result << b.name();
	}

	return result;
}

void YaWindowTheme::optionChanged(const QString& option)
{
	if (option == customFrameOptionPath) {
		customFrame_ = PsiOptions::instance()->getOption(customFrameOptionPath).toBool();
	}
}

bool YaWindowTheme::customFrame() const
{
	return customFrame_;
}
