find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

set(additionalLibraries )
if(HAVE_X11 AND XCB_XTEST_FOUND)
    add_definitions(-DHAVE_XCB_XTEST)
    set(additionalLibraries Qt5::X11Extras XCB::XTEST XCB::KEYSYMS)
else()
    set(additionalLibraries )
endif()

include(ECMAddTests)

ecm_add_tests(
   kglobalshortcuttest.cpp
   LINK_LIBRARIES Qt5::Test KF5::GlobalAccel ${additionalLibraries}
)
