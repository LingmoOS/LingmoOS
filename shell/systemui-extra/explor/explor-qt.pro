TEMPLATE = subdirs

SUBDIRS = src libexplor-qt \ # plugin #libexplor-qt/test \ #plugin-iface
    #libexplor-qt/model/model-test \
    #libexplor-qt/file-operation/file-operation-test \
    #explor-qt-plugin-test \
    explor-qt-desktop \
    stable

CONFIG += debug_and_release
CONFIG(release,debug|release){
    DEFINES += QT_NO_DEBUG_OUTPUT
}

src.depends = libexplor-qt
explor-qt-plugin-test.depends = libexplor-qt
explor-qt-desktop.depends = libexplor-qt
stable.depends = libexplor-qt
