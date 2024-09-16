# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.10)
project(deepin-ocr)

#玲珑构建依赖目录
include_directories(${CMAKE_INSTALL_PREFIX}/include)

#for Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QtModule Core Gui Widgets DBus LinguistTools)

if(DOTEST)
    set(QtModule ${QtModule} Test)
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage -lgcov -fno-rtti")
    endif()
endif()

find_package(Qt5 REQUIRED ${QtModule})

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
)
add_definitions(${QT_DEFINITIONS})

include(CMakePackageConfigHelpers)

#for ocr plugin manager
#find_package(dtkocr)
#include_directories(${dtkocr_INCLUDE_DIRS})

#for OpenMP
find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

#for project files
include_directories(.)
include_directories(view)
include_directories(service)
include_directories(engine)

aux_source_directory(. allSource)
aux_source_directory(./view allSource)
aux_source_directory(./service allSource)
aux_source_directory(./engine allSource)

#translation
file(GLOB TargetTsFiles LIST_DIRECTORIES false ../translations/${PROJECT_NAME}*.ts)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)
if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(LangSrcs ${allSource} ${TargetTsFiles}
        OPTIONS )
endif()
qt5_add_translation(${TargetTsFiles})
set_source_files_properties(${qmfiles} PROPERTIES OUTPUT_LOCATION "translations")

#QRC file
set(AllQRC ../resource.qrc)

add_executable(${PROJECT_NAME} ${allSource} ${LangSrcs} ${AllQRC})

#for test
if(DOTEST)
    SET(PROJECT_NAME_TEST ${PROJECT_NAME}_test)

    include_directories(./../tests/)
    include_directories(./../3rdparty/stub_linux/)

    aux_source_directory(./../tests/ allTestSource)
    aux_source_directory(./view allTestSource)
    aux_source_directory(./service allTestSource)
    aux_source_directory(./paddleocr-ncnn allTestSource)
    aux_source_directory(../3rdparty/clipper allTestSource)

    FILE(GLOB allTestSource1
        "./frame.cpp"
        "./loadingwidget.cpp"
        "./ocrapplication.cpp"
        "./mainwidget.cpp"
        "./mainwindow.cpp"
        "./resulttextview.cpp"
        "./textloadwidget.cpp"
        )

    add_executable(${PROJECT_NAME_TEST} ${allHeaders} ${allTestSource} ${allTestSource1})
    target_link_libraries(${PROJECT_NAME_TEST} gmock gtest pthread)

    add_test(${PROJECT_NAME_TEST} COMMAND ${PROJECT_NAME_TEST})

#------------------------------ 创建'make test'指令---------------------------------------
    add_custom_target(test
        COMMAND mkdir -p coverageResult
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_custom_command(TARGET test
        COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "

        COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME_TEST}.dir --zerocounters
        COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
        COMMAND ./${PROJECT_NAME}/${PROJECT_NAME_TEST}

        COMMAND lcov --directory . --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info

        COMMAND echo " =================== do filter begin ==================== "
        COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info
        '*/${PROJECT_NAME_TEST}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*'
        -o ./coverageResult/${PROJECT_NAME}_Coverage.info
        COMMAND echo " =================== do filter end ==================== "

        COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info
        COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_dependencies(test ${PROJECT_NAME_TEST})
endif()

#for dtk
find_package(PkgConfig REQUIRED)
pkg_check_modules(dtk_lib REQUIRED dtkwidget dtkcore dtkocr)
target_include_directories(${PROJECT_NAME} PUBLIC ${dtk_lib_INCLUDE_DIRS})

if(DOTEST)
    target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${dtk_lib_INCLUDE_DIRS})
endif()

qt5_use_modules(${PROJECT_NAME} ${QtModule})

if(DOTEST)
    qt5_use_modules(${PROJECT_NAME_TEST} ${QtModule})
endif()

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets ${dtk_lib_LIBRARIES} pthread)

if(DOTEST)
    target_link_libraries(${PROJECT_NAME_TEST} Qt5::Core Qt5::Gui Qt5::Widgets ${dtk_lib_LIBRARIES} pthread)
endif()

#dde项目，期望CMAKE_INSTALL_PREFIX为/usr
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# install
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(TranslationDir ${PREFIX}/share/${PROJECT_NAME}/translations)
set(DesktopDir ${PREFIX}/share/applications/)
set(AppIconDir ${PREFIX}/share/icons/hicolor/scalable/apps)
set(DBusServiceDir ${PREFIX}/share/dbus-1/services)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${PREFIX}/lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION ${PREFIX}/bin
        INCLUDES DESTINATION allHeaders)

install(FILES ${LangSrcs} DESTINATION ${TranslationDir})
install(FILES ../deepin-ocr.desktop DESTINATION ${DesktopDir})
install(FILES ../assets/deepin-ocr.svg DESTINATION ${AppIconDir})
install(FILES ../com.deepin.Ocr.service DESTINATION ${DBusServiceDir})
