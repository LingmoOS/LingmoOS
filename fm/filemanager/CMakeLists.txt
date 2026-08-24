set(SCREEN_SRCS
    window.cpp
    desktopiconprovider.cpp
    model/foldermodel.cpp
    model/placesmodel.cpp
    model/placesitem.cpp
    model/pathbarmodel.cpp
    model/dirlister.cpp
    model/positioner.cpp
    widgets/rubberband.cpp
    widgets/itemviewadapter.cpp
    helper/datehelper.cpp
    helper/pathhistory.cpp
    helper/fm.cpp
    helper/shortcut.cpp
    helper/filelauncher.cpp
    helper/keyboardsearchmanager.cpp
    mimetype/mimeappmanager.cpp
    mimetype/xdgdesktopfile.cpp
    dialogs/createfolderdialog.cpp
    dialogs/filepropertiesdialog.cpp
    dialogs/openwithdialog.cpp
    dialogs/deletedialog.cpp
    cio/cfilejob.cpp
    cio/cfilesizejob.cpp
    plugin.cpp
)

find_package(Qt5 COMPONENTS Core DBus Quick LinguistTools REQUIRED)
find_package(KF5KIO)
find_package(KF5Solid)
find_package(KF5WindowSystem)
find_package(KF5Config)

add_library(lingmofilemanager_qmlplugins SHARED ${SCREEN_SRCS})

target_link_libraries (lingmofilemanager_qmlplugins
    Qt5::Core
    Qt5::Quick
    Qt5::Gui
    Qt5::DBus
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::Solid
    KF5::WindowSystem
    KF5::ConfigCore
)

install(TARGETS lingmofilemanager_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Lingmo/FileManager)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Lingmo/FileManager)
