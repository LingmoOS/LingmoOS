include_directories(${Inotify_INCLUDE_DIRS})
add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file6\")

set(file_static_lib_SRCS
    # File Indexer
    mainhub.cpp
    fileindexerconfig.cpp
    basicindexingjob.cpp
    powerstatemonitor.cpp

    fileindexscheduler.cpp

    firstrunindexer.cpp
    newfileindexer.cpp
    xattrindexer.cpp
    modifiedfileindexer.cpp
    unindexedfileindexer.cpp

    filecontentindexer.cpp
    filecontentindexerprovider.cpp
    extractorprocess.cpp
    extractor/commandpipe.cpp
    timeestimator.cpp

    indexcleaner.cpp

    # Common
    priority.cpp
    regexpcache.cpp
    fileexcludefilters.cpp
    storagedevices.cpp
    filtereddiriterator.cpp
    unindexedfileiterator.cpp
    migrator.cpp

     # File Watcher
    filewatch.cpp
    pendingfilequeue.cpp
    metadatamover.cpp
    org.kde.BalooWatcherApplication.xml
    pendingfile.cpp
    kinotify.cpp

    propertydata.cpp
)

ecm_qt_declare_logging_category(file_static_lib_SRCS
    HEADER baloodebug.h
    IDENTIFIER BALOO
    CATEGORY_NAME kf.baloo
)

# KF6 TODO - remove BalooWatcherApplication
qt_add_dbus_interface(file_static_lib_SRCS org.kde.BalooWatcherApplication.xml baloowatcherapplication_interface)
kconfig_add_kcfg_files(file_static_lib_SRCS ../lib/baloosettings.kcfgc GENERATE_MOC)

add_library(baloofilecommon STATIC ${file_static_lib_SRCS})
target_link_libraries(baloofilecommon
    Qt6::DBus
    KF6::I18n
    KF6::Solid
    KF6::FileMetaData
    KF6::ConfigCore
    KF6::BalooEngine
    ${Inotify_LIBRARIES}
)

set(file_SRCS
    main.cpp
)

if (BUILD_INDEXER_SERVICE)
    add_executable(baloo_file ${file_SRCS})
    ecm_mark_nongui_executable(baloo_file)
    target_compile_definitions(baloo_file PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

    target_link_libraries(baloo_file
        baloofilecommon
        KF6::Crash
    )

    install(TARGETS baloo_file DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR_KF})

    ecm_install_configured_files(INPUT baloo_file.desktop.in DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})

    ecm_install_configured_files(
       INPUT kde-baloo.service.in
       DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
    )

    add_subdirectory(extractor)

    install(FILES
        org.kde.BalooWatcherApplication.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    )
endif()

