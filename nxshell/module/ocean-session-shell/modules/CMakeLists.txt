add_subdirectory(virtualkeyboard)

# debug模式下增加示例demo的编译
if ((${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    add_subdirectory(examples)
    add_subdirectory(login)
endif ()
