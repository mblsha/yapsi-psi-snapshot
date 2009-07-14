TEMPLATE = app
CONFIG += qt debug
QT += gui xml qt3support

SOURCES += \
	chatseparatortestmain.cpp \
	../yachatseparator.cpp \
	../yaclosebutton.cpp \
	../../yastyle.cpp

HEADERS += \
	../yachatseparator.h \
	../overlaywidget.h \
	../yaclosebutton.h \
	../../yastyle.h

INCLUDEPATH += $$PWD/.. $$PWD/../..
DEPENDPATH  += $$PWD/.. $$PWD/../..

RESOURCES += chatseparatortest.qrc

DEFINES += NO_ICONSET_SOUND NO_ICONSET_ZIP WIDGET_PLUGIN
include(../../../iconset/iconset.pri)

include(../../../../../unittest/unittest.pri)

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

