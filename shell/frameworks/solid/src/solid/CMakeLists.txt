ecm_create_qm_loader(solid_QM_LOADER solid6_qt)

include(devices/CMakeLists.txt)

if (WITH_NEW_SOLID_JOB)
    include(power/CMakeLists.txt)
endif()

set(solid_LIB_SRCS ${solid_LIB_SRCS} ${solid_QM_LOADER})

add_library(KF6Solid  ${solid_LIB_SRCS})
set(solid_BUILD_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
target_include_directories(KF6Solid PUBLIC "$<BUILD_INTERFACE:${solid_BUILD_INCLUDE_DIRS}>")

ecm_generate_export_header(KF6Solid
    BASE_NAME Solid
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

add_library(KF6::Solid ALIAS KF6Solid)

# Apps must include <Solid/File> or <solid/file.h>
target_include_directories(KF6Solid INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Solid>")

target_link_libraries(KF6Solid PUBLIC Qt6::Core
                               PRIVATE Qt6::Xml
                                       Qt6::Gui
                                       ${solid_OPTIONAL_LIBS}
 )
if (HAVE_DBUS)
  target_link_libraries(KF6Solid PRIVATE Qt6::DBus)
endif()

set_target_properties(KF6Solid PROPERTIES  VERSION ${SOLID_VERSION}
                                           SOVERSION ${SOLID_SOVERSION}
                                           EXPORT_NAME Solid
)

ecm_generate_headers(Solid_CamelCase_HEADERS
  HEADER_NAMES
  Device
  DeviceNotifier
  DeviceInterface
  GenericInterface
  Processor
  Block
  StorageAccess
  StorageDrive
  OpticalDrive
  StorageVolume
  OpticalDisc
  Camera
  PortableMediaPlayer
  Battery
  Predicate
  NetworkShare
  SolidNamespace

  RELATIVE devices/frontend
  REQUIRED_HEADERS Solid_HEADERS
  PREFIX Solid
)

if (WITH_NEW_SOLID_JOB)
    ecm_generate_headers(Solid_CamelCase_HEADERS
    HEADER_NAMES
    Job

    RELATIVE power/
    REQUIRED_HEADERS Solid_HEADERS
    PREFIX Solid
    )
endif()

if(WITH_NEW_SOLID_JOB AND WITH_NEW_POWER_ASYNC_API)
    ecm_generate_headers(Solid_CamelCase_HEADERS
        HEADER_NAMES
        AcPluggedJob
        InhibitionJob
        Inhibition
        Power
        StatesJob
        RequestStateJob

        RELATIVE power/
        REQUIRED_HEADERS Solid_HEADERS
        PREFIX Solid
    )
endif()

install(FILES ${Solid_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Solid/Solid COMPONENT Devel)

install(TARGETS KF6Solid EXPORT KF6SolidTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### static lib for tests  ###############

if (BUILD_TESTING)
  add_library(KF6Solid_static STATIC ${solid_LIB_SRCS})
  set_target_properties(KF6Solid_static PROPERTIES COMPILE_FLAGS -DSOLID_STATIC_DEFINE=1)

  target_link_libraries(KF6Solid_static PUBLIC Qt6::Core)
  target_link_libraries(KF6Solid_static PRIVATE Qt6::Xml Qt6::Gui ${solid_OPTIONAL_LIBS})
  if (HAVE_DBUS)
    target_link_libraries(KF6Solid_static PRIVATE Qt6::DBus)
  endif()
  target_include_directories(KF6Solid_static PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")
endif()

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/solid_export.h
  ${Solid_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Solid/solid COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Solid_QCH
        NAME Solid
        BASE_NAME KF6Solid
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Solid_HEADERS}
            "${CMAKE_SOURCE_DIR}/docs/song.md"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${solid_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            SOLID_EXPORT
            SOLID_DEPRECATED
            SOLID_DEPRECATED_EXPORT
            "SOLID_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

