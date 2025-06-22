include(ECMMarkAsTest)

#######################################
# LogindTest
#######################################
set(logindTest_SRCS
    logindtest.cpp
    fakelogind.cpp
    ../logind.cpp
    ../kscreenlocker_logging.cpp
)

add_executable(logindTest ${logindTest_SRCS})
target_link_libraries(logindTest Qt::DBus Qt::Test KF5::I18n)
add_test(NAME ksmserver-logindTest COMMAND logindTest)
ecm_mark_as_test(logindTest)

#######################################
# PamTest
#######################################

pkg_search_module(PamWrapper pam_wrapper)
if (PamWrapper_FOUND)
    pkg_get_variable(PamWrapperModuleDir pam_wrapper "modules")
    configure_file(data/test_service.in ./test_service)
    add_executable(pamTest pamtest.cpp)
    target_link_libraries(pamTest Qt::Test kscreenlocker_authenticator)
    add_test(NAME ksmserver-pamTest COMMAND pamTest)
    ecm_mark_as_test(pamTest)
    set_property(TEST ksmserver-pamTest
        PROPERTY
        ENVIRONMENT LD_PRELOAD=libpam_wrapper.so)
endif()

#######################################
# KSldTest
#######################################
add_executable(ksldTest ksldtest.cpp)
target_link_libraries(ksldTest Qt::Test Qt::Widgets KF5::IdleTime XCB::XTEST KScreenLocker)
if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(ksldTest Qt::GuiPrivate)
endif()
add_test(NAME ksmserver-ksldTest COMMAND ksldTest)
ecm_mark_as_test(ksldTest)

#######################################
# KeyboardGrabber
#######################################
add_executable(keyboardGrabber keyboardgrabber.cpp)
target_link_libraries(keyboardGrabber Qt::Core XCB::XCB)
ecm_mark_as_test(keyboardGrabber)

#######################################
# PointerGrabber
#######################################
add_executable(pointerGrabber pointergrabber.cpp)
target_link_libraries(pointerGrabber Qt::Core XCB::XCB)
ecm_mark_as_test(pointerGrabber)

#######################################
# LockWindowTest
#######################################
set(x11LockerTest_SRCS
    x11lockertest.cpp
    ../x11locker.cpp
    ../globalaccel.cpp
    ../abstractlocker.cpp
    ../kscreenlocker_logging.cpp
)

add_executable(x11LockerTest ${x11LockerTest_SRCS})
target_link_libraries(x11LockerTest
    KF5::GlobalAccel
    KF5::I18n
    KF5::WindowSystem
    Qt::Test
    Qt::Widgets
    X11::X11
    XCB::XCB
    XCB::KEYSYMS
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(x11LockerTest Qt5::X11Extras)
else()
    target_link_libraries(x11LockerTest Qt::GuiPrivate)
endif()
add_test(NAME ksmserver-x11LockerTest COMMAND x11LockerTest)
ecm_mark_as_test(x11LockerTest)
