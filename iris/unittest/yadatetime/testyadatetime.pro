CONFIG += unittest
include($$PWD/../../../qa/oldtest/unittest.pri)

INCLUDEPATH += $$PWD/../../include
DEPENDPATH  += $$PWD/../../include

SOURCES += \
	$$PWD/testyadatetime.cpp \
	$$PWD/../../xmpp-core/xmpp_yadatetime.cpp
