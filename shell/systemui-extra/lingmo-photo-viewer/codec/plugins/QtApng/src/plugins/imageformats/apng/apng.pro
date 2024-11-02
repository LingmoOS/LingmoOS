TARGET  = qapng

HEADERS += \
	apngimageplugin.h \
	apngreader_p.h \
	apngimagehandler_p.h

SOURCES += \
	apngimageplugin.cpp \
	apngimagehandler.cpp \
	apngreader.cpp

OTHER_FILES += \
	qapng.json

system_libpng {
	CONFIG += link_pkgconfig
	PKGCONFIG += libpng
} else {
	QMAKE_USE_PRIVATE += apng_png apng_zlib
}

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = ApngImagePlugin
load(qt_plugin)
