cmake_minimum_required(VERSION 3.5)

project(lingmo-ai-base-test LANGUAGES CXX C)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../include)
include_directories(../src)
include_directories(../include/lingmo-ai/ai-base)
include_directories(../include/lingmo-ai)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
find_package(jsoncpp REQUIRED)
pkg_check_modules(JSON REQUIRED json-glib-1.0)

include_directories(${GIO_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})

set(SERVICE_SOURCES
    ../src/services/textprocessorproxy.cpp
    ../src/services/textprocessorglue.c
    ../src/services/speechprocessorproxy.cpp
    ../src/services/speechprocessorglue.c
    ../src/services/modelmanagerserviceproxy.cpp
    ../src/services/modelmanagerserviceglue.c
    ../src/services/serverproxy.cpp
    ../src/services/visionprocessorproxy.cpp
    ../src/services/visionprocessorglue.c
    ../src/services/keywordrecognizerproxy.cpp
    ../src/services/keywordrecognizerglue.c
)

set(SETTINGS_SOURCES
    ../src/settings/childsettings.cpp
    ../src/settings/globalsettings.cpp
    ../src/settings/settings.cpp
)

set(EXEC_SOURCES
    ../src/modelconfig.cpp
    ../src/nlp.cpp
    ../src/vision.cpp
    ../src/visionsession.cpp
    ../src/nlpsession.h
    ../src/nlpsession.cpp
    ../src/speech.cpp
    ${SERVICE_SOURCES}
    ${SETTINGS_SOURCES}
    ../src/speechsession.h
    ../src/speechsession.cpp
)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(testSpeechApi testSpeechApi.cpp
    ../src/config.cpp
    ${EXEC_SOURCES}
)
target_link_libraries(testSpeechApi
    ${GIO_LIBRARIES}
    pthread
    jsoncpp
    json-glib-1.0
    ${OPENSSL_LIBRARIES}
)

add_executable(testNlpApi testNlpApi.cpp
    ../src/config.cpp
    ${EXEC_SOURCES}
)
target_link_libraries(testNlpApi
    ${GIO_LIBRARIES}
    pthread
    jsoncpp
    json-glib-1.0
    ${OPENSSL_LIBRARIES}
)

add_test(NAME testSpeechApi COMMAND testSpeechApi)

add_executable(testsettings testsettings.cpp
    ${SETTINGS_SOURCES}
)

target_link_libraries(testsettings jsoncpp json-glib-1.0
    ${GIO_LIBRARIES}
)

add_test(NAME testsettings COMMAND testsettings)

add_executable(testconfig testconfig.cpp
    ../src/config.cpp
    ${SETTINGS_SOURCES}
)

target_link_libraries(testconfig jsoncpp json-glib-1.0
    ${GIO_LIBRARIES}
)

add_test(NAME testconfig COMMAND testconfig)

add_executable(testVisionApi testVisionApi.cpp ../src/config.cpp ${EXEC_SOURCES})
target_link_libraries(testVisionApi
    ${GIO_LIBRARIES}
    pthread
    jsoncpp
    json-glib-1.0
    ${OPENSSL_LIBRARIES}
)

add_executable(testOcrApi testOcrApi.cpp
    ../src/ocr.cpp
    ../src/ocrsession.h
    ../src/ocrsession.cpp
    ../src/services/ocrprocessorproxy.cpp
    ${EXEC_SOURCES}
)
target_link_libraries(testOcrApi
    ${GIO_LIBRARIES}
    pthread
    jsoncpp
    json-glib-1.0
    ${OPENSSL_LIBRARIES}
    tesseract
    leptonica
)
