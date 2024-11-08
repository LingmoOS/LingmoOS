QT += core gui widgets concurrent

include(../common.pri)

TARGET = explorer-admin
TEMPLATE = lib

DEFINES += PEONYADMIN_LIBRARY

PKGCONFIG += explorer
CONFIG += link_pkgconfig \
          c++11 \
	  plugin \
	  debug

#DESTDIR += ../testdir


PEONY_PATH = "/usr/bin/explorer"
PLUMA_PATH = "/usr/bin/pluma"
TERMINAL_PATH = "/usr/bin/mate-terminal"

policy.input = org.freedesktop.explorer-admin.policy.in
policy.output = org.freedesktop.explorer-admin.policy
policy.config = verbatim
QMAKE_SUBSTITUTES += policy

policy_file.files = org.freedesktop.explorer-admin.policy
policy_file.path = /usr/share/polkit-1/actions

SOURCES += admin-menu-plugin.cpp
HEADERS += admin-menu-plugin.h

TRANSLATIONS += translations/explorer-admin_cs.ts \
                translations/explorer-admin_tr.ts \
		translations/explorer-admin_zh_CN.ts

#RESOURCES += resources.qrc

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/explorer-extensions
INSTALLS += target policy_file
