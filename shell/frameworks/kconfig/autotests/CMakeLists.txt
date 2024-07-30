include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)
find_package(Qt6Concurrent ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt6Test_FOUND)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

if(NOT Qt6Concurrent_FOUND)
    message(STATUS "Qt6Concurrent not found, autotests will not be built.")
    return()
endif()

# compile KEntryMap into the test since it's not exported
ecm_add_test(
  kentrymaptest.cpp
  ../src/core/kconfigdata.cpp
  TEST_NAME kentrymaptest
  LINK_LIBRARIES Qt6::Test
)
target_include_directories(kentrymaptest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/core)

qt_add_resources(sharedconfigresources sharedconfigresources.qrc)

ecm_add_test(ksharedconfigtest.cpp ${sharedconfigresources} TEST_NAME kconfigcore-ksharedconfigtest LINK_LIBRARIES KF6::ConfigCore Qt6::Test Qt6::Concurrent)

# test for fallback to :/kconfig/xxxx config resource
qt_add_resources(fallbackconfigresources fallbackconfigresources.qrc)
ecm_add_test(fallbackconfigresourcestest.cpp ${fallbackconfigresources} TEST_NAME kconfigcore-fallbackconfigresourcestest LINK_LIBRARIES KF6::ConfigCore Qt6::Test Qt6::Concurrent)

ecm_add_tests(
    kconfignokdehometest.cpp
    kconfigtest.cpp
    kdesktopfiletest.cpp
    test_kconf_update.cpp
    ksharedconfig_in_global_object.cpp
    dbussanitizertest.cpp
    NAME_PREFIX kconfigcore-
    LINK_LIBRARIES KF6::ConfigCore Qt6::Test Qt6::Concurrent
)

add_dependencies(test_kconf_update kconf_update)

if(TARGET Qt6::Gui)
ecm_add_tests(
    kconfigguitest.cpp
    kconfigloadertest.cpp
    kconfigskeletontest.cpp
    kstandardshortcuttest.cpp
    NAME_PREFIX kconfiggui-
    LINK_LIBRARIES KF6::ConfigGui Qt6::Test
)

ecm_add_test(
    kstandardshortcutwatchertest.cpp
    # add sources instead of linking so we can use the unexported initialize()
    ../src/gui/kstandardshortcut.cpp
    ../src/gui/kstandardshortcutwatcher.cpp
    LINK_LIBRARIES KF6::ConfigCore Qt6::Gui Qt6::Test
    TEST_NAME kstandardshortcutwatchertest
    NAME_PREFIX kconfiggui-
)
target_include_directories(kstandardshortcutwatchertest PRIVATE "$<TARGET_PROPERTY:KF6ConfigGui,INTERFACE_INCLUDE_DIRECTORIES>")
target_compile_definitions(kstandardshortcutwatchertest PRIVATE "-DKCONFIGGUI_STATIC_DEFINE")

# These tests do a global cleanup of ~/.qttest, so they can't run in parallel
set_tests_properties(kconfigcore-kconfigtest PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(kconfigcore-kconfignokdehometest PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(kconfiggui-kconfigguitest PROPERTIES RUN_SERIAL TRUE)

if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(kconfig_compiler)
endif()
endif()

find_package(Qt6 OPTIONAL_COMPONENTS Widgets)
if(TARGET Qt6::Widgets)
    ecm_add_test(kwindowstatesavertest.cpp LINK_LIBRARIES KF6::ConfigGui Qt6::Test Qt6::Widgets)
endif()

# benchmarks, don't execute during normal testing
# they don't test additional stuff
add_executable(kconfig_benchmark kconfig_benchmark.cpp)
ecm_mark_nongui_executable(kconfig_benchmark)
add_test(NAME kconfig_benchmark COMMAND kconfig_benchmark CONFIGURATIONS BENCHMARK)
target_link_libraries(kconfig_benchmark KF6::ConfigCore Qt6::Test)
