add_executable(writeiotest writeiotest.cpp)
target_link_libraries(writeiotest
    KF6::BalooEngine
)
ecm_mark_as_test(writeiotest)

add_executable(memorytest memorytest.cpp)
target_link_libraries(memorytest
    KF6::BalooEngine
)
ecm_mark_as_test(memorytest)

add_executable(documenturldb documenturldbtest.cpp)
target_link_libraries(documenturldb
    Qt6::Core
    KF6::BalooEngine
)
ecm_mark_as_test(documenturldb)
