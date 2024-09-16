find_package(Qt5 COMPONENTS Test DBus REQUIRED)
find_package(GTest REQUIRED)
ADD_COMPILE_OPTIONS(-fno-access-control)

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -lgcov -fno-access-control")

# 增加安全编译参数
set(SECURITY_COMPILE, "-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")
set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} ${SECURITY_COMPILE}")
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} ${SECURITY_COMPILE}")

# Address Sanitizer 内存错误检测工具,打开下面的编译选项可以看到调试信息，正常运行时不需要这些信息
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O2")
endif()

add_subdirectory(dde-lock)
add_subdirectory(lightdm-deepin-greeter)
