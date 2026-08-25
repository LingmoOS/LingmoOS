cmake_minimum_required(VERSION 3.16)
project(lingmo-oobe VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(TS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/translations)
set(TS_FILE ${TS_DIR}/lingmo-oobe_zh_CN.ts)
set(QM_FILE ${TS_DIR}/lingmo-oobe_zh_CN.qm)

# Try LinguistTools for build-time .ts→.qm compilation
find_package(Qt6 COMPONENTS LinguistTools QUIET)
if(Qt6LinguistTools_FOUND)
    message(STATUS "Qt6 LinguistTools found, using qt6_add_translations")
    set(TS_FILES ${TS_FILE})
    add_executable(lingmo-oobe
        main.cpp
        OobeWindow.cpp
        resources.qrc
    )
    target_link_libraries(lingmo-oobe Qt6::Widgets)
    qt6_add_translations(lingmo-oobe
        TS_FILES ${TS_FILES}
        RESOURCE_PREFIX "/i18n"
    )
    qt6_add_resources(lingmo-oobe "oobe_resources"
        PREFIX "/"
        FILES logo.png
    )
else()
    message(STATUS "Qt6 LinguistTools not found, using lrelease at configure time")
    # Try to compile .ts→.qm at configure time
    find_program(LRELEASE
        NAMES lrelease-qt6 lrelease
        PATHS /usr/lib/qt6/bin /usr/lib/x86_64-linux-gnu/qt6/bin
    )
    if(LRELEASE AND EXISTS ${TS_FILE})
        execute_process(COMMAND ${LRELEASE} ${TS_FILE} -qm ${QM_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE LRELEASE_RESULT
        )
        if(LRELEASE_RESULT EQUAL 0)
            message(STATUS "Compiled translations: lingmo-oobe_zh_CN.ts → .qm")
        else()
            message(WARNING "lrelease failed to compile .ts file")
        endif()
    endif()

    add_executable(lingmo-oobe
        main.cpp
        OobeWindow.cpp
        resources.qrc
    )
    target_link_libraries(lingmo-oobe Qt6::Widgets)

    qt6_add_resources(lingmo-oobe "oobe_resources"
        PREFIX "/"
        FILES logo.png
    )

    # Embed .qm if available
    if(EXISTS ${QM_FILE})
        set_source_files_properties(${QM_FILE} PROPERTIES
            QT_RESOURCE_ALIAS "lingmo-oobe_zh_CN.qm"
        )
        qt6_add_resources(lingmo-oobe "oobe_translations"
            PREFIX "/i18n"
            FILES ${QM_FILE}
        )
    else()
        message(STATUS "No .qm file found -- building without translations")
    endif()
endif()

install(TARGETS lingmo-oobe RUNTIME DESTINATION bin)
install(FILES oobe.desktop DESTINATION /etc/xdg/autostart)
install(FILES firstboot.service DESTINATION /lib/systemd/system)
install(PROGRAMS oobe.sh DESTINATION /usr/lib/lingmo-oobe)
install(FILES xsessions/lingmo-oobe.desktop DESTINATION /usr/share/xsessions)
install(FILES wayland-sessions/lingmo-oobe.desktop DESTINATION /usr/share/wayland-sessions)
install(FILES lingmo-oobe.rules DESTINATION /etc/polkit-1/rules.d RENAME 10-lingmo-oobe.rules)
