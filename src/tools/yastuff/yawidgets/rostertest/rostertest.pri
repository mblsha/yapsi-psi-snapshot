SOURCES += \
	$$PWD/testmodel.cpp \
	$$PWD/../yacontactlistview.cpp \
	$$PWD/../yacontactlistviewdelegate.cpp \
	$$PWD/../yacontactlistviewslimdelegate.cpp \
	$$PWD/../yacontactlistviewlargedelegate.cpp \
	$$PWD/../../../../contactlistviewdelegate.cpp

HEADERS += \
	$$PWD/testmodel.h \
	$$PWD/../yacontactlistview.h \
	$$PWD/../yacontactlistviewdelegate.h \
	$$PWD/../yacontactlistviewslimdelegate.h \
	$$PWD/../yacontactlistviewlargedelegate.h \
	$$PWD/../../../../contactlistviewdelegate.h

INTERFACES += \
	$$PWD/rostertest.ui

include($$PWD/../../../smoothscrollbar/smoothscrollbar.pri)
include($$PWD/../stubs/stubs.pri)
DEFINES += NO_ICONSET_ZIP NO_ICONSET_SOUND
include($$PWD/../../../iconset/iconset.pri)

RESOURCES += \
	$$PWD/rostertest.qrc
