HEADERS += \
	$$PWD/lib/JsonQtExport.h \
	$$PWD/lib/JsonRpc.h \
	$$PWD/lib/JsonRpcAdaptor.h \
	$$PWD/lib/JsonRpcAdaptorPrivate.h \
	$$PWD/lib/JsonToProperties.h \
	$$PWD/lib/JsonToVariant.h \
	$$PWD/lib/ParseException.h \
	$$PWD/lib/VariantToJson.h

SOURCES += \
	$$PWD/lib/JsonRpc.cpp \
	$$PWD/lib/JsonRpcAdaptor.cpp \
	$$PWD/lib/JsonRpcAdaptorPrivate.cpp \
	$$PWD/lib/JsonToProperties.cpp \
	$$PWD/lib/JsonToVariant.cpp \
	$$PWD/lib/ParseException.cpp \
	$$PWD/lib/VariantToJson.cpp

INCLUDEPATH += $$PWD/lib
DEPENDPATH  += $$PWD/lib
