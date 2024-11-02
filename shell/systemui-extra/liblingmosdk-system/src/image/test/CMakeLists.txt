add_executable(kyimage-test kyimage-test.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LINGMOSDKIMAGEPROC lingmosdk-imageproc)
target_include_directories(kyimage-test PRIVATE ${LINGMOSDKIMAGEPROC_INCLUDE_DIRS})
target_link_directories(kyimage-test PRIVATE ${LINGMOSDKIMAGEPROC_LIBRARY_DIRS})
target_link_libraries(kyimage-test PRIVATE ${LINGMOSDKIMAGEPROC_LIBRARIES})