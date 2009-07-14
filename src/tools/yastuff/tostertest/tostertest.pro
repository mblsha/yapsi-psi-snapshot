SOURCES += \
	$$PWD/main.cpp \
	$$PWD/yaautoreplacer.cpp

HEADERS += \
	$$PWD/yaautoreplacer.h

TEMPLATE = app

CONFIG += warn_on \
	  thread \
          qt

QT += gui \
	xml

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..
