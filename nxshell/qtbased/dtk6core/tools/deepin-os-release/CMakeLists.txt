set(TARGET_NAME deepin-os-release)
set(BIN_NAME ${TARGET_NAME}${DTK_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
add_definitions(-DDTK_NO_PROJECT)
# start dci
set(dci_SRCS
  ../../include/global/dsysinfo.h
  ../../include/global/ddesktopentry.h
  ../../src/dsysinfo.cpp
  ../../src/ddesktopentry.cpp
)
add_executable(${BIN_NAME}
  ${dci_SRCS}
  main.cpp
)

target_compile_definitions(${BIN_NAME} PUBLIC
    DSYSINFO_PREFIX="${DSYSINFO_PREFIX}")

target_link_libraries(${BIN_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::CorePrivate
)
target_include_directories(${BIN_NAME} PUBLIC
  ../../include/
  ../../include/dci/
  ../../include/DtkCore/
  ../../include/base/
  ../../include/global/
)
set_target_properties(${BIN_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")

#end dci
