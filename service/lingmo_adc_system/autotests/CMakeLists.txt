include(ECMAddTests)

find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} CONFIG QUIET)
find_package(Qt${QT_MAJOR_VERSION}Concurrent ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt${QT_MAJOR_VERSION}Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

if(NOT Qt${QT_MAJOR_VERSION}Concurrent_FOUND)
    message(STATUS "Qt5Concurrent not found, autotests will not be built.")
    return()
endif()

# compile KEntryMap into the test since it's not exported
ecm_add_test(
  kentrymaptest.cpp
  ../src/core/kconfigdata.cpp
  TEST_NAME kentrymaptest
  LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test
)
target_include_directories(kentrymaptest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/core)

# compile KConfigUtils into the test since it's not exported
ecm_add_test(
  test_kconfigutils.cpp
  ../src/kconf_update/kconfigutils.cpp
  TEST_NAME test_kconfigutils
  LINK_LIBRARIES KF5::ConfigCore Qt${QT_MAJOR_VERSION}::Test
)
target_include_directories(test_kconfigutils PRIVATE ../src/kconf_update)

qt_add_resources(sharedconfigresources sharedconfigresources.qrc)

ecm_add_test(ksharedconfigtest.cpp ${sharedconfigresources} TEST_NAME kconfigcore-ksharedconfigtest LINK_LIBRARIES KF5::ConfigCore Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Concurrent)

# test for fallback to :/kconfig/xxxx config resource
qt_add_resources(fallbackconfigresources fallbackconfigresources.qrc)
ecm_add_test(fallbackconfigresourcestest.cpp ${fallbackconfigresources} TEST_NAME kconfigcore-fallbackconfigresourcestest LINK_LIBRARIES KF5::ConfigCore Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Concurrent)

ecm_add_tests(
   kconfignokdehometest.cpp
   kconfigtest.cpp
   kdesktopfiletest.cpp
   test_kconf_update.cpp
   ksharedconfig_in_global_object.cpp
   NAME_PREFIX kconfigcore-
   LINK_LIBRARIES KF5::ConfigCore Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Concurrent
)

target_include_directories(test_kconf_update PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../src/kconf_update)

if(TARGET Qt${QT_MAJOR_VERSION}::Gui)
ecm_add_tests(
   kconfigguitest.cpp
   kconfigloadertest.cpp
   kconfigskeletontest.cpp
   kstandardshortcuttest.cpp
   NAME_PREFIX kconfiggui-
   LINK_LIBRARIES KF5::ConfigGui Qt${QT_MAJOR_VERSION}::Test
)

ecm_add_test(
   kstandardshortcutwatchertest.cpp
   # add sources instead of linking so we can use the unexported initialize()
   ../src/gui/kstandardshortcut.cpp
   ../src/gui/kstandardshortcutwatcher.cpp
   LINK_LIBRARIES KF5::ConfigCore Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Test
   TEST_NAME kstandardshortcutwatchertest
   NAME_PREFIX kconfiggui-
)
target_include_directories(kstandardshortcutwatchertest PRIVATE "$<TARGET_PROPERTY:KF5ConfigGui,INTERFACE_INCLUDE_DIRECTORIES>")
target_compile_definitions(kstandardshortcutwatchertest PRIVATE "-DKCONFIGGUI_STATIC_DEFINE")

# These tests do a global cleanup of ~/.qttest, so they can't run in parallel
set_tests_properties(kconfigcore-kconfigtest PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(kconfigcore-kconfignokdehometest PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(kconfiggui-kconfigguitest PROPERTIES RUN_SERIAL TRUE)

if (NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(kconfig_compiler)
endif()
endif()

find_package(Qt${QT_MAJOR_VERSION} OPTIONAL_COMPONENTS Widgets)
if (TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    ecm_add_test(kwindowstatesavertest.cpp LINK_LIBRARIES KF5::ConfigGui Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Widgets)
endif()
