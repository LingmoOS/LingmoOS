# collect all files
file(GLOB_RECURSE ALL_FILES
    RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    "libraries/*.*"
    "commands/*.*"
    "indentation/*.*"
    "files/*.*")

# generate the resource file

set(INDEXFILE "")
string(APPEND INDEXFILE "<!DOCTYPE RCC>\n")
string(APPEND INDEXFILE "<RCC version=\"1.0\">\n")
string(APPEND INDEXFILE "<qresource prefix=\"/ktexteditor/script\">\n")
foreach(highlighter ${ALL_FILES})
    file(RELATIVE_PATH highlighter_base ${CMAKE_CURRENT_SOURCE_DIR} ${highlighter})
    string(APPEND INDEXFILE "<file alias=\"${highlighter_base}\">${highlighter}</file>\n")
endforeach()
string(APPEND INDEXFILE "</qresource>\n")
string(APPEND INDEXFILE "</RCC>\n")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/script-tmp.qrc ${INDEXFILE})
configure_file(${CMAKE_CURRENT_BINARY_DIR}/script-tmp.qrc
               ${CMAKE_CURRENT_BINARY_DIR}/script.qrc COPYONLY)
