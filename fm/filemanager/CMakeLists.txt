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

find_package(Qt6 REQUIRED COMPONENTS Core Gui DBus Quick LinguistTools)
# Look up each Framework directly: Debian packages ship component configs,
# rather than a monolithic KF6Config.cmake package.
find_package(KF6KIO REQUIRED)
find_package(KF6Solid REQUIRED)
find_package(KF6WindowSystem REQUIRED)
find_package(KF6Config REQUIRED)

add_library(lingmofilemanager_qmlplugins SHARED ${SCREEN_SRCS})

target_link_libraries (lingmofilemanager_qmlplugins
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::DBus
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    KF6::Solid
    KF6::WindowSystem
    KF6::ConfigCore
)

install(TARGETS lingmofilemanager_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Lingmo/FileManager)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Lingmo/FileManager)
