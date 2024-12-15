if(NOT TARGET uuid)

  add_definitions(
      -DHAVE_INTTYPES_H
      -DHAVE_UNISTD_H
      -DHAVE_ERRNO_H
      -DHAVE_NETINET_IN_H
      -DHAVE_SYS_IOCTL_H
      -DHAVE_SYS_MMAN_H
      -DHAVE_SYS_MOUNT_H
      -DHAVE_SYS_PRCTL_H
      -DHAVE_SYS_RESOURCE_H
      -DHAVE_SYS_SELECT_H
      -DHAVE_SYS_STAT_H
      -DHAVE_SYS_TYPES_H
      -DHAVE_STDLIB_H
      -DHAVE_STRDUP
      -DHAVE_MMAP
      -DHAVE_UTIME_H
      -DHAVE_GETPAGESIZE
      -DHAVE_LSEEK64
      -DHAVE_LSEEK64_PROTOTYPE
      -DHAVE_EXT2_IOCTLS
      -DHAVE_LINUX_FD_H
      -DHAVE_TYPE_SSIZE_T
      -DHAVE_SYS_TIME_H
      -DHAVE_SYS_PARAM_H
      -DHAVE_SYSCONF
  )

  # Module library
  file(GLOB SOURCE_FILES "uuid/*.c")
  add_library(uuid STATIC ${SOURCE_FILES})
  # 为特定目标添加编译选项
  target_compile_options(uuid PRIVATE -fdeclspec)

  # Module folder
  set_target_properties(uuid PROPERTIES FOLDER "modules/uuid")

endif()
