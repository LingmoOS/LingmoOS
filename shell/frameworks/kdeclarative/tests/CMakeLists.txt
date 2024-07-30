include(ECMMarkAsTest)

find_package(Qt6Test REQUIRED)

add_executable(qimageitemtest qimageitemtest.cpp)

ecm_mark_as_test(qimageitemtest)
target_link_libraries(qimageitemtest
   Qt6::Quick
   Qt6::Test
)
