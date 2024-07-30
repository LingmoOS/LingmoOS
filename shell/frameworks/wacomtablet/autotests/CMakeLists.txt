find_package(Qt6Test REQUIRED)
include(ECMMarkAsTest)

include_directories( ${CMAKE_SOURCE_DIR}/src )
include_directories( ${CMAKE_SOURCE_DIR}/src/common )
include_directories( ${CMAKE_SOURCE_DIR}/src/kded )
include_directories( ${CMAKE_SOURCE_DIR}/src/kcmodule )

set(WACOM_COMMON_TEST_LIBS wacom_common Qt::Test)
set(WACOM_KDED_TEST_LIBS kded_wacomtablet_lib ${kded_wacomtablet_LIBS} Qt::Test)
set(WACOM_KCM_TEST_LIBS kcm_testlib ${kcm_wacomtablet_LIBS} Qt::Test)

add_subdirectory(faketablet)

# Add common Tests
add_subdirectory( common/buttonshortcut )
add_subdirectory( common/deviceinformation )
add_subdirectory( common/deviceprofile )
add_subdirectory( common/deviceprofileconfigadaptor )
add_subdirectory( common/deviceproperty )
add_subdirectory( common/enum )
add_subdirectory( common/libwacomdata )
add_subdirectory( common/profilemanager )
add_subdirectory( common/property )
add_subdirectory( common/propertyset )
add_subdirectory( common/screenspace )
add_subdirectory( common/tabletarea )
add_subdirectory( common/tabletinformation )
add_subdirectory( common/tabletprofile )
add_subdirectory( common/tabletprofileconfigadaptor )

# Add kded Tests
add_subdirectory( kded/dbustabletservice )
add_subdirectory( kded/tabletbackend )
add_subdirectory( kded/tabletdatabase )
add_subdirectory( kded/tablethandler )
add_subdirectory( kded/xinputadaptor )
add_subdirectory( kded/xsetwacomadaptor )

# Add kcm tests
add_subdirectory( kcm/styluspage )
add_subdirectory( kcm/tabletpage )
