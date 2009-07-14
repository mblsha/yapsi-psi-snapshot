/*
 * yawindowtheme.h
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

#ifndef YAWINDOWTHEME_H
#define YAWINDOWTHEME_H

#include <QObject>
#include <QPixmap>
#include <QVector>

class QDir;
class QDomElement;

class YaWindowTheme : public QObject
{
public:
	enum Mode {
		None = 0,
		Random,
		Roster,
		Chat,
		ChatFirst,
		ProgressDialog
	};

	YaWindowTheme();
	YaWindowTheme(Mode mode);

	static QStringList funnyThemes();

	class Theme
	{
	public:
		Theme() {}
		Theme(const QDomElement& e);

		const QString& name() const { return name_; }
		const QString& fileName() const { return fileName_; }
		const QPixmap& top() const { return top_; }
		const QPixmap& relief() const { return relief_; }
		const QPixmap& onlineExpansion() const { return onlineExpansion_; }
		const QColor& bottomColor() const { return bottomColor_; }
		const QColor& tabBackgroundColor() const { return tabBackgroundColor_; }
		const QColor& tabBlinkColor() const { return tabBlinkColor_; }
		const QColor& rosterLogoColor() const { return rosterLogoColor_; }
		const QPixmap& rosterLogoBackground() const { return rosterLogoBackground_; }
		const QColor& nickColor() const { return nickColor_; }
		const QColor& preferencesTabTextActive() const { return preferencesTabTextActive_; }
		const QColor& preferencesTabTextInactive() const { return preferencesTabTextInactive_; }
		const QColor& profileTextColor() const { return profileTextColor_; }
		const QColor& profileBackgroundColor() const { return profileBackgroundColor_; }

		const QPixmap& closeButton(bool hovered) const { return closeButton_[buttonPixmapIndex(hovered)]; }
		const QPixmap& maximizeButton(bool hovered) const { return maximizeButton_[buttonPixmapIndex(hovered)]; }
		const QPixmap& minimizeButton(bool hovered) const { return minimizeButton_[buttonPixmapIndex(hovered)]; }
		const QPixmap& gearButton(bool hovered) const { return gearButton_[buttonPixmapIndex(hovered)]; }
		const QPixmap& profileBackground() const { return profileBackground_; }

	private:
		QString name_;
		QString fileName_;
		QPixmap top_;
		QPixmap relief_;
		QPixmap onlineExpansion_;
		QColor bottomColor_;
		QColor tabBackgroundColor_;
		QColor tabBlinkColor_;
		QColor rosterLogoColor_;
		QPixmap rosterLogoBackground_;
		QColor nickColor_;
		QColor preferencesTabTextActive_;
		QColor preferencesTabTextInactive_;
		QColor profileTextColor_;
		QColor profileBackgroundColor_;

		static const int THEME_BUTTON_COUNT = 2;
		QPixmap closeButton_[THEME_BUTTON_COUNT];
		QPixmap maximizeButton_[THEME_BUTTON_COUNT];
		QPixmap minimizeButton_[THEME_BUTTON_COUNT];
		QPixmap gearButton_[THEME_BUTTON_COUNT];
		QPixmap profileBackground_;

		int buttonPixmapIndex(bool hovered) const;
	};

	QString name() const;
	const Theme& theme() const;
	void paintBackground(QPainter* p, const QRect& rect, bool isActiveWindow) const;
	bool customFrame() const;

protected slots:
	virtual void optionChanged(const QString& option);

private:
	Q_DISABLE_COPY(YaWindowTheme);

	Mode mode_;
	int theme_;
	bool customFrame_;
	static QVector<Theme> themes_;
	static int randomTheme_;
	static int defaultTheme_;
	static int progressDialogTheme_;
	mutable const Theme* currentTheme_;
	friend class Theme;

	static void ensureThemes();
	void init(Mode mode);
	void init(const QString& name);

	static QDir dir();
	static QDir onlineExpansionDir();
	static QString randomTheme();
};

#endif
