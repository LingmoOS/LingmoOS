set(KCOREADDONS_INTERNAL_SKIP_PLUGIN_INSTALLATION ON)
add_definitions(-DQT_FORCE_ASSERTS=1)

include(ECMAddTests)
include(ConfigureChecks.cmake) # configure checks for QFileSystemWatcher
include(../KF6CoreAddonsMacros.cmake)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::Test)
    message(STATUS "QtTest not found, autotests will not be built.")
    return()
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES "[Dd]ebug$")
    set(ENABLE_BENCHMARKS 1)
endif()

add_library(autotests_static STATIC)
# Needed to link this static lib to shared libs
set_property(TARGET autotests_static PROPERTY POSITION_INDEPENDENT_CODE ON)
ecm_qt_declare_logging_category(autotests_static
    HEADER kcoreaddons_debug.h
    IDENTIFIER KCOREADDONS_DEBUG
    CATEGORY_NAME kf.coreaddons
)
target_link_libraries(autotests_static Qt6::Core)

if(ENABLE_PCH)
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp "/*empty file*/")
    endif()
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pch_tmp.h
        "#pragma once\n"
        "#include <QObject>\n"
        "#include <QTest>\n"
        "#include <QString>\n"
        "#include <QDateTime>\n"
    )
    # avoid rebuilding if there was no change
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/pch_tmp.h" "${CMAKE_CURRENT_BINARY_DIR}/pch.h")

    add_library(tests_pch STATIC ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp)
    target_link_libraries(tests_pch Qt6::Core Qt6::Test)
    target_precompile_headers(tests_pch PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/pch.h)
endif()


configure_file(config-tests.h.in config-tests.h)

add_subdirectory(plugin)

macro(add_unit_test testname)
    ecm_add_test(${testname}.cpp ${ARGN}
        TEST_NAME ${testname}
        LINK_LIBRARIES Qt6::Test KF6::CoreAddons autotests_static)
    if(ENABLE_PCH)
        target_precompile_headers(${testname} REUSE_FROM tests_pch)
    endif()
endmacro()

add_unit_test(kaboutdatatest)
add_unit_test(kaboutdataapplicationdatatest)
add_unit_test(kautosavefiletest)
add_unit_test(kcompositejobtest)
add_unit_test(kformattest)
add_unit_test(kjobtest)
add_unit_test(kosreleasetest)
add_unit_test(kprocesstest)
add_unit_test(krandomtest)
add_unit_test(kshareddatacachetest)
add_unit_test(kshelltest)
add_unit_test(kurlmimedatatest)
add_unit_test(kstringhandlertest)
add_unit_test(kmacroexpandertest)
add_unit_test(kusertest)
add_unit_test(kprocesslisttest)
add_unit_test(kfileutilstest)
add_unit_test(kfuzzymatchertest)
add_unit_test(knetworkmountstestcanonical)
add_unit_test(knetworkmountstestnoconfig)
add_unit_test(knetworkmountstestpaths)
add_unit_test(knetworkmountsteststatic)
add_unit_test(klibexectest)
add_unit_test(kmemoryinfotest)
add_unit_test(kruntimeplatformtest)
if (WIN32)
    add_unit_test(klistopenfilesjobtest_win)
endif ()
if (UNIX)
    add_unit_test(klistopenfilesjobtest_unix)
endif ()

if(TARGET klistopenfilesjobtest_unix AND CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    find_package(Qt6Network ${REQUIRED_QT_VERSION} CONFIG QUIET)
    target_link_libraries(klistopenfilesjobtest_unix Qt6::Network)
endif()

add_library(ktexttohtmlteststatic STATIC ${CMAKE_SOURCE_DIR}/src/lib/text/ktexttohtml.cpp ${CMAKE_SOURCE_DIR}/src/lib/text/kemoticonsparser.cpp)
# include the binary dir in order to get kcoreaddons_export.h
target_include_directories(ktexttohtmlteststatic PUBLIC ${KCoreAddons_BINARY_DIR}/src/lib)
# fake static linking to prevent the export macros on Windows from kicking in
target_compile_definitions(ktexttohtmlteststatic PUBLIC -DKCOREADDONS_STATIC_DEFINE=1)
target_link_libraries(ktexttohtmlteststatic PUBLIC Qt6::Test autotests_static)
ecm_add_test(ktexttohtmltest.cpp
    TEST_NAME ktexttohtmltest
    LINK_LIBRARIES ktexttohtmlteststatic
)

add_executable(ktexttohtmlbenchmarktest ktexttohtmlbenchmarktest.cpp ${CMAKE_SOURCE_DIR}/src/lib/text/ktexttohtml.cpp ${CMAKE_SOURCE_DIR}/src/lib/text/kemoticonsparser.cpp)
target_link_libraries(ktexttohtmlbenchmarktest PUBLIC ktexttohtmlteststatic)

add_executable(kprocesstest_helper kprocesstest_helper.cpp)
target_link_libraries(kprocesstest_helper KF6::CoreAddons)

set(KDIRWATCH_BACKENDS_TO_TEST Stat) #Stat is always compiled

if (HAVE_SYS_INOTIFY_H)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST INotify)
endif()

if (HAVE_QFILESYSTEMWATCHER)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST QFSWatch)
endif()

foreach(_backendName ${KDIRWATCH_BACKENDS_TO_TEST})
    string(TOLOWER ${_backendName} _lowercaseBackendName)
    set(BACKEND_TEST_TARGET kdirwatch_${_lowercaseBackendName}_unittest)
    set(BACKEND_BENCHMARK_TARGET kdirwatch_${_lowercaseBackendName}_benchmarktest)
    add_executable(${BACKEND_TEST_TARGET} kdirwatch_unittest.cpp)
    target_link_libraries(${BACKEND_TEST_TARGET} Qt6::Test KF6::CoreAddons autotests_static)

    if(NOT WIN32)
        target_link_libraries(${BACKEND_TEST_TARGET} Threads::Threads)
    endif()

    ecm_mark_as_test(${BACKEND_TEST_TARGET})
    add_test(NAME ${BACKEND_TEST_TARGET} COMMAND ${BACKEND_TEST_TARGET})
    target_compile_definitions(${BACKEND_TEST_TARGET} PUBLIC -DKDIRWATCH_TEST_METHOD=\"${_backendName}\")

    add_executable(${BACKEND_BENCHMARK_TARGET} kdirwatch_benchmarktest.cpp)
    target_compile_definitions(${BACKEND_BENCHMARK_TARGET} PUBLIC -DKDIRWATCH_TEST_METHOD=\"${_backendName}\")
    target_link_libraries(${BACKEND_BENCHMARK_TARGET} Qt6::Test KF6::CoreAddons autotests_static)
    if(NOT WIN32)
        target_link_libraries(${BACKEND_BENCHMARK_TARGET} Threads::Threads)
    endif()

    if(ENABLE_PCH AND NOT WIN32)
        target_precompile_headers(${BACKEND_TEST_TARGET} REUSE_FROM tests_pch)
        target_precompile_headers(${BACKEND_BENCHMARK_TARGET} REUSE_FROM tests_pch)
    endif()

endforeach()
