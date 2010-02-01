CONFIG += release

TEMPLATE = app
TARGET   = mrimhelper
CONFIG  += qt thread x11

unix:!mac {
	exists($$[QT_INSTALL_LIBS]/libQtCore.a) {
		CONFIG += qt_static
	}
}
windows: {
	exists($$[QT_INSTALL_LIBS]/QtCore.lib) {
		CONFIG += qt_static
	}
}

windows: include($$PWD/../conf_windows.pri)

win32-msvc|win32-msvc.net|win32-msvc2005|win32-msvc2008 {
	CONFIG += win32-msvc-flags
}

yapsi_activex_server {
	CONFIG  += qaxserver qaxcontainer
	contains(CONFIG, static): DEFINES += QT_NODLL

	# DEF_FILE = ../win32/yapsiserver.def
	RC_FILE  = ../win32/yapsiserver.rc
}
else {
	windows: RC_FILE = ../win32/psi_win32.rc
}

QT += xml network qt3support

#CONFIG += use_crash
# CONFIG += pep
DEFINES += QT_STATICPLUGIN

# Import several very useful Makefile targets 
# as well as set up default directories for 
# generated files
include(../qa/valgrind/valgrind.pri)
include(../qa/oldtest/unittest.pri)

# qconf

exists(../conf.pri) {
	include(../conf.pri)
}

windows {
	LIBS += -lWSock32 -lUser32 -lShell32 -lGdi32 -lAdvAPI32
	INCLUDEPATH += . # otherwise MSVC will fail to find "common.h" when compiling options/* stuff
}

qt_static {
	DEFINES += STATIC_IMAGEFORMAT_PLUGINS
	QTPLUGIN += qjpeg qgif
}

# Configuration
# IPv6 ?
#DEFINES += NO_NDNS

qca-static {
	# QCA
	DEFINES += QCA_STATIC
	include($$PWD/../third-party/qca/qca.pri)

	# QCA-OpenSSL
	contains(DEFINES, HAVE_OPENSSL) {
		include($$PWD/../third-party/qca/qca-ossl.pri)
	}
	
	# QCA-SASL
	contains(DEFINES, HAVE_CYRUSSASL) {
		include($$PWD/../third-party/qca/qca-cyrus-sasl.pri)
	}

	# QCA-GnuPG
	include($$PWD/../third-party/qca/qca-gnupg.pri)
}
else {
	CONFIG += crypto	
}

# Protection against buffer overruns
unix:debug {
	# QMAKE_CFLAGS   += -fstack-protector-all -Wstack-protector
	# QMAKE_CXXFLAGS += -fstack-protector-all -Wstack-protector
}
win32-msvc-flags:debug {
	# QMAKE_CFLAGS   += /GS
	# QMAKE_CXXFLAGS += /GS
}

# Speed up compilation process
win32-msvc-flags:debug {
	# /MD (Multithreaded runtime)  http://msdn2.microsoft.com/en-us/library/2kzt1wy3.aspx
	# /Gm (Enable Minimal Rebuild) http://msdn2.microsoft.com/en-us/library/kfz8ad09.aspx
	# /INCREMENTAL                 http://msdn2.microsoft.com/en-us/library/4khtbfyf.aspx
	QMAKE_CFLAGS   += /Gm
	QMAKE_CXXFLAGS += /Gm
	QMAKE_LFLAGS += /INCREMENTAL
}

# Platform specifics
win32 {
	# generate program debug detabase
	win32-msvc-flags {
		QMAKE_CFLAGS += /Zi
		QMAKE_LFLAGS += /DEBUG
	}

	# buggy MSVC workaround
	# win32-msvc-flags: QMAKE_LFLAGS += /FORCE:MULTIPLE
}
mac {
	# Universal binaries
	qc_universal:contains(QT_CONFIG,x86):contains(QT_CONFIG,ppc) {
		CONFIG += x86 ppc
		QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
		QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
	}

	QMAKE_LFLAGS += -framework CoreFoundation

	# Frameworks are specified in src.pri
	# QMAKE_INFO_PLIST = ../mac/Info.plist
	# RC_FILE = ../mac/application.icns
	# QMAKE_POST_LINK = \
	# 	mkdir -p `dirname $(TARGET)`/../Resources/iconsets/emoticons; \
	# 	cp -R tools/yastuff/iconsets/emoticons/* `dirname $(TARGET)`/../Resources/iconsets/emoticons; \
	# 	cp -R ../certs ../sound `dirname $(TARGET)`/../Resources; \
	# 	echo "APPLyach" > `dirname $(TARGET)`/../PkgInfo
}

SOURCES += \
	$$PWD/mrimhelper.cpp \
	$$PWD/../src/psilogger.cpp

HEADERS += \
	$$PWD/mrimhelper.h \
	$$PWD/../src/psilogger.h

INTERFACES += \
	$$PWD/mrimhelper.ui

include($$PWD/../cutestuff/cutestuff.pri)
include($$PWD/../src/tools/zip/zip.pri)
include($$PWD/../iris/iris.pri)

INCLUDEPATH += $$PWD/../src
DEPENDPATH  += $$PWD/../src
