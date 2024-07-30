project(Unzipper)

find_package(KF6Archive ${KF_VERSION} REQUIRED)

add_executable(unzipper main.cpp)
target_link_libraries(unzipper KF6::Archive)
