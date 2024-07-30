add_definitions(-DTRANSLATION_DOMAIN=\"processcore\")

########### next target ###############

set(ksysguard_LIB_SRCS
   application_data_model.cpp application_data_model.h
   cgroup.cpp cgroup.h
   cgroup_data_model.cpp cgroup_data_model.h
   extended_process_list.cpp extended_process_list.h
   processes.cpp
   process.cpp
   process_attribute.cpp
   processes_local_p.cpp processes_local_p.h
   processes_remote_p.cpp processes_remote_p.h
   processes_base_p.cpp processes_base_p.h
   processes_atop_p.cpp processes_atop_p.h atop_p.h
   process_controller.cpp
   process_attribute_model.cpp
   process_data_model.cpp
   process_data_provider.cpp
   read_procsmaps_runnable.cpp read_procsmaps_runnable.h
)
ecm_qt_declare_logging_category(ksysguard_LIB_SRCS HEADER processcore_debug.h IDENTIFIER LIBKSYSGUARD_PROCESSCORE CATEGORY_NAME org.kde.libksysguard.processcore
    DESCRIPTION "libksysguard (processcore)" EXPORT LIBKSYSGUARD)

add_library(processcore ${ksysguard_LIB_SRCS})
add_library(KSysGuard::ProcessCore ALIAS processcore)

generate_export_header(processcore)

target_link_libraries(processcore
   PUBLIC
    Qt::Core
    KSysGuard::Formatter
   PRIVATE
    KF6::I18n
    KF6::AuthCore
    KF6::CoreAddons
    KF6::Service
    ZLIB::ZLIB
)

if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(processcore kvm)
endif()

if(NOT HAVE_CLOCK_GETTIME_C)
    target_link_libraries(processcore PRIVATE rt)
endif()

target_include_directories(processcore
    PUBLIC
        "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard>"
)
set_target_properties(processcore PROPERTIES
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${KSYSGUARD_SOVERSION}
    EXPORT_NAME ProcessCore
)
install(TARGETS processcore EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(declarative)
add_subdirectory(plugins)

########### install files ###############

install( FILES
    processes.h
    process.h
    process_controller.h
    process_attribute.h
    process_data_provider.h
    process_data_model.h
    process_attribute_model.h
    formatter.h
    unit.h
    ${CMAKE_CURRENT_BINARY_DIR}/processcore_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/processcore
    COMPONENT Devel
)

#------ KAuth stuff

if(ENABLE_KAUTH_HELPER)
    # Auth example helper
    set(ksysguardprocesslist_helper_srcs
        helper.cpp helper.h
        process.cpp
        processes_local_p.cpp processes_local_p.h
        processes_base_p.cpp processes_base_p.h
        read_procsmaps_runnable.cpp read_procsmaps_runnable.h
    )

    add_executable(ksysguardprocesslist_helper ${ksysguardprocesslist_helper_srcs})
    target_link_libraries(ksysguardprocesslist_helper Qt::Core KF6::AuthCore KF6::I18n)
    install(TARGETS ksysguardprocesslist_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

    kauth_install_helper_files(ksysguardprocesslist_helper org.kde.ksysguard.processlisthelper root)
    kauth_install_actions(org.kde.ksysguard.processlisthelper actions.actions)

    set_target_properties(ksysguardprocesslist_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")
endif()
