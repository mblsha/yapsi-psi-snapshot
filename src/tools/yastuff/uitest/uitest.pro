# the rest
TEMPLATE = app
CONFIG  += qt uitools debug
CONFIG  -= app_bundle

DEFINES += WIDGET_PLUGIN

INCLUDEPATH += ..
DEPENDPATH += ..

SOURCES += \
	$$PWD/uitest.cpp \
	$$PWD/stylesheeteditor.cpp \
	$$PWD/../yastyle.cpp

HEADERS += \
	$$PWD/stylesheeteditor.h \
	$$PWD/../yastyle.h

INTERFACES += \
	$$PWD/stylesheeteditor.ui

include($$PWD/../../../../unittest/unittest.pri)

RESOURCES += \
	$$PWD/../yastuff.qrc
