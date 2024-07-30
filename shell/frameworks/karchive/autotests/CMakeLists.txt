remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)
find_package(Qt6Network)

if(NOT Qt6Test_FOUND)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

ecm_add_tests(
    karchivetest.cpp
    kfiltertest.cpp
    LINK_LIBRARIES KF6::Archive Qt6::Test
)

target_link_libraries(kfiltertest ZLIB::ZLIB)

########### klimitediodevicetest ###############

ecm_add_test(
    klimitediodevicetest.cpp
    ../src/klimitediodevice.cpp
    TEST_NAME klimitediodevicetest
    LINK_LIBRARIES Qt6::Test
)
target_include_directories(klimitediodevicetest
    PRIVATE $<TARGET_PROPERTY:KF6Archive,INTERFACE_INCLUDE_DIRECTORIES>)

########## kcompressiondevicetest ##############
if (Qt6Network_FOUND)
    ecm_add_test(
        kcompressiondevicetest.cpp
        LINK_LIBRARIES KF6::Archive Qt6::Test Qt6::Network
    )

    set(testDir $<TARGET_FILE_DIR:kcompressiondevicetest>)
    get_filename_component(topdir ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

    add_custom_command(TARGET kcompressiondevicetest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar czf
        ${testDir}/kcompressiondevice_test.tar.gz examples
        WORKING_DIRECTORY ${topdir})
    add_custom_command(TARGET kcompressiondevicetest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar cjf
        ${testDir}/kcompressiondevice_test.tar.bz2 examples
        WORKING_DIRECTORY ${topdir})

    if (LIBLZMA_FOUND)
        add_custom_command(TARGET kcompressiondevicetest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E tar cJf
            ${testDir}/kcompressiondevice_test.tar.xz examples
            WORKING_DIRECTORY ${topdir})
    endif()

    if (LibZstd_FOUND)
        # cmake doesn't support creating zstd files so run tar directly
        # which is a bit annoying since not all tars support zstd either
        # so we first check that the installed tar has the --zstd
        # option and then we check that the zstd binary is available

        execute_process(COMMAND tar --help OUTPUT_VARIABLE TAR_OUTPUT)
        if (TAR_OUTPUT MATCHES ".*--zstd.*")
            find_program(ZSTD_FOUND zstd)
            if (ZSTD_FOUND)
                add_custom_command(TARGET kcompressiondevicetest POST_BUILD
                    COMMAND tar --zstd -cf
                    ${testDir}/kcompressiondevice_test.tar.zst examples
                    WORKING_DIRECTORY ${topdir})

                target_compile_definitions(kcompressiondevicetest PRIVATE HAVE_ZSTD_SUPPORT_FILE)
            endif()
        endif()
    endif()
endif()
