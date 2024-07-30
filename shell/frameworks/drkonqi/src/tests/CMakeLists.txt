remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG REQUIRED)

if(NOT CMAKE_SYSTEM_NAME MATCHES FreeBSD)
    find_pythonmodule(chai)
    set_package_properties("chai-PythonModule" PROPERTIES
        DESCRIPTION "Python module chai is required for testing."
        TYPE REQUIRED)
    add_test(NAME preambletest
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/preambletest.py)
endif()

# Test helper for systeminformationtest
if(HAVE_UNAME)
    add_executable(lsb_release lsb_release_double.c)

    ecm_add_tests(systeminformationtest.cpp LINK_LIBRARIES Qt::Core Qt::Test DrKonqiInternal)
endif()

add_subdirectory(crashtest)
add_subdirectory(backtraceparsertest)
add_subdirectory(bugzillalibtest)
add_subdirectory(sentrytest)

ecm_add_tests(gdbbacktracelinetest.cpp LINK_LIBRARIES Qt::Core Qt::Test drkonqi_backtrace_parser)
ecm_add_tests(
        linuxprocmapsparsertest.cpp
        statusnotifier_activationclosetimertest.cpp
    LINK_LIBRARIES Qt::Core Qt::Test DrKonqiInternal)

if(NOT APPLE)
    if(NOT RUBY_EXECTUABLE)
        find_program(RUBY_EXECUTABLE ruby)
    endif()
    if(RUBY_EXECUTABLE)
        execute_process(COMMAND ${RUBY_EXECUTABLE} -e "require 'atspi'" RESULT_VARIABLE RUBY_ATSPI)
    endif()
    if(NOT GDB_EXECUTABLE)
        # Needed so drkonqi can actually trace something.
        find_program(GDB_EXECUTABLE gdb)
    endif()
    if(NOT XVFB_RUN_EXECTUABLE)
        find_program(XVFB_RUN_EXECTUABLE xvfb-run)
    endif()

    set(ATSPI_PATHS
        /usr/lib/at-spi2-core/ # debians
        /usr/libexec/ # newer debians
        /usr/lib/at-spi2/ # suses
    )
    if(NOT ATSPI_BUS_LAUNCHER_EXECUTABLE)
        find_program(
            ATSPI_BUS_LAUNCHER_EXECUTABLE
            NAMES at-spi-bus-launcher
            PATHS ${ATSPI_PATHS}
            DOC "AT-SPI accessibility dbus launcher"
        )
    endif()
    if(NOT ATSPI_REGISTRYD_EXECUTABLE)
        find_program(
            ATSPI_REGISTRYD_EXECUTABLE
            NAMES at-spi2-registryd
            PATHS ${ATSPI_PATHS}
            DOC "AT-SPI accessibility registry daemon"
        )
    endif()

    if(
        RUBY_EXECUTABLE
        AND XVFB_RUN_EXECTUABLE
        AND ATSPI_BUS_LAUNCHER_EXECUTABLE
        AND ATSPI_REGISTRYD_EXECUTABLE
        AND GDB_EXECUTABLE
        AND RUBY_ATSPI EQUAL 0
    )
        set(WITH_DRKONI_INTEGRATION_TESTING TRUE)
        add_subdirectory(integration)
    endif()
    add_feature_info(
        DrKonqiIntegrationTesting
        WITH_DRKONI_INTEGRATION_TESTING
        "Needs Ruby, functional atspi gem, gdb, as well as xvfb-run."
    )
endif()
