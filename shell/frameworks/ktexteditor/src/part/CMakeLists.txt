# kate part itself just is core + the factory
add_library (katepart MODULE katepart.cpp)

configure_file(katepart.json.in ${CMAKE_CURRENT_BINARY_DIR}/katepart.json @ONLY)

# link the part, use kate part core + kde stuff
target_link_libraries (katepart KF6TextEditor KF6::SyntaxHighlighting)

# install the part
install (TARGETS katepart DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/parts)
