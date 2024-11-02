set(encryption_SOURCES
    ../src/rsa2048.cpp
    ../src/pbkdf2.cpp
    ../src/aes256.cpp
    ../src/encryption.cpp
)

set(cpr_SOURCES
    ../src/services/threadpool/singleton.h
    ../src/services/threadpool/async.h
    ../src/services/threadpool/async.cpp
    ../src/services/threadpool/threadpool.cpp
)

set(engine_SOURCES
    ../src/engine/aienginepluginmanager.cpp
    ../src/engine/library.cpp
)

set(config_SOURCES
    ../src/config/engineconfiguration.cpp
    ../src/config/prompttemplate.cpp
)

add_executable(testencryption
    testencryption.cpp
    ${encryption_SOURCES}
)
target_link_libraries(testencryption crypto ${OPENSSL_LIBRARIES})
add_test(NAME testencryption COMMAND testencryption)

add_executable(testaes256
    testaes256.cpp
    ../src/aes256.cpp
)
target_link_libraries(testaes256 crypto ${OPENSSL_LIBRARIES})
add_test(NAME testaes256 COMMAND testaes256)


find_package(PkgConfig REQUIRED)
find_package(LingmoAiEngine REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})
add_executable(testtextprocessor
    testtextprocessor.cpp
    ../src/services/modelmanagerservice.cpp
    ../src/services/modelmanagerserviceglue.c
    ../src/services/serverproxy.cpp
    ../src/services/textprocessorproxy.cpp
    ../src/services/textprocessorglue.c
    ../src/services/textprocessor.cpp
    ../src/services/server.cpp
    ../src/services/servicemanager.cpp
    ../src/services/speechprocessor.cpp
    ../src/services/speechprocessorproxy.cpp
    ../src/services/speechprocessorglue.c
    ../src/services/visionprocessor.cpp
    ../src/services/visionprocessorproxy.cpp
    ../src/services/visionprocessorglue.c
    ${config_SOURCES}
    ${cpr_SOURCES}
    ${engine_SOURCES}
)
target_link_libraries(testtextprocessor dl pthread jsoncpp ${GIO_LIBRARIES} ${OPENSSL_LIBRARIES})
add_test(NAME testtextprocessor COMMAND testtextprocessor)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})
add_executable(testmodelmanagerservice
    testmodelmanagerservice.cpp
    ../src/services/modelmanagerservice.cpp
    ../src/services/modelmanagerserviceglue.c
    ../src/services/modelmanagerserviceproxy.cpp
    ../src/services/serverproxy.cpp
    ../src/services/textprocessorproxy.cpp
    ../src/services/textprocessorglue.c
    ../src/services/textprocessor.cpp
    ../src/services/server.cpp
    ../src/services/servicemanager.cpp
    ../src/services/speechprocessor.cpp
    ../src/services/speechprocessorproxy.cpp
    ../src/services/speechprocessorglue.c
    ../src/services/visionprocessor.cpp
    ../src/services/visionprocessorproxy.cpp
    ../src/services/visionprocessorglue.c
    ${config_SOURCES}
    ${cpr_SOURCES}
    ${engine_SOURCES}
)
target_link_libraries(testmodelmanagerservice pthread jsoncpp dl ${GIO_LIBRARIES} ${OPENSSL_LIBRARIES})
add_test(NAME testmodelmanagerservice COMMAND testmodelmanagerservice)

find_package(PkgConfig REQUIRED)
find_package(LingmoAiEngine REQUIRED)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})
add_executable(testvisionprocessor
    testvisionprocessor.cpp
    ../src/services/modelmanagerservice.cpp
    ../src/services/modelmanagerserviceglue.c
    ../src/services/serverproxy.cpp
    ../src/services/textprocessorproxy.cpp
    ../src/services/textprocessorglue.c
    ../src/services/textprocessor.cpp
    ../src/services/server.cpp  ../src/services/server.h
    ../src/services/servicemanager.cpp
    ../src/services/speechprocessor.cpp
    ../src/services/speechprocessorproxy.cpp
    ../src/services/speechprocessorglue.c
    ../src/services/visionprocessor.cpp
    ../src/services/visionprocessorproxy.cpp
    ../src/services/visionprocessorglue.c
    ${config_SOURCES}
    ${cpr_SOURCES}
    ${engine_SOURCES}
)
target_link_libraries(testvisionprocessor dl pthread jsoncpp ${GIO_LIBRARIES} ${OPENSSL_LIBRARIES})
add_test(NAME testvisionprocessor COMMAND testvisionprocessor)

include(../cmake/engine-plugin-path-defs.cmake)
include_directories(../ai-engine)
configure_file(16k.pcm ${CMAKE_CURRENT_BINARY_DIR}/16k.pcm COPYONLY)
add_executable(testenginemanager
    ../src/engine/aienginepluginmanager.cpp
    ../src/engine/library.cpp
    testenginemanager.cpp
)
target_link_libraries(testenginemanager dl ${OPENSSL_LIBRARIES})
add_test(NAME testenginemanager COMMAND testenginemanager)
