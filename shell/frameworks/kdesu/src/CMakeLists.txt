add_subdirectory(kdesud)

include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckFunctionExists)

check_function_exists(setpriority HAVE_SETPRIORITY)
check_function_exists(getpeereid HAVE_GETPEEREID)
check_function_exists(initgroups HAVE_INITGROUPS)

check_include_files(sys/select.h  HAVE_SYS_SELECT_H)

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
check_type_size("struct ucred" STRUCT_UCRED) #defines HAVE_STRUCT_UCRED (bool) and STRUCT_UCRED (size, unused)
set(CMAKE_EXTRA_INCLUDE_FILES)  #reset CMAKE_EXTRA_INCLUDE_FILES

configure_file(config-kdesu.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdesu.h)

add_library(KF6Su)
add_library(KF6::Su ALIAS KF6Su)

set_target_properties(KF6Su PROPERTIES
    VERSION     ${KDESU_VERSION}
    SOVERSION   ${KDESU_SOVERSION}
    EXPORT_NAME Su
)

target_sources(KF6Su PRIVATE
  client.cpp
  ptyprocess.cpp
  kcookie.cpp
  suprocess.cpp
  sshprocess.cpp
  stubprocess.cpp
)

ecm_qt_declare_logging_category(KF6Su
    HEADER ksu_debug.h
    IDENTIFIER KSU_LOG
    CATEGORY_NAME kf.su
    DESCRIPTION "KSu (KDESu)"
    EXPORT KSU
)

ecm_generate_export_header(KF6Su
    EXPORT_FILE_NAME kdesu/kdesu_export.h
    BASE_NAME KDESu
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

if(KDESU_USE_SUDO_DEFAULT)
  target_compile_definitions(KF6Su PRIVATE -DKDESU_USE_SUDO_DEFAULT="true")
elseif(KDESU_USE_DOAS_DEFAULT)
  target_compile_definitions(KF6Su PRIVATE -DKDESU_USE_DOAS_DEFAULT="true")
endif()

target_include_directories(KF6Su PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

# Apps must include <KDESu/File> or <kdesu/file.h>
target_include_directories(KF6Su INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KDESu>")

target_link_libraries(KF6Su
  PUBLIC
    KF6::Pty
  PRIVATE
    KF6::CoreAddons # KUser::loginName
    KF6::ConfigCore # KConfigGroup
)


ecm_generate_headers(KDESu_CamelCase_HEADERS
  HEADER_NAMES
  PtyProcess
  SuProcess
  SshProcess
  StubProcess
  Client

  PREFIX KDESu
  REQUIRED_HEADERS KDESu_HEADERS
)
install(FILES ${KDESu_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDESu/KDESu COMPONENT Devel)

install(TARGETS KF6Su EXPORT KF6SuTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

include(ECMMarkNonGuiExecutable)

set_source_files_properties(kdesu_stub.c PROPERTIES
                            COMPILE_DEFINITIONS "_XOPEN_SOURCE;_BSD_SOURCE")
add_executable(kdesu_stub kdesu_stub.c)
ecm_mark_nongui_executable(kdesu_stub)

install(TARGETS kdesu_stub DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})

########### install files ###############
install(FILES
  defaults.h
  ${CMAKE_CURRENT_BINARY_DIR}/kdesu/kdesu_export.h
  ${KDESu_HEADERS}
  DESTINATION  ${KDE_INSTALL_INCLUDEDIR_KF}/KDESu/kdesu COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Su_QCH
        NAME KDESu
        BASE_NAME KF6Su
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KDESu_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            KF6Pty_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KDESU_EXPORT
            KDESU_DEPRECATED
            KDESU_DEPRECATED_EXPORT
            "KDESU_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

ecm_qt_install_logging_categories(
    EXPORT KSU
    FILE ksu.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
