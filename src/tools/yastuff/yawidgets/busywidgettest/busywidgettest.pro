TEMPLATE = app
CONFIG += qt
QT += gui

SOURCES += \
	busywidgettestmain.cpp \
	$$PWD/../yabusywidget.cpp \
	yabusywidget2.cpp

HEADERS += \
	$$PWD/../yabusywidget.h \
	yabusywidget2.h

INCLUDEPATH += $$PWD/..
DEPENDPATH  += $$PWD/..

INTERFACES += busywidgettest.ui
RESOURCES += busywidgettest.qrc

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

