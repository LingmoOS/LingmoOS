include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)

find_package(Qt6 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(kptyprocesstest kptyprocesstest.cpp)
target_link_libraries(kptyprocesstest KF6::Pty Qt6::Test)
ecm_mark_as_test(kptyprocesstest)
ecm_mark_nongui_executable(kptyprocesstest)
add_test(NAME kptyprocesstest COMMAND kptyprocesstest)
