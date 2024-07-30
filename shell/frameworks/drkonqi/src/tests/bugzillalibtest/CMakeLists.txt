set(bugzillalibtest_SRCS bugzillalibtest.cpp)

ecm_qt_declare_logging_category(
    bugzillalibtest_SRCS
    HEADER drkonqi_debug.h
    IDENTIFIER DRKONQI_LOG
    CATEGORY_NAME org.kde.drkonqi
)

add_executable(bugzillalibtest ${bugzillalibtest_SRCS})
ecm_mark_as_test(bugzillalibtest)

target_link_libraries(bugzillalibtest KF6::KIOWidgets KF6::I18n qbugzilla DrKonqiInternal)
