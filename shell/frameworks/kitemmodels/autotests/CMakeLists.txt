
set(proxyModelSmokeTestSources
  kselectionproxymodeltestsuite.cpp
)

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(proxymodeltestsuite)
add_subdirectory(bihash)

include(ECMAddTests)

ecm_add_tests(
    kcolumnheadersmodeltest.cpp
    kdescendantsproxymodel_smoketest.cpp
    kdescendantsproxymodeltest.cpp
    kextracolumnsproxymodeltest.cpp
    klinkitemselectionmodeltest.cpp
    kmodelindexproxymappertest.cpp
    knumbermodeltest.cpp
    krearrangecolumnsproxymodeltest.cpp
    kselectionproxymodeltest.cpp

    LINK_LIBRARIES
        KF6::ItemModels
        Qt6::Test
        Qt6::Widgets
        proxymodeltestsuite
)

macro(kitemmodels_add_tests)
    ecm_add_tests(${ARGV}
        TARGET_NAMES_VAR _target_names
        LINK_LIBRARIES
            Qt6::Gui
            Qt6::Test
            Qt6::Qml
    )
    # In case of static builds, we need to link against and import the QML plugin
    if (NOT BUILD_SHARED_LIBS)
        foreach(_target ${_target_names})
            target_link_libraries(${_target} itemmodelsplugin)
            target_compile_definitions(${_target} PRIVATE -DIMPORT_ITEMMODELSPLUGIN)
        endforeach()
    endif()
endmacro()

if (TARGET Qt6::Qml)
    kitemmodels_add_tests(
        krolenames_qml.cpp
        ksortfilterproxymodel_qml.cpp
    )
endif()

# we need additional sources for this test, can't use it in ecm_add_tests
ecm_add_test(kselectionproxymodel_smoketest.cpp ${proxyModelSmokeTestSources}
    TEST_NAME "kselectionproxymodel_smoketest"
    LINK_LIBRARIES
        KF6::ItemModels
        Qt6::Test
        Qt6::Widgets
        proxymodeltestsuite
)
