TEMPLATE = app
CONFIG += qt debug
QT += gui qt3support xml

# INCLUDEPATH += $$PWD/..
# DEPENDPATH  += $$PWD/..

DEFINES += WIDGET_PLUGIN

SOURCES += \
	statusbartestmain.cpp \
	testmodel.cpp \
	$$PWD/../yastatusbar.cpp \
	$$PWD/../yastatusbarlayout.cpp \
	$$PWD/../yainformerbutton.cpp \
	$$PWD/../yainformer.cpp \
	$$PWD/../yabusywidget.cpp

HEADERS += \
	testmodel.h \
	$$PWD/../yastatusbar.h \
	$$PWD/../yastatusbarlayout.h \
	$$PWD/../yainformerbutton.h \
	$$PWD/../yainformer.h \
	$$PWD/../yabusywidget.h

INTERFACES += \
	statusbartest.ui

include(../../../smoothscrollbar/smoothscrollbar.pri)
include(../stubs/stubs.pri)
DEFINES += NO_ICONSET_ZIP NO_ICONSET_SOUND YAPSI
# include(../../../iconset/iconset.pri)

RESOURCES += \
	$$PWD/statusbartest.qrc

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

