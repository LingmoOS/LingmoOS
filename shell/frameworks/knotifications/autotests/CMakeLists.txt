include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::Test)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

#if (HAVE_DBUS)
#    set(KNotificationTest_SRCS knotification_test.cpp fake_notifications_server.cpp)
#    ecm_add_test(${KNotificationTest_SRCS} TEST_NAME "KNotificationTest" LINK_LIBRARIES Qt6::Test Qt6::DBus KF6::Notifications)
#endif()
