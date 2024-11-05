  TEMPLATE        = lib
  CONFIG         += plugin
  QT             += widgets
  INCLUDEPATH    += ../plugin-iface
  HEADERS         = menuplugin.h \
    menu-iface.h
  SOURCES         = menuplugin.cpp
  TARGET          = $$qtLibraryTarget(menutest1)
  DESTDIR         = ../testdir

  EXAMPLE_FILES = menuplugintest1.json

  # install
  #target.path = ../testdir
  #INSTALLS += target

  CONFIG += install_ok  # Do not cargo-cult this!

DISTFILES +=
