add_definitions(-DTRANSLATION_DOMAIN=\"kio6_timeline\")

set(kio_timeline_SRCS
  kio_timeline.cpp
  timelinetools.cpp
)

ecm_qt_declare_logging_category(kio_timeline_SRCS
    HEADER kio_timeline_debug.h
    IDENTIFIER Baloo::KIO_TIMELINE
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME kf.kio.workers.timeline
    OLD_CATEGORY_NAMES kf5.kio.kio_timeline kf.kio.slaves.timeline
    DESCRIPTION "Baloo Timeline (KIO)"
    EXPORT BALOO
)

add_library(timeline MODULE ${kio_timeline_SRCS})

target_link_libraries(timeline
  KF6::KIOCore
  KF6::I18n
  KF6::Baloo
)

install(TARGETS timeline DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kio)
