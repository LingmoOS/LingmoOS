project(TarLocalFiles)

find_package(KF6Archive ${KF_VERSION} REQUIRED)

add_executable(tarlocalfiles main.cpp)
target_link_libraries(tarlocalfiles KF6::Archive)
