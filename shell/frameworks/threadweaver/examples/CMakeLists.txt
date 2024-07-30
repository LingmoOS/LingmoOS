find_package(Qt6Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

# CMake file that produces Markdown files to check if all snippet references resolve.
find_program(SNIPPETEXTRACTOR snippetextractor)
if (SNIPPETEXTRACTOR)
   # ...generate Markdown files with Snippets for HelloWorldRaw.in.md and HelloInternet.in.md
endif()

add_subdirectory(HelloWorld)
add_subdirectory(HelloWorldRaw)
add_subdirectory(HelloInternet)
add_subdirectory(ThumbNailer)
