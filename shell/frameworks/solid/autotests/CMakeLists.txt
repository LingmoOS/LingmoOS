include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Concurrent Test)

########### fakehardwaretest ###############

if (BUILD_DEVICE_BACKEND_fakehw)
    ecm_add_test(fakehardwaretest.cpp LINK_LIBRARIES KF6Solid_static Qt6::Xml Qt6::Test)
    # QT5 TODO Port solid to QFINDTESTDATA
    target_compile_definitions(fakehardwaretest PRIVATE SOLID_STATIC_DEFINE=1 TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/devices/backends/fakehw/fakecomputer.xml")
endif()

########### solidhwtest ###############

if (BUILD_DEVICE_BACKEND_fakehw)
    ecm_add_test(solidhwtest.cpp LINK_LIBRARIES Qt6::Xml Qt6::Test ${LIBS} KF6Solid_static)
    target_compile_definitions(solidhwtest PRIVATE SOLID_STATIC_DEFINE=1 FAKE_COMPUTER_XML="${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/devices/backends/fakehw/fakecomputer.xml")
    target_include_directories(solidhwtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/devices/backends/fakehw)
endif()

########### solidmttest ###############

ecm_add_test(solidmttest.cpp LINK_LIBRARIES Qt6::Xml Qt6::Test ${LIBS} KF6Solid_static Qt6::Concurrent)
target_compile_definitions(solidmttest PRIVATE SOLID_STATIC_DEFINE=1)

########### solidmttest ###############
if (WITH_NEW_SOLID_JOB)
    ecm_add_test(solidjobtest.cpp LINK_LIBRARIES Qt6::Test ${LIBS} KF6Solid_static)
endif()

########### solidpowertest ###############
if (WITH_NEW_SOLID_JOB AND WITH_NEW_POWER_ASYNC_API)
    ecm_add_test(solidpowertest.cpp LINK_LIBRARIES Qt6::Test ${LIBS} KF6Solid_static)
endif()

########### solidpowertest ###############
if(HAVE_DBUS AND WITH_NEW_POWER_ASYNC_API AND WITH_NEW_POWER_ASYNC_FREEDESKTOP)
    ecm_add_test(
        solidfreedesktoptest.cpp
        fakeUpower.cpp
        fakelogind.cpp
        TEST_NAME "solidfreedesktopbackend"
        LINK_LIBRARIES Qt6::Test Qt6::DBus ${LIBS} KF6Solid_static
    )
endif()

########### logindinhibitionargument ###########
if(WITH_NEW_POWER_ASYNC_API AND WITH_NEW_POWER_ASYNC_FREEDESKTOP)
    ecm_add_test(
        logindinhibitionargumenttest.cpp
        ../src/solid/power/backends/freedesktop/logindinhibitionargument.cpp
        TEST_NAME "logindinhibitionargument"
        LINK_LIBRARIES Qt6::Test KF6Solid_static
    )
endif()
