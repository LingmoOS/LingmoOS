set(BACKTRACEPARSER_SRCS
    backtraceparser.cpp
    backtraceparsergdb.cpp
    backtraceparsernull.cpp
    backtraceparserlldb.cpp
    backtraceparser.h
    backtraceparsergdb.h
    backtraceparsernull.h
    backtraceparserlldb.h
)

ecm_qt_declare_logging_category(
    BACKTRACEPARSER_SRCS
    HEADER drkonqi_parser_debug.h
    IDENTIFIER DRKONQI_PARSER_LOG
    CATEGORY_NAME org.kde.drkonqi.parser
    DESCRIPTION "drkonqi parser"
    EXPORT DRKONQI
)

add_library(drkonqi_backtrace_parser STATIC ${BACKTRACEPARSER_SRCS})
target_link_libraries(drkonqi_backtrace_parser PUBLIC Qt::Core)
