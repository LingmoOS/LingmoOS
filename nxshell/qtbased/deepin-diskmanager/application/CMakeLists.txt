cmake_minimum_required(VERSION 3.0)
project(deepin-diskmanager)

if (NOT DEFINED VERSION)
    message(WARNING "Not defined version ,about dialog version set 1.1.0.1")
    set(VERSION 1.3.14)
endif ()


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)#moc 预编译包含QObject文件，生成xx_moc.h cpp
set(CMAKE_AUTORCC ON)#开启rcc 编译资源文件qrc
set(CMAKE_AUTOUIC ON)#开启uic 编译转换*.ui文件生存设计界面对应的.h .cpp文件

# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
#find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Widgets
    DBus
    REQUIRED)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate

    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)

#common resource names
set(APP_RES_DIR "assets")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-diskmanager.desktop")
set(APP_QRC "${APP_RES_DIR}/appicons.qrc")
set(APP_ICONPATH "${APP_RES_DIR}/icons/deepin-diskmanager.svg")

configure_file(${APP_RES_DIR}/environments.h.in environments.h @ONLY)
#add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/translations")
#set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1 "${CMAKE_CURRENT_SOURCE_DIR}/translations")
#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/translations PROPERTY CLEAN_NO_CUSTOM  1)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1 )
#lupdate start
set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/translations")
set(TS_FILES
    "${TS_DIR}/${PROJECT_NAME}_en_US.ts"    
)
find_program(LUPDATE_EXECUTABLE lupdate)
foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})
endforeach()
#lupdate end

file (GLOB DTNG_TS_FILES translations/*.ts)
qt5_create_translation(DTNG_QM_FILES
    ${DTNG_TS_FILES}
     ${DTNG_QM_FILES}
     )

file(GLOB ALL_SOURCES "*.cpp" "./partedproxy/*.cpp" "./widgets/*.cpp" "./widgets/customcontrol/*.cpp" "./widgets/accessible/*.cpp")
file(GLOB ALL_HEADERS "*.h" "./partedproxy/*.h" "./widgets/*.h" "./widgets/customcontrol/*.h" "./widgets/accessible/*.h")

include_directories(../basestruct)
#link_directories(${PROJECT_BINARY_DIR})
set(ALL_SRCS   ${ALL_SOURCES} ${ALL_HEADERS})

# 生成执行文件
add_executable(${PROJECT_NAME} ${ALL_SRCS} ${APP_QRC}  ${DTNG_QM_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS} basestruct)

set(CMAKE_INSTALL_PREFIX /usr)

# Install files
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#file(GLOB APP_QM_FILES "translations/*.qm")
#install(FILES ${APP_QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)
install(FILES ${DTNG_QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)
install(FILES ${APP_ICONPATH} DESTINATION share/icons/hicolor/scalable/apps)
install(FILES ${APP_DESKTOP} DESTINATION share/applications)
install(DIRECTORY ${APP_RES_DIR}/deepin-diskmanager DESTINATION /usr/share/deepin-manual/manual-assets/application)




