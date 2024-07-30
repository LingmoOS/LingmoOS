# Configure checks for the caching subdir
include(CheckIncludeFiles)
check_include_files("sys/types.h;sys/mman.h" HAVE_SYS_MMAN_H)
configure_file(caching/config-caching.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-caching.h)

include(CheckSymbolExists)
check_symbol_exists("getgrouplist" "grp.h" HAVE_GETGROUPLIST)

if(UNIX)
    function(check_dladdr) # use a function to scope the variables!
        set(CMAKE_REQUIRED_FLAGS ${DLADDR_LINK_FLAGS})
        set(CMAKE_REQUIRED_LIBRARIES ${DLADDR_LINK_LIBRARIES})
        check_symbol_exists("dladdr" "dlfcn.h" HAVE_DLADDR)
        set(HAVE_DLADDR ${HAVE_DLADDR} PARENT_SCOPE)
    endfunction()

    if(CMAKE_SYSTEM_NAME MATCHES "Linux") # on linux dladdr isn't part of libc, on freebsd and osx it is
        set(DLADDR_LINK_FLAGS "-D_GNU_SOURCE")
        set(DLADDR_LINK_LIBRARIES "dl")
    endif()
    check_dladdr()

    add_feature_info(dladdr ${HAVE_DLADDR} "Can resolve shared library paths and by extension libexec paths at runtime using dladdr() API")
endif()

set(ACCOUNTS_SERVICE_ICON_DIR "/var/lib/AccountsService/icons" CACHE STRING "Accounts Services icon storage directory")

configure_file(util/config-util.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-util.h)

add_library(KF6CoreAddons)
add_library(KF6::CoreAddons ALIAS KF6CoreAddons)

set_target_properties(KF6CoreAddons PROPERTIES
    VERSION     ${KCOREADDONS_VERSION}
    SOVERSION   ${KCOREADDONS_SOVERSION}
    EXPORT_NAME CoreAddons
)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(KF6CoreAddons PUBLIC -DKCOREADDONS_STATIC)
endif()

ecm_create_qm_loader(KF6CoreAddons kcoreaddons6_qt)

if (Inotify_FOUND)
    target_include_directories(KF6CoreAddons PRIVATE ${Inotify_INCLUDE_DIRS})
    target_link_libraries(KF6CoreAddons PRIVATE ${Inotify_LIBRARIES})
endif ()

if(NOT WIN32)
    target_sources(KF6CoreAddons PRIVATE
        caching/kshareddatacache.cpp
        caching/kshareddatacache.h
        caching/ksdclock.cpp
        caching/ksdcmemory.cpp
    )

    set_source_files_properties(caching/kshareddatacache.cpp
        PROPERTIES COMPILE_FLAGS -fexceptions)

    target_link_libraries(KF6CoreAddons PRIVATE Threads::Threads)
else()
    target_sources(KF6CoreAddons PRIVATE
        caching/kshareddatacache_win.cpp
    )
endif()

if (WIN32)
    target_sources(KF6CoreAddons PRIVATE
        text/kmacroexpander_win.cpp
        util/klistopenfilesjob_win.cpp
        util/kprocesslist_win.cpp
        util/kshell_win.cpp
        util/kuser_win.cpp
    )
endif ()

if (UNIX)
    target_sources(KF6CoreAddons PRIVATE
        text/kmacroexpander_unix.cpp
        util/klistopenfilesjob_unix.cpp
        util/kuser_unix.cpp
        util/kshell_unix.cpp
    )
    if (HAVE_PROCSTAT)
        target_sources(KF6CoreAddons PRIVATE
            util/kprocesslist_unix_procstat.cpp
        )
        target_compile_definitions(KF6CoreAddons PRIVATE HAVE_PROCSTAT=1)
    else ()
        target_sources(KF6CoreAddons PRIVATE
            util/kprocesslist_unix.cpp
        )
    endif ()
endif ()

if(HAVE_QTDBUS)
    set(_dbus_SRCS)

    set_source_files_properties(io/org.freedesktop.portal.FileTransfer.xml PROPERTIES INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/io/dbustypes_p.h)
    qt_add_dbus_interface(_dbus_SRCS io/org.freedesktop.portal.FileTransfer.xml io/org.freedesktop.portal.FileTransfer)

    set_source_files_properties(io/org.kde.KIOFuse.VFS.xml PROPERTIES NO_NAMESPACE TRUE)
    qt_add_dbus_interface(_dbus_SRCS io/org.kde.KIOFuse.VFS.xml io/org.kde.KIOFuse.VFS)

    target_sources(KF6CoreAddons PRIVATE ${_dbus_SRCS})
    target_link_libraries(KF6CoreAddons PRIVATE Qt6::DBus)
endif()

if (TARGET UDev::UDev)
    target_link_libraries(KF6CoreAddons PRIVATE UDev::UDev)
endif()

target_sources(KF6CoreAddons PRIVATE
    licenses/licenses.qrc
    kaboutdata.cpp
    kcoreaddons.cpp
    io/kautosavefile.cpp
    io/kdirwatch.cpp
    io/kfilesystemtype.cpp
    io/kprocess.cpp
    io/kbackup.cpp
    io/kurlmimedata.cpp
    io/kfileutils.cpp
    io/knetworkmounts.cpp
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobtrackerinterface.cpp
    jobs/kjobuidelegate.cpp
    plugin/kpluginfactory.cpp
    plugin/kpluginmetadata.cpp
    plugin/kstaticpluginhelpers.cpp
    randomness/krandom.cpp
    text/kemoticonsparser.cpp
    text/kjsonutils.cpp
    text/kfuzzymatcher.cpp
    text/kmacroexpander.cpp
    text/kstringhandler.cpp
    text/ktexttohtml.cpp
    util/kformat.cpp
    util/kformatprivate.cpp
    util/kosrelease.cpp
    util/kprocesslist.cpp
    util/kshell.cpp
    util/klibexec.cpp
    util/ksignalhandler.cpp
    util/ksandbox.cpp
    util/kmemoryinfo.cpp
    util/kruntimeplatform.cpp

    kaboutdata.h
    kcoreaddons.h
    io/kautosavefile.h
    io/kdirwatch.h
    io/kfilesystemtype.h
    io/kprocess.h
    io/kbackup.h
    io/kurlmimedata.h
    io/kfileutils.h
    io/knetworkmounts.h
    jobs/kcompositejob.h
    jobs/kjob.h
    jobs/kjobtrackerinterface.h
    jobs/kjobuidelegate.h
    plugin/kpluginfactory.h
    plugin/kpluginmetadata.h
    randomness/krandom.h
    text/kjsonutils.h
    text/kfuzzymatcher.h
    text/kmacroexpander.h
    text/kstringhandler.h
    text/ktexttohtml.h
    util/kformat.h
    util/kosrelease.h
    util/kprocesslist.h
    util/kshell.h
    util/klibexec.h
    util/ksignalhandler.h
    util/ksandbox.h
    util/kmemoryinfo.h
    util/kruntimeplatform.h
)

if (ENABLE_PCH)
    target_precompile_headers(KF6CoreAddons PRIVATE
        <QObject>
        <QLoggingCategory>
        <QString>
    )
endif()


set(kcoreaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/caching/
    ${CMAKE_CURRENT_BINARY_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/
    ${CMAKE_CURRENT_SOURCE_DIR}/randomness/
    ${CMAKE_CURRENT_SOURCE_DIR}/text/
    ${CMAKE_CURRENT_SOURCE_DIR}/util/
)

ecm_qt_export_logging_category(
    IDENTIFIER KDIRWATCH
    CATEGORY_NAME kf.coreaddons.kdirwatch
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kdirwatch
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KDirWatch (KCoreAddons)"
    EXPORT KCOREADDONS
)
ecm_qt_export_logging_category(
    IDENTIFIER KABOUTDATA
    CATEGORY_NAME kf.coreaddons.kaboutdata
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kaboutdata
    DESCRIPTION "KAboutData (KCoreAddons)"
    EXPORT KCOREADDONS
)
ecm_qt_export_logging_category(
    IDENTIFIER LOG_KMEMORYINFO
    CATEGORY_NAME kf.coreaddons.kmemoryinfo
    DESCRIPTION "KMemoryInfo (KCoreAddons)"
    EXPORT KCOREADDONS
)

ecm_qt_declare_logging_category(KF6CoreAddons
    HEADER kcoreaddons_debug.h
    IDENTIFIER KCOREADDONS_DEBUG
    CATEGORY_NAME kf.coreaddons
    OLD_CATEGORY_NAMES org.kde.kcoreaddons
    DESCRIPTION "kcoreaddons (kcoreaddons lib)"
    EXPORT KCOREADDONS
)

ecm_generate_export_header(KF6CoreAddons
    BASE_NAME KCoreAddons
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6CoreAddons PUBLIC "$<BUILD_INTERFACE:${kcoreaddons_INCLUDE_DIRS}>")

target_link_libraries(KF6CoreAddons
  PUBLIC
    Qt6::Core
)

if(WIN32)
    target_link_libraries(KF6CoreAddons PRIVATE netapi32 userenv psapi)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    target_link_libraries(KF6CoreAddons PRIVATE kvm)
endif()

if (HAVE_PROCSTAT)
    target_link_libraries(KF6CoreAddons PRIVATE Procstat::Procstat)
endif()

if(HAVE_DLADDR)
    set_source_files_properties(SOURCE util/klibexec.cpp PROPERTIES COMPILE_FLAGS "${DLADDR_LINK_FLAGS}") # ensure _GNU_SOURCE on Linux
    target_link_libraries(KF6CoreAddons PRIVATE ${DLADDR_LINK_LIBRARIES})
endif()

target_include_directories(KF6CoreAddons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCoreAddons>")

target_compile_definitions(KF6CoreAddons INTERFACE "$<INSTALL_INTERFACE:KCOREADDONS_LIB>")

ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KAboutData,KAboutPerson,KAboutLicense,KAboutComponent
        KCoreAddons
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES KSharedDataCache
    RELATIVE caching
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KAutoSaveFile
        KDirWatch
        KProcess
        KBackup
        KUrlMimeData
        KFileSystemType
        KFileUtils
        KNetworkMounts
    RELATIVE io
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KCompositeJob
        KJob
        KJobTrackerInterface
        KJobUiDelegate
    RELATIVE jobs
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KPluginFactory
        KPluginMetaData
    RELATIVE plugin
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KRandom
    RELATIVE randomness
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KJsonUtils
        KFuzzyMatcher
        KMacroExpander
        KStringHandler
        KTextToHTML
    RELATIVE text
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KFormat
        KOSRelease
        KUser
        KShell
        KProcessList
        KListOpenFilesJob
        KLibexec
        KSignalHandler
        KRuntimePlatform
        KSandbox
        KMemoryInfo
    RELATIVE util
    REQUIRED_HEADERS KCoreAddons_HEADERS
)

install(TARGETS KF6CoreAddons EXPORT KF6CoreAddonsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${KCoreAddons_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCoreAddons COMPONENT Devel
)
install(FILES plugin/kpluginmetadata.schema.json DESTINATION ${KDE_INSTALL_DATADIR}/kf6/jsonschema COMPONENT Devel)

if(BUILD_QCH)
    ecm_add_qch(
        KF6CoreAddons_QCH
        NAME KCoreAddons
        BASE_NAME KF6CoreAddons
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCoreAddons_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
            ${kcoreaddons_INCLUDE_DIRS}
        BLANK_MACROS
            KCOREADDONS_EXPORT
            KCOREADDONS_DEPRECATED
            KCOREADDONS_DEPRECATED_EXPORT
            "KCOREADDONS_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
