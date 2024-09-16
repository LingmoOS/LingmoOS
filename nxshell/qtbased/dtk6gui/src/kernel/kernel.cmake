file(GLOB KERNEL_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/kernel/*.h
)
file(GLOB KERNEL_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
set(kernel_SRC 
  ${KERNEL_HEADER}
  ${KERNEL_SOURCE}
)
