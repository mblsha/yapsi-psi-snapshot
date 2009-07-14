SPARKLE_FRAMEWORK = /Library/Frameworks/Sparkle.framework
exists($$SPARKLE_FRAMEWORK) {
	DEFINES += HAVE_SPARKLE
	QMAKE_LFLAGS += -framework Sparkle

	INCLUDEPATH += $$PWD
	DEPENDPATH  += $$PWD
	SOURCES += $$PWD/sparkle.m
	HEADERS += $$PWD/sparkle.h
}
