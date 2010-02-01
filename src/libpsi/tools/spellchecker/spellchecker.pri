INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

HEADERS += \
	$$PWD/spellchecker.h \
	$$PWD/yandexspeller.h \
	$$PWD/spellcheckingtextedit.h

SOURCES += \
	$$PWD/spellchecker.cpp \
	$$PWD/yandexspeller.cpp \
	$$PWD/spellcheckingtextedit.cpp

mac {
	HEADERS += \
		$$PWD/macspellchecker.h \
		$$PWD/privateqt_mac.h
	SOURCES += \
		$$PWD/macspellchecker.mm \
		$$PWD/privateqt_mac.mm
}

!mac:contains(DEFINES, HAVE_ASPELL) {
	HEADERS += $$PWD/aspellchecker.h
	SOURCES += $$PWD/aspellchecker.cpp
}

!mac:contains(DEFINES, HAVE_ENCHANT) {
	HEADERS += $$PWD/enchantchecker.h
	SOURCES += $$PWD/enchantchecker.cpp
}
