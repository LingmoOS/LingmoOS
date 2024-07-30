include_directories(${CMAKE_BINARY_DIR})

add_definitions(-DKDED_UNIT_TEST)

macro(ADD_KDED_TEST testname)
    set(test_SRCS
        ${testname}.cpp
        ${CMAKE_SOURCE_DIR}/kded/generator.cpp
        ${CMAKE_SOURCE_DIR}/kded/device.cpp
        ${CMAKE_SOURCE_DIR}/kded/config.cpp
        ${CMAKE_SOURCE_DIR}/kded/output.cpp
        ${CMAKE_SOURCE_DIR}/common/globals.cpp
        ${CMAKE_SOURCE_DIR}/common/control.cpp
        #${CMAKE_SOURCE_DIR}/kded/daemon.cpp
    )
    ecm_qt_declare_logging_category(test_SRCS HEADER kscreen_daemon_debug.h IDENTIFIER KSCREEN_KDED CATEGORY_NAME kscreen.kded)

    qt_add_dbus_interface(test_SRCS
        ${CMAKE_SOURCE_DIR}/kded/org.freedesktop.DBus.Properties.xml
        freedesktop_interface
    )

    add_executable(${testname} ${test_SRCS})
    add_dependencies(${testname} kscreen) # make sure the dbus interfaces are generated
    target_compile_definitions(${testname} PRIVATE "-DTEST_DATA=\"${CMAKE_CURRENT_SOURCE_DIR}/\"")
    target_link_libraries(${testname} Qt::Test Qt::DBus Qt::Gui Qt::Sensors KF6::Screen KF6::CoreAddons)
    add_test(NAME kscreen-kded-${testname} COMMAND ${testname})
    ecm_mark_as_test(${testname})
endmacro()

add_kded_test(testgenerator)
add_kded_test(configtest)
#add_kded_test(testdaemon)
