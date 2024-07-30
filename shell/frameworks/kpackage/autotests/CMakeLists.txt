find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt6Test PROPERTIES PURPOSE "Required for tests")

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(ECMMarkAsTest)
include(ECMAddTests)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

ecm_add_tests(
    fallbackpackagetest.cpp
    packagestructuretest.cpp
    plasmoidpackagetest.cpp
    querytest.cpp
    LINK_LIBRARIES Qt6::Test KF6::Package KF6::I18n KF6::Archive
)


kcoreaddons_add_plugin(plasmoidstructure_plugin STATIC SOURCES packagestructures/plasmoidstructure.cpp INSTALL_NAMESPACE kf6/packagestructure)
target_link_libraries(plasmoidstructure_plugin KF6::Package)
kcoreaddons_target_static_plugins(querytest NAMESPACE kf6/packagestructure)
kcoreaddons_target_static_plugins(plasmoidpackagetest NAMESPACE kf6/packagestructure)

set_tests_properties(querytest PROPERTIES RUN_SERIAL TRUE) # it wipes out ~/.qttest/share
set_tests_properties(plasmoidpackagetest PROPERTIES RUN_SERIAL TRUE)

function(kpackagetooltest testname)
    add_test(NAME ${testname}-appstream COMMAND cmake -Dkpackagetool=$<TARGET_FILE:kpackagetool6>
                                                      -Dgenerated=${CMAKE_CURRENT_BINARY_DIR}/${testname}.appdata.xml
                                                      -Dinput=${CMAKE_CURRENT_LIST_DIR}/data/${testname}/
                                                      -Doutput=${CMAKE_CURRENT_SOURCE_DIR}/data/${testname}/${testname}.testappdataxml
                                                      -P ${CMAKE_CURRENT_SOURCE_DIR}/kpackagetoolappstreamtest.cmake
    )
    set(XDG_DATA_DIRS "$ENV{XDG_DATA_DIRS}")
    if(NOT XDG_DATA_DIRS)
        set(XDG_DATA_DIRS "/usr/local/share:/usr/share")
    endif()
    set_property(TEST ${testname}-appstream PROPERTY ENVIRONMENT "XDG_DATA_DIRS=${CMAKE_SOURCE_DIR}/src/kpackage/data:${XDG_DATA_DIRS}")
endfunction()

foreach(var "testfallbackpackage" "testpackage" "testpackage-nodisplay" "testpackagesdep" "testpackagesdepinvalid") # "customcontent" "simplecontent"
    kpackagetooltest(${var})
endforeach()

add_subdirectory(mockdepresolver)
