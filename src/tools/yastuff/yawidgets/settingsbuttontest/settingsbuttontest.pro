TEMPLATE = app
CONFIG += qt debug
QT += gui

CONFIG -= app_bundle

SOURCES += \
	settingsbuttontestmain.cpp \
	yasettingsbutton.cpp \
	../yavisualutil.cpp

HEADERS += \
	yasettingsbutton.h \
	../overlaywidget.h

INTERFACES += \
	settingstest.ui

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

include($$PWD/../../../animationhelpers/animationhelpers.pri)
include($$PWD/../stubs/stubs.pri)

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
