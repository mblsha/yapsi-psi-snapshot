TEMPLATE = app
CONFIG += qt debug console
CONFIG -= app_bundle
QT += gui

SOURCES += \
	customwindowtestmain.cpp \
	../yawindow.cpp
# ../yavisualutil.cpp

HEADERS += \
	../yawindow.h \
	../overlaywidget.h
# ../yavisualutil.h \

INCLUDEPATH += ..
DEPENDPATH += ..

INTERFACES += mywindow.ui
RESOURCES += customwindowtest.qrc

# include(../stubs/stubs.pri)
include($$PWD/../../../advwidget/advwidget.pri)

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

