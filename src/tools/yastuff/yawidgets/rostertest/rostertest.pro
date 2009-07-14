TEMPLATE = app
CONFIG += qt debug
QT += gui qt3support xml

# INCLUDEPATH += $$PWD/..
# DEPENDPATH  += $$PWD/..

include($$/rostertest.pri)

SOURCES += \
	rostertestmain.cpp

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

