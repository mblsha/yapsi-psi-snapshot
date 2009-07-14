TEMPLATE = app
CONFIG += qt
CONFIG -= app_bundle
QT += gui
SOURCES += \
	trimmablemultilinelabeltestmain.cpp \
	trimmablemultilinelabel.cpp

HEADERS += \
	trimmablemultilinelabel.h

INTERFACES += \
	main.ui

INCLUDEPATH += $$PWD/..
DEPENDPATH  += $$PWD/..

include(../../../../../qa/oldtest/unittest.pri)
