########### install files ###########

ecm_generate_headers(Syndication_Atom_CamelCase_HEADERS
  HEADER_NAMES
  Atom
  Category
  Constants
  Content
  Document
  Entry
  Generator
  Link
  Parser
  Person
  Source

  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/..
  PREFIX Syndication/Atom
  REQUIRED_HEADERS Syndication_Atom_HEADERS
)

install(FILES
    ${Syndication_Atom_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Syndication/Syndication/Atom
    COMPONENT Devel

)

install(FILES
    ${Syndication_Atom_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Syndication/syndication/atom
        COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(Syndication_Atom_APIDOX_SRCS ${Syndication_Atom_HEADERS} PARENT_SCOPE)
