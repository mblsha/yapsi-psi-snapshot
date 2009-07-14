TEMPLATE = app
CONFIG += qt
QT += gui
SOURCES += \
	$$PWD/animatedstackedwidgettestmain.cpp \
	$$PWD/animatedstackedwidget.cpp \
	$$PWD/baseanimatedstackedwidget.cpp \

HEADERS += \
	$$PWD/animatedstackedwidget.h \
	$$PWD/baseanimatedstackedwidget.h

INTERFACES += \
	animatedstackedwidgettest.ui

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

