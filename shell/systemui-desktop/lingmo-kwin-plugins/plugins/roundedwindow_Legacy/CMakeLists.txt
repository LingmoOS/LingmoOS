set(CMAKE_CXX_STANDARD 20)

find_package(KF5CoreAddons)
find_package(KF5Config)
find_package(KF5WindowSystem)

find_path(EFFECTS_H kwineffects.h PATH_SUFFIXES kf5)

if (EFFECTS_H)
    include_directories(${EFFECTS_H})
else (EFFECTS_H)
    message(STATUS "didnt find kwineffects.h, not building effects")
endif (EFFECTS_H)

find_library(KWIN_EFFECTS NAMES kwineffects PATH_SUFFIXES kf5)
find_library(KWIN_GLUTILS NAMES kwinglutils PATH_SUFFIXES kf5)
find_library(OPENGL NAMES GL)

if (NOT KWIN_EFFECTS)
    message(STATUS "didnt find kwineffects lib, not building effects")
endif (NOT KWIN_EFFECTS)

if (NOT KWIN_GLUTILS)
    message(STATUS "didnt find kwin glutils lib, not building effects")
endif (NOT KWIN_GLUTILS)

if (NOT OPENGL)
    message(STATUS "didnt find opengl, not building effects")
endif (NOT OPENGL)

if (NOT EFFECTS_H OR NOT KWIN_GLUTILS OR NOT KWIN_EFFECTS OR NOT OPENGL)
    message(FATAL_ERROR "cant continue")
endif (NOT EFFECTS_H OR NOT KWIN_GLUTILS OR NOT KWIN_EFFECTS OR NOT OPENGL)

add_library(roundedwindow MODULE
    main.cpp
    roundedwindow.cpp
    resources.qrc
)

target_include_directories(roundedwindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(roundedwindow
    PUBLIC
        Qt5::Core
        Qt5::Gui
    PRIVATE
        KF5::CoreAddons
        KF5::ConfigCore
        KF5::WindowSystem
)

install (TARGETS roundedwindow DESTINATION ${QT_PLUGINS_DIR}/kwin/effects/plugins)
