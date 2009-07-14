TEMPLATE = app
CONFIG += qt debug
CONFIG -= app_bundle
QT += gui qt3support

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

include(../stubs/stubs.pri)
include(../../../../../unittest/unittest.pri)
include(../../../animationhelpers/animationhelpers.pri)

RESOURCES += \
	$$PWD/newselfavatar.qrc \

SOURCES += \
	main.cpp \
	yaselfmood.cpp \
	yaselfmoodaction.cpp \
	$$PWD/../borderrenderer.cpp \
	$$PWD/../yavisualutil.cpp

HEADERS += \
	yaselfmood.h \
	yaselfmoodaction.h \
	$$PWD/../overlaywidget.h \
	$$PWD/../borderrenderer.h \
	$$PWD/../yavisualutil.h

INTERFACES += \
	test.ui

windows {
	# otherwise we would get 'unresolved external _WinMainCRTStartup'
	# when compiling with MSVC
	MOC_DIR     = _moc
	OBJECTS_DIR = _obj
	UI_DIR      = _ui
	RCC_DIR     = _rcc
}
!windows {
	MOC_DIR     = .moc
	OBJECTS_DIR = .obj
	UI_DIR      = .ui
	RCC_DIR     = .rcc
}

