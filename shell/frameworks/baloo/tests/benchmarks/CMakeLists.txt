add_executable(unindexedfileiteratorbenchmark unindexedfileiteratortest.cpp)
target_link_libraries(unindexedfileiteratorbenchmark
    Qt6::Core
    baloofilecommon
)
ecm_mark_as_test(unindexedfileiteratorbenchmark)
