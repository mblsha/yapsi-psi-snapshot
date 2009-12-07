INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
# DEFINES += YAPSI_DEV
# DEFINES += DEFAULT_XMLCONSOLE
# DEFINES += YAPSI_NO_STYLESHEETS
# DEFINES += YAPSI_STRESSTEST_ACCOUNTS

include($$PWD/../slickwidgets/slickwidgets.pri)
include($$PWD/../smoothscrollbar/smoothscrollbar.pri)
include($$PWD/../animationhelpers/animationhelpers.pri)
include($$PWD/../cutejson/cutejson.pri)
include($$PWD/../../../third-party/JsonQt/jsonqt.pri)

# there's also a section in src.pro
yapsi_activex_server {
	DEFINES += YAPSI_ACTIVEX_SERVER

	HEADERS += \
		$$PWD/yapsiserver.h \
		$$PWD/yaonline.h \
		$$PWD/ycuapiwrapper.h \
		$$PWD/yaaddcontacthelper.h

	SOURCES += \
		$$PWD/yapsiserver.cpp \
		$$PWD/yaonline.cpp \
		$$PWD/ycuapiwrapper.cpp \
		$$PWD/yaaddcontacthelper.cpp
}

debug {
	MODELTEST_PRI = $$PWD/../../../../../modeltest/modeltest.pri
	exists($$MODELTEST_PRI) {
		include($$MODELTEST_PRI)
		DEFINES += MODELTEST
	}
}

HEADERS += \
	$$PWD/yaprofile.h \
	$$PWD/yarostertooltip.h \
	$$PWD/yachattooltip.h \
	$$PWD/yachattiplabel.h \
	$$PWD/yarostertiplabel.h \
	$$PWD/yamainwin.h \
	$$PWD/yaonlinemainwin.h \
	$$PWD/yaroster.h \
	$$PWD/yachatdlg.h \
	$$PWD/yatrayicon.h \
	$$PWD/yaeventnotifier.h \
	$$PWD/yatabbednotifier.h \
	$$PWD/yaloginpage.h \
	$$PWD/yacontactlistmodel.h \
	$$PWD/fakegroupcontact.h \
	$$PWD/yainformersmodel.h \
	$$PWD/yacontactlistcontactsmodel.h \
	$$PWD/yacommon.h \
	$$PWD/yastyle.h \
	$$PWD/yatoster.h \
	$$PWD/yapopupnotification.h \
	$$PWD/yaabout.h \
	$$PWD/yapreferences.h \
	$$PWD/yaprivacymanager.h \
	$$PWD/yaipc.h \
	$$PWD/yatoastercentral.h \
	$$PWD/yadayuse.h \
	$$PWD/yalicense.h \
	$$PWD/delayedvariablebase.h \
	$$PWD/delayedvariable.h \
	$$PWD/yahistorycachemanager.h \
	$$PWD/yaunreadmessagesmanager.h \
	$$PWD/yaexception.h \
	$$PWD/yadebugconsole.h

SOURCES += \
	$$PWD/yaprofile.cpp \
	$$PWD/yarostertooltip.cpp \
	$$PWD/yachattooltip.cpp \
	$$PWD/yachattiplabel.cpp \
	$$PWD/yarostertiplabel.cpp \
	$$PWD/yamainwin.cpp \
	$$PWD/yaonlinemainwin.cpp \
	$$PWD/yaroster.cpp \
	$$PWD/yachatdlg.cpp \
	$$PWD/yatrayicon.cpp \
	$$PWD/yaeventnotifier.cpp \
	$$PWD/yatabbednotifier.cpp \
	$$PWD/yaloginpage.cpp \
	$$PWD/yacontactlistmodel.cpp \
	$$PWD/fakegroupcontact.cpp \
	$$PWD/yainformersmodel.cpp \
	$$PWD/yacontactlistcontactsmodel.cpp \
	$$PWD/yacommon.cpp \
	$$PWD/yastyle.cpp \
	$$PWD/yatoster.cpp \
	$$PWD/yapopupnotification.cpp \
	$$PWD/yaabout.cpp \
	$$PWD/yapreferences.cpp \
	$$PWD/yaprivacymanager.cpp \
	$$PWD/yaipc.cpp \
	$$PWD/yatoastercentral.cpp \
	$$PWD/yadayuse.cpp \
	$$PWD/yalicense.cpp \
	$$PWD/delayedvariablebase.cpp \
	$$PWD/yahistorycachemanager.cpp \
	$$PWD/yaunreadmessagesmanager.cpp \
	$$PWD/yaexception.cpp \
	$$PWD/yadebugconsole.cpp

RESOURCES += \
	$$PWD/yastuff.qrc \
	$$PWD/yaiconsets.qrc

INTERFACES += \
	$$PWD/yaloginpage.ui \
	$$PWD/yamainwindow.ui \
	$$PWD/yachatdialog.ui \
	$$PWD/yarostertiplabel.ui \
	$$PWD/yaabout.ui \
	$$PWD/yapreferences.ui \
	$$PWD/yalicense.ui \
	$$PWD/yadebugconsole.ui

HEADERS -= \
	$$PWD/../../accountmodifydlg.h

SOURCES -= \
	$$PWD/../../accountmodifydlg.cpp

INTERFACES -= \
	$$PWD/../../accountmodify.ui
