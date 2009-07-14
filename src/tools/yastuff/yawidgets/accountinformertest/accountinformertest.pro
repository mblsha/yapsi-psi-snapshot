TEMPLATE = app
CONFIG += qt debug
CONFIG -= app_bundle
QT += gui xml qt3support network

SOURCES += \
	accountinformertestmain.cpp \
	testmodel.cpp \
	accountinformerdelegate.cpp \
	accountinformerview.cpp

HEADERS += \
	testmodel.h \
	accountinformerdelegate.h \
	accountinformerview.h

INTERFACES += \
	accountinformertest.ui

CONFIG += qca-static
SRC_DIR = $$PWD/../../../..
include($$SRC_DIR/src.pri)

INCLUDEPATH += $$SRC_DIR
DEPENDPATH  += $$SRC_DIR

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
