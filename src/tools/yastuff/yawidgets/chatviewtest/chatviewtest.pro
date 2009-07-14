TEMPLATE = app
CONFIG += qt debug
CONFIG -= app_bundle
QT += gui xml qt3support
SOURCES += \
	chatviewtestmain.cpp \
	../yachatview.cpp \
	../yachatviewdelegate.cpp \
	../yachatviewmodel.cpp \
	testmodel.cpp

HEADERS += \
	../yachatview.h \
	../yachatviewdelegate.h \
	../yachatviewmodel.h \
	testmodel.h

INCLUDEPATH += $$PWD $$PWD/.. $$PWD/../private $$PWD/../../../../widgets
DEPENDPATH  += $$PWD $$PWD/.. $$PWD/../private $$PWD/../../../../widgets

include(../../../iconset/iconset.pri)
include(../../../../widgets/widgets.pri)
include(../stubs/stubs.pri)
include(../../../smoothscrollbar/smoothscrollbar.pri)

DEFINES += \
	NO_SHORTCUTMANAGER NO_DESKTOPUTIL NO_YAHIGHLIGHTERS \
	NO_ICONSET_SOUND NO_ICONSET_ZIP

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

