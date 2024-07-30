# We only need to build the examples, to be sure they are up-to-date.
add_executable(client client.cpp)
target_link_libraries(client KF6::AuthCore)

add_executable(helper helper.cpp)
target_link_libraries(helper KF6::AuthCore)
