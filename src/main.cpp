/*
 * main.cpp - initialization and profile/settings handling
 * Copyright (C) 2001-2003  Justin Karneges
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

#ifdef HAVE_BREAKPAD
#include "breakpad.h"
#include "yapsi_revision.h" // auto-generated file, see src.pro for details
#endif

#ifdef HAVE_SPARKLE
extern "C" {
	#include "carboncocoa.h"
	#include "sparkle.h"
}
#endif

#ifdef YAPSI_ACTIVEX_SERVER
#include <QAxFactory>
#include <QProcess>
#include "yaonline.h"
#endif
#ifdef YAPSI
#include "yaipc.h"
#include "yalicense.h"
#endif

#include "main.h"

#include "psiapplication.h"
#include <qtimer.h>

#include <qimage.h>
#include <qbitmap.h>
#include <qtextcodec.h>
#include <qsettings.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <QtCrypto>
#include <QTranslator>
#include <QDir>

#include <stdlib.h>
#include <time.h>
#include "profiledlg.h"
#include "activeprofiles.h"
#include "psioptions.h"

#include "eventdlg.h"
#include "psicon.h"
#include "psiiconset.h"
#include "translationmanager.h"
#include "applicationinfo.h"
#ifdef USE_CRASH
#	include"crash.h"
#endif

#ifdef Q_WS_MAC
#include "CocoaUtilities/CocoaInitializer.h"
#endif

#include "psilogger.h"

#ifdef Q_OS_WIN
#	include <qt_windows.h> // for RegDeleteKey
#endif

/** \mainpage Psi API Documentation
 *
 *	\section intro_sec Indroduction
 *		Let's write an introduction to go here
 *	
 *	\section Installation
 *		For installation details, please see the INSTALL file
 *
 *	\section Contact Details
 *		And here we might put our contact details
 */
  


using namespace XMPP;

PsiMain::PsiMain(const QString& uriToOpen, QObject *par)
	: QObject(par)
	, uri(uriToOpen)
{
	LOG_TRACE;
	pcon = 0;

#ifndef YAPSI
	// migrate old (pre 0.11) registry settings...
	QSettings sUser(QSettings::UserScope, "psi-im.org", "Psi");
#else
	QSettings sUser(QSettings::UserScope, "Yandex", "Ya.Chat");
#endif
	lastProfile = sUser.value("last_profile").toString();
	lastLang = sUser.value("last_lang").toString();
	autoOpen = sUser.value("auto_open", QVariant(false)).toBool();

	QSettings s(ApplicationInfo::homeDir() + "/psirc", QSettings::IniFormat);
	lastProfile = s.value("last_profile", lastProfile).toString();
	lastLang = s.value("last_lang", lastLang).toString();
	autoOpen = s.value("auto_open", autoOpen).toBool();

#ifdef YAPSI
#ifdef YAPSI_ACTIVEX_SERVER
	lastProfile = YaOnlineHelper::instance()->activeProfile();
#else
	lastProfile = "default";
#endif
	autoOpen = true;
#endif

#ifndef YAPSI
	if(lastLang.isEmpty()) {
		lastLang = QTextCodec::locale();
		//printf("guessing locale: [%s]\n", lastLang.latin1());
	}
#else
	lastLang = "ru"; // QString(QTextCodec::locale()).left(2);
#endif

	TranslationManager::instance()->loadTranslation(lastLang);

#ifndef YAPSI
	if(autoOpen && !lastProfile.isEmpty() && profileExists(lastProfile)) {
		// Auto-open the last profile
		activeProfile = lastProfile;
		QTimer::singleShot(0, this, SLOT(sessionStart()));
	}
	else if (!lastProfile.isEmpty() && !getProfilesList().isEmpty()) {
		// Select a profile
		QTimer::singleShot(0, this, SLOT(chooseProfile()));
	}
	else if (getProfilesList().count() == 1) {
		// Open the (only) profile
		activeProfile = getProfilesList()[0];
		QTimer::singleShot(0, this, SLOT(sessionStart()));
	}
	else if (!getProfilesList().isEmpty()) {
		// Select a profile
		QTimer::singleShot(0, this, SLOT(chooseProfile()));
	}
	else {
#endif
		QString newProfile = "default";
#ifdef YAPSI_ACTIVEX_SERVER
		newProfile = YaOnlineHelper::instance()->activeProfile();
#endif
		// Create & open the default profile
		if (!profileExists(newProfile) && !profileNew(newProfile)) {
			QMessageBox::critical(0, tr("Error"), 
				tr("There was an error creating the %1 profile.").arg(newProfile));
			QTimer::singleShot(0, this, SLOT(bail()));
		}
		else {
			lastProfile = activeProfile = newProfile;
			// options.xml will be created by PsiCon::init
			autoOpen = true;
			QTimer::singleShot(0, this, SLOT(sessionStart()));
		}
#ifndef YAPSI
	}
#endif
}

PsiMain::~PsiMain()
{
	LOG_TRACE;
	delete pcon;
	LOG_TRACE;

	QSettings s(ApplicationInfo::homeDir() + "/psirc", QSettings::IniFormat);
	s.setValue("last_profile", lastProfile);
	s.setValue("last_lang", lastLang);
	s.setValue("auto_open", autoOpen);
}

void PsiMain::chooseProfile()
{
	LOG_TRACE;
	if(pcon) {
		delete pcon;
		pcon = 0;
	}

	QString str = "";

	// dirty, dirty, dirty hack
	PsiIconset::instance()->loadSystem();

	while(1) {
		ProfileOpenDlg *w = new ProfileOpenDlg(lastProfile, TranslationManager::instance()->availableTranslations(), TranslationManager::instance()->currentLanguage());
		w->ck_auto->setChecked(autoOpen);
		int r = w->exec();
		// lang change
		if(r == 10) {
			TranslationManager::instance()->loadTranslation(w->newLang);
			lastLang = TranslationManager::instance()->currentLanguage();
			delete w;
			continue;
		}
		else {
			bool again = false;
			autoOpen = w->ck_auto->isChecked();
			if(r == QDialog::Accepted) {
				str = w->cb_profile->currentText();
#ifndef YAPSI
				again = !ActiveProfiles::instance()->setThisProfile(str);
				if (again) {
					QMessageBox mb(QMessageBox::Question,
						CAP(tr("Profile already in use")),
						QString(tr("The \"%1\" profile is already in use.\nWould you like to activate that session now?")).arg(str),
						QMessageBox::Cancel);
					QPushButton *activate = mb.addButton(tr("Activate"), QMessageBox::AcceptRole);
					mb.exec();
					if (mb.clickedButton() == activate) {
						ActiveProfiles::instance()->raiseOther(str, true);
						quit();
						return;
					}
				}
#endif
			}
			delete w;
			if (again) {
				str = "";
				continue;
			}
			break;
		}
	}

	if(str.isEmpty()) {
		quit();
		return;
	}

	// only set lastProfile if the user opened it
	lastProfile = str;

	activeProfile = str;
	sessionStart();
}

void PsiMain::sessionStart()
{
	LOG_TRACE;
#ifndef YAPSI
	if (!ActiveProfiles::instance()->setThisProfile(activeProfile)) { // already running
		if (!ActiveProfiles::instance()->raiseOther(activeProfile, true)) {
			QMessageBox::critical(0, tr("Error"), tr("Cannot open this profile - it is already running, but not responding"));
		}
		quit();
		return;
	}
#endif

	// make sure we have clean PsiOptions
	PsiOptions::reset();
	// get a PsiCon
	pcon = new PsiCon();
	if (!pcon->init()) {
		delete pcon;
		pcon = 0;
		quit();
		return;
	}
	connect(pcon, SIGNAL(quit(int)), SLOT(sessionQuit(int)));
	if (!uri.isEmpty()) {
		pcon->doOpenUri(uri);
		uri.clear();
	}
}

void PsiMain::sessionQuit(int x)
{
	LOG_TRACE;
	if(x == PsiCon::QuitProgram) {
		QTimer::singleShot(0, this, SLOT(bail()));
	}
	else if(x == PsiCon::QuitProfile) {
		QTimer::singleShot(0, this, SLOT(chooseProfile()));
	}
}

void PsiMain::bail()
{
	LOG_TRACE;
	if(pcon) {
		delete pcon;
		pcon = 0;
	}
	quit();
}

PsiCon* PsiMain::controller() const
{
	return pcon;
}

#ifdef YAPSI_ACTIVEX_SERVER
static void startOnline()
{
	QSettings sUser(QSettings::UserScope, "Yandex", "Online");
	QDir programDir(sUser.value("ProgramDir").toString());
	QString onlineExe = programDir.absoluteFilePath("online.exe");
	if (QFile::exists(onlineExe)) {
		QStringList arguments;
		arguments << "-program:yachat.exe";
		if (YaOnline::onlineIsRunning()) {
			QProcess::execute(onlineExe, arguments);
		}
		else {
			QProcess::startDetached(onlineExe, arguments);
		}
	}
}
#endif

int main(int argc, char *argv[])
{
	// If Psi runs as uri handler the commandline might contain
	// almost arbitary network supplied data after the "--uri" argument.
	// To prevent any potentially dangerous options in Psi or
	// Qt to be triggered by this, filter out the uri and any following data
	// as early as possible.
	// see http://www.mozilla.org/security/announce/2007/mfsa2007-23.html
	// for how this problem affected firefox on windows.

	QByteArray uriBA;
	for (int i=1; i<argc; i++) {
		QByteArray str = QByteArray(argv[i]);
		QByteArray var, val;
		int x = str.find('=');
		if(x == -1) {
			var = str;
			val = "";
		} else {
			var = str.mid(0,x);
			val = str.mid(x+1);
		}

		if (var == "--uri") {
			uriBA = val;
#ifdef Q_WS_WIN
			// FIXME think about handling of quirks on the windows platform.
#endif
			if (uriBA.isEmpty() && i+1 < argc) {
				uriBA = QByteArray(argv[i+1]);
			}

			// terminate args here. Everything that follow mustn't be availible
			// in later commandline scanning.
			argc = i;
			argv[i] = 0;
			break;
		}
		
	}

	// NOTE: Qt 4.5 compatibility note: please don't move this call.
	//   instead, upgrade to QCA 2.0.2, which fixes the bug in the right
	//   place.
	QCA::Initializer init;
	// END NOTE

#ifdef Q_WS_MAC
	CocoaInitializer cocoaInitializer;
#endif

	// it must be initialized first in order for ApplicationInfo::resourcesDir() to work
	PsiApplication app(argc, argv);
	QApplication::addLibraryPath(ApplicationInfo::resourcesDir());
	QApplication::addLibraryPath(ApplicationInfo::homeDir());
	QApplication::setQuitOnLastWindowClosed(false);
	LOG_TRACE;

#ifdef Q_WS_MAC
	QDir dir(QApplication::applicationDirPath());
	dir.cdUp();
	dir.cd("Plugins");
	QApplication::addLibraryPath(dir.absolutePath());
#endif

#ifdef YAPSI
	bool ipcCommands = false;
	for(int n = 1; n < argc; ++n) {
		QString str = argv[n];
		if (str.startsWith("-ipc:")) {
			ipcCommands = true;
			QStringList ipc = str.split(":");
			Q_ASSERT(ipc.count() > 1);
			ipc.takeFirst();
			YaIPC::sendMessage(ipc.join(":"));
		}
	}

	if (YaIPC::isRunning() && !ipcCommands) {
#ifdef YAPSI_ACTIVEX_SERVER
		startOnline();
#endif

		ipcCommands = true;
		YaIPC::sendMessage("activate");
	}

	if (ipcCommands) {
		return 0;
	}
	YaIPC::initIPC();
#endif

	// Initialize QCA
	QCA::setProperty("pgp-always-trust", true);
	QCA::KeyStoreManager keystoremgr;
	QCA::KeyStoreManager::start();
	keystoremgr.waitForBusyFinished(); // FIXME get rid of this

#ifdef HAVE_BREAKPAD
	{
		QMap<QString, QString> p;
		p["name"]         = "yachat";
		p["version"]      = QString("%1.%2").arg(YAPSI_VERSION).arg(YAPSI_REVISION);
		p["report-url"]   = "http://localhost/cgi-bin/cgi-collector/collector.py";
		p["report-email"] = "report@e.mail";
		Breakpad::install(ApplicationInfo::crashReporterDir(), p);
	}
#endif

#ifdef USE_CRASH
	int useCrash = true;
	int i;
	for(i = 1; i < argc; ++i) {
		QString str = argv[i];
		if ( str == "--nocrash" )
			useCrash = false;
	}

	if ( useCrash )
		Crash::registerSigsegvHandler(argv[0]);
#endif

	// seed the random number generator
	srand(time(NULL));

	//dtcp_port = 8000;

	QString uri = QString::fromLocal8Bit(uriBA);
	for(int n = 1; n < argc; ++n) {
		QString str = argv[n];
		QString var, val;
		int x = str.find('=');
		if(x == -1) {
			var = str;
			val = "";
		}
		else {
			var = str.mid(0,x);
			val = str.mid(x+1);
		}

		//if(var == "--no-gpg")
		//	use_gpg = false;
		//else if(var == "--no-gpg-agent")
		//	no_gpg_agent = true;
		//else if(var == "--linktest")
		//	link_test = true;
	}

	if (!uri.isEmpty()) {
		if (ActiveProfiles::instance()->sendOpenUri(uri)) {
			return 0;
		}
	}

	//if(link_test)
	//	printf("Link test enabled\n");

	// silly winsock workaround
	//QSocketDevice *d = new QSocketDevice;
	//delete d;

	// Initialize translations
	TranslationManager::instance();

	// need SHA1 for Iconset sound
	//if(!QCA::isSupported(QCA::CAP_SHA1))
	//	QCA::insertProvider(XMPP::createProviderHash());

#ifdef HAVE_SPARKLE
	initSparkle();
#endif

#ifdef YAPSI_ACTIVEX_SERVER
	if (!QAxFactory::isServer()) {
		startOnline();
		return -1;
	}

	LOG_TRACE;
	int returnValue = app.exec();
	LOG_TRACE;
	return returnValue;
#else
#ifdef YAPSI
	LOG_TRACE;
	// make sure TranslationManager is initialized
	PsiMain *tmp = new PsiMain(QString());
	delete tmp;
	tmp = 0;

	if (!YaLicense::ensureLicenseAccepted()) {
		qWarning("License not accepted.");
		return -1;
	}
#endif

	LOG_TRACE;
	PsiMain *psi = new PsiMain(uri);

	QObject::connect(psi, SIGNAL(quit()), &app, SLOT(quit()));
	LOG_TRACE;
	int returnValue = app.exec();
	LOG_TRACE;
	delete psi;

	LOG_TRACE;
	return returnValue;
#endif
}

#ifdef QCA_STATIC
#include <QtPlugin>
#ifdef HAVE_OPENSSL
Q_IMPORT_PLUGIN(qca_ossl)
#endif
#ifdef HAVE_CYRUSSASL
Q_IMPORT_PLUGIN(qca_cyrus_sasl)
#endif
#ifndef YAPSI
Q_IMPORT_PLUGIN(qca_gnupg)
#endif
#endif

#if defined(STATIC_IMAGEFORMAT_PLUGINS)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qgif)
#endif

