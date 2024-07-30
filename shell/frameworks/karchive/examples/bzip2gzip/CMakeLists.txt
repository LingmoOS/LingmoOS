project(BZip2GZip)

find_package(KF6Archive ${KF_VERSION} REQUIRED)

add_executable(bzip2gzip main.cpp)
target_link_libraries(bzip2gzip KF6::Archive)
