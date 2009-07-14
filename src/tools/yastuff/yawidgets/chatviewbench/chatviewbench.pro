TEMPLATE = app
CONFIG += qt release
CONFIG -= app_bundle
QT += gui xml qt3support
SOURCES += \
	chatviewbenchmain.cpp \
	../yachatview.cpp \
	../yachatviewdelegate.cpp \
	../yachatviewmodel.cpp \
	$$PWD/../../../../pixmaputil.cpp

HEADERS += \
	../yachatview.h \
	../yachatviewdelegate.h \
	../yachatviewmodel.h

INCLUDEPATH += $$PWD $$PWD/.. $$PWD/../private $$PWD/../../../..
DEPENDPATH  += $$PWD $$PWD/.. $$PWD/../private $$PWD/../../../..

include(../../../smoothscrollbar/smoothscrollbar.pri)
include(../../../iconset/iconset.pri)
include(../../../../widgets/widgets.pri)

DEFINES += NO_SHORTCUTMANAGER NO_ICONSET_ZIP NO_ICONSET_SOUND

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
