file(GLOB LOG_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/log/LogManager.h
)
set(LOG_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/LogManager.cpp
)

set(log_SRCS
${LOG_HEADER}
${LOG_SOURCE}
)

