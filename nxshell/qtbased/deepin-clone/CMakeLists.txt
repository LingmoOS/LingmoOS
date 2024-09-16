cmake_minimum_required(VERSION 3.7)

project(deepin-clone CXX)

set(HONEST_NAME deepin-clone-honest)
set(APP_NAME deepin-clone)
set(PLUGIN_NAME dfm-plugin-dim-file)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(APP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/app)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate policy file translations
    execute_process (
        COMMAND deepin-policy-ts-convert policy2ts com.deepin.pkexec.deepin-clone.policy.tmp translations
        WORKING_DIRECTORY ${APP_SOURCE_DIR}
    )

    # generate back to policy file
    execute_process (
        COMMAND deepin-policy-ts-convert ts2policy com.deepin.pkexec.deepin-clone.policy.tmp translations com.deepin.pkexec.deepin-clone.policy
        RESULT_VARIABLE POLICY_FILE_GENERATED
        WORKING_DIRECTORY ${APP_SOURCE_DIR}
    )

    # fallback
    if (NOT ${POLICY_FILE_GENERATED} EQUAL 0)
        execute_process(COMMAND cp com.deepin.pkexec.deepin-clone.policy.tmp com.deepin.pkexec.deepin-clone.policy WORKING_DIRECTORY ${APP_SOURCE_DIR})
    endif ()

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${APP_SOURCE_DIR})
    execute_process(COMMAND bash "translate_ts2desktop.sh"
                    WORKING_DIRECTORY ${APP_SOURCE_DIR})
else ()
    message("Building debug mode, translation files will not generated")
endif ()

# Install settings
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)

add_definitions(-DQT_MESSAGELOGCONTEXT)
add_definitions(-DHOST_ARCH_${CMAKE_SYSTEM_PROCESSOR})
add_definitions(-DHOST_ARCH="${CMAKE_SYSTEM_PROCESSOR}")

file(GLOB_RECURSE WIDGETS_SRCS
    "app/src/widgets/*.h"
    "app/src/widgets/*.cpp"
    "app/src/widgets/qss.qrc"
    "app/sources.qrc"
)

file(GLOB_RECURSE FIXBOOT_SRCS
    "app/src/fixboot/*.h"
    "app/src/fixboot/*.cpp"
    "app/src/fixboot/*.qrc"
)

file(GLOB_RECURSE CORELIB_SRCS
    "app/src/corelib/*.h"
    "app/src/corelib/*.cpp"
)

file(GLOB_RECURSE DFM_PLUGIN_SRCS
    "dfm_plugin/*.h"
    "dfm_plugin/*.cpp"
)

set(APP_SRCS
    app/src/main.cpp
    app/src/dglobal.h
    app/src/commandlineparser.cpp
    app/src/commandlineparser.h
    ${FIXBOOT_SRCS}
    ${CORELIB_SRCS}
)

set(APP_INCLUDE
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
)

set(APP_LIBRARY
    ${Qt5Core_LIBRARIES}
)

if(NOT (DEFINED DISABLE_GUI OR DEFINED DISABLE_DTK))
    add_definitions(-DDEEPIN_CLONE_UID="DEEPIN_CLONE_UID"
                    -DDEEPIN_CLONE_OPEN_DIALOG="DEEPIN_CLONE_OPEN_DIALOG"
                    -DDEEPIN_CLONE_OPEN_URL="DEEPIN_CLONE_OPEN_URL"
                    -DDEEPIN_CLONE_PARENT_WINID="DEEPIN_CLONE_PARENT_WINID"
                    -DDEEPIN_CLONE_TITLE="DEEPIN_CLONE_TITLE"
                    -DDEEPIN_CLONE_FILE_NAME="DEEPIN_CLONE_FILE_NAME"
                    -DDEEPIN_CLONE_NAME_FILTER="DEEPIN_CLONE_NAME_FILTER"
                    -DDEEPIN_CLONE_GET_FILE="DEEPIN_CLONE_GET_FILE"
                    -DDEEPIN_CLONE_SHOW_FILE="DEEPIN_CLONE_SHOW_FILE")
    add_definitions(-DENABLE_GUI)

    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    find_package(DtkWidget REQUIRED)

    #HONEST_NAME start
    add_executable(${HONEST_NAME}
        honest_child/main.cpp
    )

    target_include_directories(${HONEST_NAME} PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
    )

    target_link_libraries(${HONEST_NAME} PRIVATE
        ${DtkWidget_LIBRARIES}
        ${Qt5Core_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
    )
    install(TARGETS ${HONEST_NAME} DESTINATION bin)
    #HONEST_NAME end

    #APP_NAME start
    list(APPEND APP_SRCS
        ${WIDGETS_SRCS}
    )

    list(APPEND APP_INCLUDE
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
    )

    list(APPEND APP_LIBRARY
        ${DtkWidget_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
    )
    #APP_NAME end
else()
    add_definitions(-DDISABLE_DTK)
    add_definitions(-DDISABLE_GUI)
endif()

set(DISABLE_DFM_PLUGIN NO)

if(NOT DEFINED DISABLE_DFM_PLUGIN)
    message("**************************sfsf")
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    pkg_check_modules(DdeFileManagerInterface REQUIRED dde-file-manager)

    add_library(${PLUGIN_NAME} SHARED ${DFM_PLUGIN_SRCS} ${CORELIB_SRCS})

    target_include_directories(${PLUGIN_NAME} PUBLIC
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
        ${DdeFileManagerInterface_INCLUDE_DIRS}
    )

    target_link_libraries(${PLUGIN_NAME} PRIVATE
        ${Qt5Core_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
        ${DdeFileManagerInterface_LIBRARIES}
    )

    if(NOT DEFINED LIB_INSTALL_DIR)
        set(LIB_INSTALL_DIR lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
    endif()
    if(NOT DEFINED DFM_PLUGIN_DIR)
        set(DFM_PLUGIN_DIR ${LIB_INSTALL_DIR}/dde-file-manager/plugins/controllers)
    endif()

    install(TARGETS ${PLUGIN_NAME} DESTINATION ${DFM_PLUGIN_DIR})
endif()

add_executable(${APP_NAME}
    ${APP_SRCS}
)

target_include_directories(${APP_NAME} PUBLIC
    ${APP_INCLUDE}
)

target_link_libraries(${APP_NAME} PRIVATE
    ${APP_LIBRARY}
)

install(TARGETS ${APP_NAME} DESTINATION sbin)

#mimetype
install(FILES app/mimetype/deepin-clone.xml DESTINATION share/mime/packages)
install(FILES app/mimetype/application-x-deepinclone-dim.svg DESTINATION share/icons/hicolor/scalable/mimetypes)

#policy file
install(FILES app/com.deepin.pkexec.deepin-clone.policy DESTINATION share/polkit-1/actions)

#bin
install(FILES app/deepin-clone-ionice DESTINATION sbin)
install(FILES app/deepin-clone-pkexec DESTINATION bin)

#qm files
file(GLOB QM_FILES "app/translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/${APP_NAME}/translations)

#install(FILES app/deepin-clone.desktop DESTINATION share/applications)
