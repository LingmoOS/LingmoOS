cmake_minimum_required(VERSION 3.13)
project(dpa-ext-gnome-keyring VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5DBus REQUIRED)

pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-unstable)
add_compile_definitions(SECRET_API_SUBJECT_TO_CHANGE)

pkg_check_modules(glib REQUIRED IMPORTED_TARGET glib-2.0)

add_compile_definitions(DPAEXTGNOMEKEYRING_LIBRARY)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
add_compile_definitions(QT_DEPRECATED_WARNINGS)

file(GLOB TS_FILES translations/dpa-ext-gnomekeyring*.ts)

qt5_add_translation(QM_FILES ${TS_FILES})

add_library(dpa-ext-gnomekeyring MODULE)

target_sources(dpa-ext-gnomekeyring PRIVATE
    gnomekeyringextention.cpp
    dpa-ext-gnomekeyring.json
    ${QM_FILES}
)

target_include_directories(dpa-ext-gnomekeyring PRIVATE
    /usr/include
    ${Qt5_INCLUDE_DIRS}
)

target_link_libraries(dpa-ext-gnomekeyring PRIVATE
    Qt5::Widgets
    PkgConfig::libsecret
    PkgConfig::glib
    ${Qt5DBus_LIBRARIES}
)

install(TARGETS
    dpa-ext-gnomekeyring
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/polkit-1-dde/plugins
)

install(FILES
    ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dpa-ext-gnomekeyring/translations
)
