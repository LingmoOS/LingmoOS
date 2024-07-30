add_subdirectory(fakebluez)

include(ECMMarkAsTest)

set(bluezqt_autotests_SRCS
    autotests.cpp
)

qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.freedesktop.DBus.Properties.xml dbusproperties_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Adapter1.xml bluezadapter1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Battery1.xml bluezbattery1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Device1.xml bluezdevice1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.GattService1.xml bluezgattservice1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.GattCharacteristic1.xml bluezgattcharacteristic1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.GattDescriptor1.xml bluezgattdescriptor1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.MediaPlayer1.xml bluezmediaplayer1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.MediaTransport1.xml bluezmediatransport1_tst)
qt_add_dbus_interface(bluezqt_autotests_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.bluez.Input1.xml bluezinput1_tst)

macro(bluezqt_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp ${bluezqt_autotests_SRCS})
        target_link_libraries(${_testname} Qt6::DBus Qt6::Test KF6BluezQt)
        add_test(NAME bluezqt-${_testname} COMMAND ${_testname})
        ecm_mark_as_test(${_testname})
        set_tests_properties(bluezqt-${_testname} PROPERTIES RUN_SERIAL TRUE)
    endforeach(_testname)
endmacro()

bluezqt_tests(
    managertest
    agentmanagertest
    obexmanagertest
    adaptertest
    batterytest
    devicetest
    gattserviceremotetest
    gattcharacteristicremotetest
    gattdescriptorremotetest
    inputtest
    mediaplayertest
    mediatransporttest
    jobstest
    mediatest
    leadvertisingmanagertest
    gattmanagertest
)

if(Qt6Qml_FOUND AND Qt6QuickTest_FOUND)
    bluezqt_tests(qmltests)
    target_link_libraries(qmltests Qt6::Qml Qt6::QuickTest)
    add_definitions(-DBLUEZQT_QML_IMPORT_PATH="${CMAKE_CURRENT_BINARY_DIR}/../src/imports")
    if (NOT BUILD_SHARED_LIBS)
        target_link_libraries(qmltests bluezqtextensionplugin)
        target_compile_definitions(qmltests PRIVATE -DIMPORT_EXTENSIONS_QML_PLUGIN)
    endif()
endif()
