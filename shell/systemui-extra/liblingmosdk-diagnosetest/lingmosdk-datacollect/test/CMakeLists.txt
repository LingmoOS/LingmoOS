add_executable(testdatanode testdatanode.c)
target_include_directories(testdatanode PUBLIC "/usr/include/lingmosdk/diagnosetest/")
target_link_libraries(testdatanode lingmosdk-datacollect)
# install(TARGETS testdatanode DESTINATION /usr/bin/ )