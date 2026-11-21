# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(filemanager LANGUAGES C CXX)

# The sources use the files:: alias for Lingmo::Files.
add_compile_options(-include${CMAKE_CURRENT_SOURCE_DIR}/src/namespace_compat.h)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(LingmoFiles REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n)
pkg_check_modules(LibCurl REQUIRED IMPORTED_TARGET libcurl)

# ---------------------------------------------------------------------------
# lingmo-mountd: phones, servers and disk images mounted with FUSE, so their
# files are local paths to everything else.
# ---------------------------------------------------------------------------
pkg_check_modules(Fuse3 REQUIRED IMPORTED_TARGET fuse3)
pkg_check_modules(LibSsh REQUIRED IMPORTED_TARGET libssh)
pkg_check_modules(SmbClient REQUIRED IMPORTED_TARGET smbclient)
pkg_check_modules(LibMtp REQUIRED IMPORTED_TARGET libmtp)
pkg_check_modules(LibUdev REQUIRED IMPORTED_TARGET libudev)
# Disk image chunks: zlib, bzip2, LZMA and LZFSE.
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
pkg_check_modules(LibLzma REQUIRED IMPORTED_TARGET liblzma)
find_path(LZFSE_INCLUDE_DIR lzfse.h REQUIRED)
find_library(LZFSE_LIBRARY lzfse REQUIRED)

add_library(mountd-core STATIC
    mountd/diskimage.cpp
    mountd/diskimagebackend.cpp
    mountd/fusemount.cpp
    mountd/hfsplus.cpp
    mountd/keyring.cpp
    mountd/mountservice.cpp
    mountd/mtpbackend.cpp
    mountd/mtpmonitor.cpp
    mountd/remoteurl.cpp
    mountd/sftpbackend.cpp
    mountd/smbbackend.cpp
)
target_include_directories(mountd-core PUBLIC mountd)
target_include_directories(mountd-core PUBLIC ${LZFSE_INCLUDE_DIR})
target_link_libraries(mountd-core PUBLIC
    Lingmo::SDKBase
    PkgConfig::Fuse3
    PkgConfig::LibSsh
    PkgConfig::SmbClient
    PkgConfig::LibMtp
    PkgConfig::LibUdev
    PkgConfig::ICU
    ZLIB::ZLIB
    BZip2::BZip2
    PkgConfig::LibLzma
    ${LZFSE_LIBRARY}
)

add_executable(lingmo-mountd mountd/main.cpp)
target_link_libraries(lingmo-mountd PRIVATE mountd-core)

configure_file(mountd/org.lingmo.Mounts1.service.in org.lingmo.Mounts1.service @ONLY)

install(TARGETS lingmo-mountd RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.Mounts1.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

# ---------------------------------------------------------------------------
# lingmo-filemanager-daemon: file operations that outlive the windows,
# org.lingmo.FileOperations.
# ---------------------------------------------------------------------------
find_path(LIBZIP_INCLUDE_DIR zip.h)
find_library(LIBZIP_LIBRARY NAMES zip)
if(NOT LIBZIP_INCLUDE_DIR OR NOT LIBZIP_LIBRARY)
    message(FATAL_ERROR "libzip development files are required to build filemanager")
endif()
pkg_check_modules(LibArchive REQUIRED IMPORTED_TARGET libarchive)

add_library(fileoperations-core STATIC
    daemon/archivejob.cpp
    daemon/daemontask.cpp
    daemon/fileoperationsservice.cpp
)
target_include_directories(fileoperations-core PUBLIC ${PROJECT_SOURCE_DIR} daemon ${LIBZIP_INCLUDE_DIR})
target_link_libraries(fileoperations-core PUBLIC Lingmo::SDKBase Lingmo::Files ${LIBZIP_LIBRARY}
                      PkgConfig::LibArchive)

# Shared runtime provider: the notification centre loads these same widgets.
file(GLOB DESKTOP_WIDGET_SOURCES CONFIGURE_DEPENDS src/desktop/widgets/*.cpp)
add_library(lingmo-desktop-widgets SHARED ${DESKTOP_WIDGET_SOURCES} src/core/config.cpp)
target_include_directories(lingmo-desktop-widgets PRIVATE src)
target_compile_options(lingmo-desktop-widgets PRIVATE -include${CMAKE_CURRENT_SOURCE_DIR}/src/core/common.h)
target_link_libraries(lingmo-desktop-widgets PRIVATE Lingmo::SDK PkgConfig::ICU PkgConfig::LibCurl)
set_target_properties(lingmo-desktop-widgets PROPERTIES VERSION 1.0 SOVERSION 1 CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
install(TARGETS lingmo-desktop-widgets LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable(lingmo-filemanager-daemon daemon/main.cpp)
target_link_libraries(lingmo-filemanager-daemon PRIVATE fileoperations-core)

configure_file(daemon/org.lingmo.FileOperations.service.in org.lingmo.FileOperations.service @ONLY)
install(TARGETS lingmo-filemanager-daemon RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.FileOperations.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

# ---------------------------------------------------------------------------
# The file manager's models: listing, sorting, selection, file operations
# through the daemon. No UI beyond the clipboard.
# ---------------------------------------------------------------------------
add_library(fm-core STATIC
    src/core/config.cpp
    src/core/dates.cpp
    src/core/fileitem.cpp
    src/core/launcher.cpp
    src/core/lister.cpp
    src/core/notifications.cpp
    src/core/preferences.cpp
    src/core/previewkeys.cpp
    src/core/standardfolders.cpp
    src/core/mounts.cpp
    src/core/shortcutconvention.cpp
    src/core/thumbnails.cpp
    src/model/foldercontents.cpp
    src/model/foldermodel.cpp
    src/model/foldersizecache.cpp
    src/model/pathbarmodel.cpp
    src/model/pathhistory.cpp
    src/model/places.cpp
    src/model/positioner.cpp
    src/ops/operations.cpp
)
target_include_directories(fm-core PUBLIC src)
target_link_libraries(fm-core PUBLIC Lingmo::SDK Lingmo::Files Lingmo::FilesThumbnails)

# The browser window, its views and dialogs.
add_library(fm-ui STATIC
    src/views/colors.cpp
    src/views/completionpopup.cpp
    src/views/computerview.cpp
    src/views/foldermenu.cpp
    src/views/folderpage.cpp
    src/views/folderview.cpp
    src/views/folderwindow.cpp
    src/views/gridview.cpp
    src/views/icongridview.cpp
    src/views/listview.cpp
    src/views/pathbar.cpp
    src/views/previewclient.cpp
    src/views/renameeditor.cpp
    src/views/sidebar.cpp
    src/views/style.cpp
    src/dialogs/archivepassworddialog.cpp
    src/dialogs/archivepasswordprompt.cpp
    src/dialogs/framelessdialog.cpp
    src/dialogs/iotaskdialog.cpp
    src/dialogs/openwithdialog.cpp
    src/dialogs/propertiesdialog.cpp
    src/dialogs/serverdialogs.cpp
    src/dialogs/unlockdialog.cpp
    src/chooser/chooserwindow.cpp
    src/desktop/desktopmenuexporter.cpp
    src/desktop/desktopview.cpp
    src/desktop/desktopwindow.cpp
    src/desktop/widgets/calendarwidget.cpp
    src/desktop/widgets/batterywidget.cpp
    src/desktop/widgets/clockwidget.cpp
    src/desktop/widgets/nowplayingwidget.cpp
    src/desktop/widgets/noteswidget.cpp
    src/desktop/widgets/weatherwidget.cpp
    src/desktop/widgets/systemwidget.cpp
    src/desktop/widgets/worldclockwidget.cpp
    src/desktop/widgets/countdownwidget.cpp
    src/desktop/widgets/networkspeedwidget.cpp
    src/desktop/widgets/todowidget.cpp
    src/desktop/widgets/launcherwidget.cpp
    src/desktop/widgets/widget.cpp
    src/desktop/widgets/widgetconfig.cpp
    src/desktop/widgets/widgetgallery.cpp
    src/desktop/widgets/widgetlayer.cpp
    src/chooser/filechooser.cpp
    src/preview/imageinfo.cpp
    src/preview/imagepreview.cpp
    src/preview/previewcontroller.cpp
    src/preview/previewwindow.cpp
)
target_link_libraries(fm-ui PUBLIC fm-core PkgConfig::ICU PkgConfig::LibCurl)

add_executable(lingmo-filemanager
    src/app/application.cpp
    src/app/chooserservice.cpp
    src/app/dialogs.cpp
    src/app/main.cpp
    src/app/previewservice.cpp
)
target_link_libraries(lingmo-filemanager PRIVATE fm-ui)
install(TARGETS lingmo-filemanager RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
configure_file(data/org.lingmo.FileManager1.service.in org.lingmo.FileManager1.service @ONLY)
configure_file(data/org.lingmo.FilePreview.service.in org.lingmo.FilePreview.service @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.FileManager1.service
              ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.FilePreview.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
install(FILES data/60-lingmo-filemanager.rules DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/rules.d)

lingmo_add_translations(lingmo-filemanager SOURCES src daemon mountd
                          DESKTOP_FILES data/lingmo-filemanager.desktop data/computer.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-filemanager.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/computer.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
