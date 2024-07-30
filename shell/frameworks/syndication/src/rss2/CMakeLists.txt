########### install files ###########

ecm_generate_headers(Syndication_Rss2_CamelCase_HEADERS
  HEADER_NAMES
  Category
  Cloud
  Document
  Enclosure
  Image
  Item
  Parser
  Rss2
  Source
  TextInput

  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/..
  PREFIX Syndication/Rss2
  REQUIRED_HEADERS Syndication_Rss2_HEADERS
)
