#ifndef APPLICATIONINFO_H
#define APPLICATIONINFO_H

class QString;

class ApplicationInfo
{
public:
	// Version info
	static QString name();
	static QString version();
	static QString capsNode();
	static QString capsVersion();
	static QString IPCName();

	// URLs
	static QString getAppCastURL();

	// Directories
	static QString homeDir();
	static QString resourcesDir();
	static QString libDir();
	static QString profilesDir();
	static QString historyDir();
	static QString vCardDir();
	static QString documentsDir();
#ifdef YAPSI
	static QString yavatarsDir();
	static QString yahistoryDir();
#endif	
#ifdef HAVE_BREAKPAD
	static QString crashReporterDir();
#endif

	static QStringList getCertificateStoreDirs();
	static QString getCertificateStoreSaveDir();

	// Namespaces
	static QString optionsNS();
	static QString storageNS();
};

#endif
