TEMPLATE = app
CONFIG += qt debug
# CONFIG -= app_bundle
QT += gui
SOURCES += \
	tabbartestmain.cpp \
	../yatabbar.cpp \
	../yatabwidget.cpp \
	../yaclosebutton.cpp \
#	../yavisualutil.cpp \
	$$PWD/../../yastyle.cpp \
	../yachevronbutton.cpp

HEADERS += \
	../yatabbar.h \
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

RESOURCES += tabbartest.qrc

include(../stubs/stubs.pri)

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

