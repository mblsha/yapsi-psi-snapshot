TEMPLATE = lib
CONFIG  += designer plugin debug
# QT      += qt3support
TARGET   = yawidgets

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS   += target

DEFINES += WIDGET_PLUGIN

SOURCES += yawidgets.cpp

PSI_CPP = $$PWD/../../..
include($$PSI_CPP/../unittest/unittest.pri)

include(yawidgets.pri)
include(stubs/stubs.pri)
