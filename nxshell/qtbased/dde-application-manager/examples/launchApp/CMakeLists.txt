set(LAUNCHAPP_BIN launchApp)

add_executable(${LAUNCHAPP_BIN} main.cpp)

target_link_libraries(${LAUNCHAPP_BIN} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)
