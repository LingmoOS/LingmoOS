set(TARGET_NAME dci)
set(BIN_NAME ${TARGET_NAME}${DTK_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
add_definitions(-DDTK_NO_PROJECT)
# start dci
include(../../src/dci/dci.cmake)
add_executable(${BIN_NAME}
  ${dci_SRCS}
  main.cpp
)
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
#end dci

install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
