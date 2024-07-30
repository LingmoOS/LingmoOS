
find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::Test)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

find_package(Qt6DBus ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::DBus)
    message(STATUS "Qt6DBus not found, autotests will not be built.")
    return()
endif()

qt_add_dbus_adaptor(kauth_dbus_adaptor_tests_SRCS
                     ../src/backends/dbus/org.kde.kf6auth.xml
                     ../src/backends/dbus/DBusHelperProxy.h
                     KAuth::DBusHelperProxy)

include(ECMAddTests)

set(kauthdebug_SRCS)
ecm_qt_declare_logging_category(kauthdebug_tests_SRCS
    HEADER kauthdebug.h
    IDENTIFIER KAUTH
    CATEGORY_NAME kf.auth
)


set(libkauth_tests_static_SRCS
    ../src/action.cpp
    ../src/actionreply.cpp
    ../src/executejob.cpp
    ../src/AuthBackend.cpp
    # Use our "special" backends manager
    BackendsManager.cpp
    ../src/HelperProxy.cpp
    ../src/helpersupport.cpp
    TestBackend.cpp
    ../src/backends/dbus/DBusHelperProxy.cpp
    ${kauth_dbus_adaptor_tests_SRCS}
    ${kauthdebug_tests_SRCS}
)

add_library(kauth_tests_static STATIC ${libkauth_tests_static_SRCS})
# make sure all executables using this library have the define set to make sure it builds on MSVC
target_compile_definitions(kauth_tests_static
    PUBLIC KAUTHCORE_STATIC_DEFINE=1
    PUBLIC KAUTH_STATIC_DEFINE=1
)
# have to manually duplicate logic of flags set by ecm_generate_expoirt_header on the library
ecm_export_header_format_version(${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    CURRENT_VERSION ${KF_VERSION}
    HEXNUMBER_VAR kauthcore_tests_no_deprecated_before_and_at
)
target_compile_definitions(kauth_tests_static
    PUBLIC "KAUTHCORE_DISABLE_DEPRECATED_BEFORE_AND_AT=${kauthcore_tests_no_deprecated_before_and_at}"
    PUBLIC "KAUTHCORE_DEPRECATED_WARNINGS_SINCE=${kauthcore_tests_no_deprecated_before_and_at}"
)
ecm_mark_as_test(kauth_tests_static)

target_include_directories(kauth_tests_static
  PUBLIC
    $<TARGET_PROPERTY:KF6::AuthCore,INTERFACE_INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(kauth_tests_static PUBLIC Qt6::DBus KF6::CoreAddons)

########### next target ###############

ecm_add_test(SetupActionTest.cpp
    TEST_NAME KAuthSetupActionTest
    LINK_LIBRARIES Qt6::Test kauth_tests_static
)

########### next target ###############

ecm_add_test(HelperTest.cpp TestHelper.cpp
    TEST_NAME KAuthHelperTest
    LINK_LIBRARIES Qt6::Test kauth_tests_static
)

########### next target ###############

add_executable(FdHelper FdHelper.cpp)
target_link_libraries(FdHelper PUBLIC kauth_tests_static)

ecm_add_test(FdTester.cpp
    TEST_NAME KAuthFdTest
    LINK_LIBRARIES Qt6::Test kauth_tests_static
)
target_compile_definitions(KAuthFdTest PRIVATE -DFDHELPEREXE=\"$<TARGET_FILE:FdHelper>\")

########### test kauth-policy-gen ###############
if(NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    add_test(NAME KAuthPolicyGenTest
             COMMAND kauth-policy-gen ${CMAKE_SOURCE_DIR}/autotests/foo_actions.actions
                                      ${CMAKE_BINARY_DIR}/generated-foo.policy)
    add_test(KAuthPolicyGenTestCompare ${CMAKE_COMMAND} -E compare_files
             ${CMAKE_BINARY_DIR}/generated-foo.policy ${CMAKE_SOURCE_DIR}/autotests/foo.policy)
    set_tests_properties(KAuthPolicyGenTestCompare PROPERTIES DEPENDS KAuthPolicyGenTest)
endif()

