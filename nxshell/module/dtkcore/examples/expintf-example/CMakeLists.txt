set(BIN_NAME exprintf${DTK_VERSION_MAJOR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)

add_executable(${BIN_NAME}
  main.cpp
)

target_link_libraries(
  ${BIN_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::DBus 
  ${LIB_NAME}
)
target_include_directories(${BIN_NAME} PUBLIC
  ../../include/base
  ../../include/base/private/
  ../../include/filesystem/
  ../../include/
)
