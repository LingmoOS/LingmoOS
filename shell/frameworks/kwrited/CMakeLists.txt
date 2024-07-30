cmake_minimum_required(VERSION 3.16)

project(kwrited)
set(PROJECT_VERSION "6.1.80")

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

include(CMakeDependentOption)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Gui
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Pty
    Notifications
)

cmake_dependent_option(BUILD_AS_EXECUTABLE "Whether to build kwrited as executable" OFF
                       "KPty_HAVE_UTEMPTER" ON)

if (NOT BUILD_AS_EXECUTABLE)
find_package(KF6DBusAddons ${KF6_MIN_VERSION})
set_package_properties(KF6DBusAddons PROPERTIES DESCRIPTION "Convenience classes for DBus"
                       TYPE REQUIRED
                       PURPOSE "Needed to build kwrited's KDED module"
                      )
endif ()

add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

ecm_set_disabled_deprecation_versions(QT 6.4.0
    KF 5.101
)

# If libutempter is not available on the system, we build
# kwrited as an executable that is installed with setuid root
# (or setgid utmp) in order to be able to login successfully
# (and thus be useful, as kwrited does not work at all if it
# cannot login to the terminal device that it opens).

configure_file(config-kwrited.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwrited.h)

if (BUILD_AS_EXECUTABLE)
    add_executable(kwrited kwrited.cpp)
    target_compile_definitions(kwrited PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
    target_link_libraries(kwrited Qt::Gui KF6::CoreAddons KF6::I18n KF6::Notifications KF6::Pty)
    install(TARGETS kwrited  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
    if(IS_ABSOLUTE BIN_INSTALL_DIR)
        set(ABS_BIN_INSTALL_DIR "${KDE_INSTALL_BINDIR}")
    else()
        set(ABS_BIN_INSTALL_DIR "\${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_BINDIR}")
    endif()
    install(CODE "
               set(KWRITED_PATH \"\$ENV{DESTDIR}${ABS_BIN_INSTALL_DIR}/kwrited\")

               # On Debian we setgid instead of setuid.
               # Feel free to add checks here for other distros as well, if needed.
               if (EXISTS /etc/debian_version)
                   set(SETGID 1)
                   set(SETGID_GROUP utmp)
               endif ()

               if (SETGID)
                   execute_process(COMMAND sh -c \"chgrp \${SETGID_GROUP} '\${KWRITED_PATH}' && chmod g+s '\${KWRITED_PATH}'\")
               else ()
                   execute_process(COMMAND sh -c \"chown root '\${KWRITED_PATH}' && chmod u+s '\${KWRITED_PATH}'\")
               endif()
           ")

    install(FILES kwrited-autostart.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})
else ()
    kcoreaddons_add_plugin(kded_kwrited
        SOURCES kwrited.cpp
        INSTALL_NAMESPACE "kf6/kded"
    )
    set_target_properties(kded_kwrited PROPERTIES OUTPUT_NAME kwrited)
    target_compile_definitions(kded_kwrited PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
    target_link_libraries(kded_kwrited KF6::CoreAddons KF6::DBusAddons KF6::I18n KF6::Notifications KF6::Pty)
endif ()

# The .notifyrc file is common
install(FILES kwrited.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
