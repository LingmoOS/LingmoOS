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
target_link_libraries(logindTest Qt::DBus Qt::Test KF6::I18n)
add_test(NAME ksmserver-logindTest COMMAND logindTest)
ecm_mark_as_test(logindTest)

#######################################
# PamTest
#######################################

pkg_search_module(PamWrapper pam_wrapper)
# We're turning it off on the CI until pam_wrapper 1.15 is available on our image (which adds sanitizer support.)
# see https://invent.kde.org/plasma/kscreenlocker/-/issues/11 for more details
if (PamWrapper_FOUND AND NOT "$ENV{KDECI_BUILD}" STREQUAL "TRUE")
    pkg_get_variable(PamWrapperModuleDir pam_wrapper "modules")
    configure_file(data/test_service.in ./test_service)
    add_executable(pamTest pamtest.cpp)
    target_link_libraries(pamTest Qt::Test kscreenlocker_authenticator)
    add_test(NAME ksmserver-pamTest COMMAND pamTest)
    # they made a mistake in the env name, so set both old and new
    set_tests_properties(ksmserver-pamTest PROPERTIES ENVIRONMENT "UID_WRAPPER_DISABLE_DEEPBIND=1;PAM_WRAPPER_DISABLE_DEEPBIND=1")
    ecm_mark_as_test(pamTest)
    set_property(TEST ksmserver-pamTest
            PROPERTY
            ENVIRONMENT "LD_PRELOAD=libpam_wrapper.so;ASAN_OPTIONS=verify_asan_link_order=0")
endif()

#######################################
# KSldTest
#######################################
add_executable(ksldTest ksldtest.cpp)
target_link_libraries(ksldTest Qt::Test Qt::Widgets KF6::IdleTime XCB::XTEST KScreenLocker)
target_link_libraries(ksldTest Qt::GuiPrivate)
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
    KF6::GlobalAccel
    KF6::I18n
    KF6::WindowSystem
    Qt::Test
    Qt::Widgets
    X11::X11
    XCB::XCB
    XCB::KEYSYMS
)

target_link_libraries(x11LockerTest Qt::GuiPrivate)

add_test(NAME ksmserver-x11LockerTest COMMAND x11LockerTest)
ecm_mark_as_test(x11LockerTest)
