TEMPLATE = subdirs

SUBDIRS += \
    liblingmo-appwidget-plugin \
    liblingmo-appwidget-manager \
    liblingmo-appwidget-provider

#编译依赖顺序
liblingmo-appwidget-plugin.depends += liblingmo-appwidget-manager
unix {
    installPath = $$[QT_INSTALL_LIBS]/qt5/qml/org/lingmo/appwidget/
#    installPath = /usr/share/appwidget/qml/
    template.files = org.lingmo.appwidget/*
    template.path = $$installPath
    INSTALLS += template
}
