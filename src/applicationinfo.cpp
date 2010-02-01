#include <QString>
#include <QDir>
#include <QFile>
#include <QSettings>

#if defined(Q_WS_X11) or defined(Q_WS_MAC)
#include <sys/stat.h> // chmod
#endif

#ifdef Q_WS_WIN
#if __GNUC__ >= 3
#	define WINVER    0x0500
#	define _WIN32_IE 0x0500
#endif
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef Q_WS_MAC
#include <CoreServices/CoreServices.h>
#endif

#include "applicationinfo.h"
#include "profiles.h"
#ifdef HAVE_CONFIG
#include "config.h"
#endif

#ifdef YAPSI
#include "yapsi_revision.h"
#endif

// Constants. These should be moved to a more 'dynamically changeable'
// place (like an external file loaded through the resources system)
// Should also be overridable through an optional file.

#ifdef YAPSI
#define PROG_NAME QString::fromUtf8("Я.Онлайн")
#define PROG_CAPS_NODE "http://online.yandex.ru/caps";
#define PROG_IPC_NAME "ru.yandex.online"
#define PROG_OPTIONS_NS "http://online.yandex.ru/options";
#define PROG_STORAGE_NS "http://online.yandex.ru/storage";
#ifdef Q_WS_MAC
#define PROG_APPCAST_URL "http://download.yandex.ru/yachat/darwin/version.xml";
#else
#define PROG_APPCAST_URL "";
#endif
#else
#define PROG_NAME "Psi"
#define PROG_VERSION "0.15-dev" " (" __DATE__ ")"; //CVS Builds are dated
//#define PROG_VERSION "0.15";
#define PROG_CAPS_NODE "http://psi-im.org/caps";
#define PROG_CAPS_VERSION "caps-b75d8d2b25";
#define PROG_IPC_NAME "org.psi-im.Psi"	// must not contain '\\' character on Windows
#define PROG_OPTIONS_NS "http://psi-im.org/options";
#define PROG_STORAGE_NS "http://psi-im.org/storage";
#ifdef Q_WS_MAC
#define PROG_APPCAST_URL "http://psi-im.org/appcast/psi-mac.xml";
#else
#define PROG_APPCAST_URL "";
#endif
#endif


#if defined(Q_WS_X11) && !defined(PSI_DATADIR)
#define PSI_DATADIR "/usr/local/share/psi"
#endif


QString ApplicationInfo::name()
{
	return PROG_NAME;
}

QString ApplicationInfo::version()
{
#ifdef YAPSI
	return YAPSI_VERSION + "." + QString::number(YAPSI_REVISION);
#else
	return PROG_VERSION;
#endif
}

QString ApplicationInfo::capsNode()
{
	return PROG_CAPS_NODE;
}

QString ApplicationInfo::capsVersion()
{
	return version();
}

QString ApplicationInfo::IPCName()
{
	return PROG_IPC_NAME;
}

QString ApplicationInfo::getAppCastURL()
{
	return PROG_APPCAST_URL;
}

QString ApplicationInfo::optionsNS()
{
	return PROG_OPTIONS_NS;
}

QString ApplicationInfo::storageNS()
{
	return PROG_STORAGE_NS;
}	

QStringList ApplicationInfo::getCertificateStoreDirs()
{
	QStringList l;
	l += ApplicationInfo::resourcesDir() + "/certs";
	l += ApplicationInfo::homeDir() + "/certs";
	return l;
}

QString ApplicationInfo::getCertificateStoreSaveDir()
{
	QDir certsave(homeDir() + "/certs");
	if(!certsave.exists()) {
		QDir home(homeDir());
		home.mkdir("certs");
	}

	return certsave.path();
}

QString ApplicationInfo::resourcesDir()
{
#if defined(Q_WS_X11)
	return PSI_DATADIR;
#elif defined(Q_WS_WIN)
	return qApp->applicationDirPath();
#elif defined(Q_WS_MAC)
	return QCoreApplication::instance()->applicationDirPath() + "/../Resources";
#endif
}

QString ApplicationInfo::libDir()
{
#if defined(Q_OS_UNIX)
	return PSI_LIBDIR;
#else
	return QString();
#endif
}

/** \brief return psi's private read write data directory
  * unix+mac: $HOME/.psi
  * environment variable "PSIDATADIR" overrides
  */
QString ApplicationInfo::homeDir()
{
	// Try the environment override first
	char *p = getenv("YACHATDATADIR");
	if(p) {
		QDir proghome(p);
		if (!proghome.exists()) {
			QDir d;
			d.mkpath(proghome.path());
		}
		return proghome.path();
	}

#if defined(Q_WS_X11) || defined(Q_WS_MAC)
	QDir proghome(QDir::homePath() + "/.yachat");
	if(!proghome.exists()) {
		QDir home = QDir::home();
		home.mkdir(".yachat");
		chmod(QFile::encodeName(proghome.path()), 0700);
	}
	return proghome.path();
#elif defined(Q_WS_WIN)
	QString base = QDir::homePath();
	WCHAR str[MAX_PATH+1] = { 0 };
	if (SHGetSpecialFolderPathW(0, str, CSIDL_APPDATA, true))
		base = QString::fromWCharArray(str);

	QDir proghome(base + "/YaChatData");
	if(!proghome.exists()) {
		QDir home(base);
		home.mkdir("YaChatData");
	}

	return proghome.path();
#endif
}

QString ApplicationInfo::historyDir()
{
	QDir history(pathToProfile(activeProfile) + "/history");
	if (!history.exists()) {
		QDir profile(pathToProfile(activeProfile));
		profile.mkdir("history");
	}

	return history.path();
}

QString ApplicationInfo::vCardDir()
{
	QDir vcard(pathToProfile(activeProfile) + "/vcard");
	if (!vcard.exists()) {
		QDir profile(pathToProfile(activeProfile));
		profile.mkdir("vcard");
	}

	return vcard.path();
}

QString ApplicationInfo::profilesDir()
{
	QString profiles_dir(homeDir() + "/profiles");
	QDir d(profiles_dir);
	if(!d.exists()) {
		QDir d(homeDir());
		d.mkdir("profiles");
	}
	return profiles_dir;
}

QString ApplicationInfo::documentsDir()
{
#ifdef Q_WS_WIN
	// http://lists.trolltech.com/qt-interest/2006-10/thread00018-0.html
	// TODO: haven't a better way been added to Qt since?
	QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
	settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
	return settings.value("Personal").toString();
#else
	return QDir::homePath();
#endif
}

#ifdef YAPSI
QString ApplicationInfo::yavatarsDir()
{
	QDir yavatars(pathToProfile(activeProfile) + "/yavatars");
	if (!yavatars.exists()) {
		QDir profile(pathToProfile(activeProfile));
		profile.mkdir("yavatars");
	}

	return yavatars.path();
}

QString ApplicationInfo::yahistoryDir()
{
	QDir yahistory(pathToProfile(activeProfile) + "/yahistory");
	if (!yahistory.exists()) {
		QDir profile(pathToProfile(activeProfile));
		profile.mkdir("yahistory");
	}

	return yahistory.path();
}
#endif

#ifdef HAVE_BREAKPAD
QString ApplicationInfo::crashReporterDir()
{
	QString crashReporterDir(homeDir() + "/crash_reporter");
	QDir d(crashReporterDir);
	if (!d.exists()) {
		QDir d(homeDir());
		d.mkdir("crash_reporter");
	}
	return crashReporterDir;
}
#endif
