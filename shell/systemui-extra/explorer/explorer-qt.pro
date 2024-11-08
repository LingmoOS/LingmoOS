TEMPLATE = subdirs

SUBDIRS = src libexplorer-qt \ # plugin #libexplorer-qt/test \ #plugin-iface
    #libexplorer-qt/model/model-test \
    #libexplorer-qt/file-operation/file-operation-test \
    #explorer-qt-plugin-test \
    explorer-qt-desktop \
    stable

CONFIG += debug_and_release
CONFIG(release,debug|release){
    DEFINES += QT_NO_DEBUG_OUTPUT
}

src.depends = libexplorer-qt
explorer-qt-plugin-test.depends = libexplorer-qt
explorer-qt-desktop.depends = libexplorer-qt
stable.depends = libexplorer-qt
