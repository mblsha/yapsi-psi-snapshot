TEMPLATE = app
CONFIG += qt debug
# CONFIG -= app_bundle
QT += gui
SOURCES += \
	tabbartestmain2.cpp \
	../yatabbarbase.cpp \
	../yatabbar.cpp \
	../yamultilinetabbar.cpp \
	../yatabwidget.cpp \
	../yaclosebutton.cpp \
#	../yavisualutil.cpp \
	$$PWD/../../yastyle.cpp \
	../yachevronbutton.cpp

HEADERS += \
	../yatabbarbase.h \
	../yatabbar.h \
	../yamultilinetabbar.h \
	../yatabwidget.h \
	../yaclosebutton.h \
#	../yavisualutil.h \
	$$PWD/../../yastyle.h \
	../overlaywidget.h \
	../yachevronbutton.h

DEFINES += WIDGET_PLUGIN
INCLUDEPATH += $$PWD/../.. $$PWD/..
DEPENDPATH  += $$PWD/../.. $$PWD/..

INCLUDEPATH += $$PWD/../private
DEPENDPATH  += $$PWD/../private

RESOURCES += tabbartest2.qrc

include(../stubs/stubs.pri)
include(../../../../../qa/oldtest/unittest.pri)

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

