CONFIG += unittest
include($$PWD/../../../../../../qa/oldtest/unittest.pri)

QT += xml

INCLUDEPATH += $$PWD/../..
DEPENDPATH  += $$PWD/../..

IRIS_BASE = $$PWD/../../../../../../iris

INCLUDEPATH += $$IRIS_BASE/include
DEPENDPATH  += $$IRIS_BASE/include
INCLUDEPATH += $$IRIS_BASE/xmpp-im
DEPENDPATH  += $$IRIS_BASE/xmpp-im

LIBIDN_BASE = $$IRIS_BASE/libidn
CONFIG += libidn
include($$IRIS_BASE/libidn.pri)

DEFINES += NO_DELIVERY_CONFIRMATION NO_YACOMMON NO_TEXTUTIL YAPSI NO_PSIOPTIONS

SOURCES += \
	$$PWD/testyachatviewmodel.cpp \
	$$PWD/../../yachatviewmodel.cpp \
	$$PWD/../../../../../../iris/xmpp-core/jid.cpp \
	$$PWD/../../../../../../iris/xmpp-core/xmpp_yadatetime.cpp

HEADERS += \
	$$PWD/../../yachatviewmodel.h
