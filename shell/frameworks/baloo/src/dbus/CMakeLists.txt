# Cannot use INTERFACE libraries, as cmake (at least 3.25) seems
# to not add dependencies for generated sources that are added as INTERFACE sources.
# So using STATIC libraries instead.

#
# MainHub
#
add_library(BalooDBusMainInterface STATIC)
set_target_properties(BalooDBusMainInterface PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(mainhub_xml org.kde.baloo.main.xml)

qt_generate_dbus_interface(
    ${CMAKE_SOURCE_DIR}/src/file/mainhub.h
    ${mainhub_xml}
    OPTIONS -a
)

qt_add_dbus_interface(maininterface_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${mainhub_xml}
    maininterface
)

target_sources(BalooDBusMainInterface PRIVATE ${maininterface_SRCS})
target_link_libraries(BalooDBusMainInterface PUBLIC Qt6::DBus)

#
# FileIndexScheduler
#
add_library(BalooDBusSchedulerInterface STATIC)
set_target_properties(BalooDBusSchedulerInterface PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(scheduler_xml org.kde.baloo.scheduler.xml)

qt_generate_dbus_interface(
    ${CMAKE_SOURCE_DIR}/src/file/fileindexscheduler.h
    ${scheduler_xml}
    OPTIONS -a
)

qt_add_dbus_interface(schedulerinterface_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${scheduler_xml}
    schedulerinterface
)

target_sources(BalooDBusSchedulerInterface PRIVATE ${schedulerinterface_SRCS})
target_link_libraries(BalooDBusSchedulerInterface PUBLIC Qt6::DBus)

#
# FileContentIndexer
#
add_library(BalooDBusFileIndexerInterface STATIC)
set_target_properties(BalooDBusFileIndexerInterface PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(contentindexer_xml org.kde.baloo.fileindexer.xml)

qt_generate_dbus_interface(
    ${CMAKE_SOURCE_DIR}/src/file/filecontentindexer.h
    ${contentindexer_xml}
    OPTIONS -a
)

qt_add_dbus_interface(fileindexerinterface_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${contentindexer_xml}
    fileindexerinterface
)

target_sources(BalooDBusFileIndexerInterface PRIVATE ${fileindexerinterface_SRCS})
target_link_libraries(BalooDBusFileIndexerInterface PUBLIC Qt6::DBus)


if (BUILD_INDEXER_SERVICE)
    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${mainhub_xml}
            ${CMAKE_CURRENT_BINARY_DIR}/${scheduler_xml}
            ${CMAKE_CURRENT_BINARY_DIR}/${contentindexer_xml}
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    )

    add_subdirectory(fake)
endif()

