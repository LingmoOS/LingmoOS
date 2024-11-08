cmake_minimum_required(VERSION 3.16)
project(ernie-bot)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5
        COMPONENTS Core Gui Quick Widgets Network LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
        COMPONENTS Core Gui Quick Widgets Network LinguistTools REQUIRED)

find_package(lingmo-menu REQUIRED)

set(QRC_FILES "content/content.qrc")
# 翻译文件
file(GLOB TS_FILES "translations/*.ts")
# 更新翻译并创建.qm文件
qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})

set(SOURCE
        src/ernie-bot.h src/ernie-bot.cpp
        src/chat.h src/chat.cpp
        src/message.cpp src/message.h
        src/message-model.cpp src/message-model.h
        src/message-action.cpp src/message-action.h
        src/session-manager.cpp src/session-manager.h
        src/session.cpp src/session.h
        )

add_library(ernie-bot SHARED ${SOURCE} ${QM_FILES} ${QRC_FILES})
target_link_libraries(ernie-bot  PRIVATE
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Quick Qt5::Network
        lingmo-menu
        )

install(TARGETS ernie-bot LIBRARY DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-menu/extensions")
install(FILES   ${QM_FILES} DESTINATION "/usr/share/lingmo-menu/extensions/translations/")
if (BUILD_TEST)
    add_subdirectory(test)
endif ()
