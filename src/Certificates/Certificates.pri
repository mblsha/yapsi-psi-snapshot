INCLUDEPATH *= $$PWD/..
DEPENDPATH *= $$PWD/.. $$PWD

INTERFACES += \
	$$PWD/CertificateDisplay.ui

HEADERS += \
	$$PWD/CertificateDisplayDialog.h \
	$$PWD/CertificateErrorDialog.h \
	$$PWD/CertificateHelpers.h

SOURCES += \
	$$PWD/CertificateDisplayDialog.cpp \
	$$PWD/CertificateErrorDialog.cpp \
	$$PWD/CertificateHelpers.cpp
