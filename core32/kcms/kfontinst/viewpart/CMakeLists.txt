include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_BINARY_DIR}
     )

set(kfontviewpart_PART_SRCS ${libkfontinstview_SRCS} ${libkfontinstdbusiface_SRCS} FontViewPart.cpp )

add_library(kfontviewpart MODULE ${kfontviewpart_PART_SRCS})

target_link_libraries(kfontviewpart KF5::Archive KF5::Parts KF5::XmlGui kfontinstui kfontinst )

install(TARGETS kfontviewpart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/parts )

install( FILES kfontviewpart.rc DESTINATION  ${KDE_INSTALL_KXMLGUIDIR}/kfontview )
