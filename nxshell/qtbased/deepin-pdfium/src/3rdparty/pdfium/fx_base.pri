# module bigint
SOURCE_DIR = $$PWD/pdfium/third_party

HEADERS += \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/address_space_randomization.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/oom.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/oom_callback.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/page_allocator.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/page_allocator_constants.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/page_allocator_internal.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/page_allocator_internals_posix.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/page_allocator_internals_win.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_alloc.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_alloc_constants.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_bucket.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_cookie.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_direct_map_extent.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_freelist_entry.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_oom.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_page.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_root_base.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/random.h \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/spin_lock.h \
    $$PWD/pdfium/third_party/base/containers/adapters.h \
    $$PWD/pdfium/third_party/base/debug/alias.h \
    $$PWD/pdfium/third_party/base/memory/aligned_memory.h \
    $$PWD/pdfium/third_party/base/numerics/checked_math.h \
    $$PWD/pdfium/third_party/base/numerics/checked_math_impl.h \
    $$PWD/pdfium/third_party/base/numerics/clamped_math.h \
    $$PWD/pdfium/third_party/base/numerics/clamped_math_impl.h \
    $$PWD/pdfium/third_party/base/numerics/safe_conversions.h \
    $$PWD/pdfium/third_party/base/numerics/safe_conversions_arm_impl.h \
    $$PWD/pdfium/third_party/base/numerics/safe_conversions_impl.h \
    $$PWD/pdfium/third_party/base/numerics/safe_math.h \
    $$PWD/pdfium/third_party/base/numerics/safe_math_arm_impl.h \
    $$PWD/pdfium/third_party/base/numerics/safe_math_clang_gcc_impl.h \
    $$PWD/pdfium/third_party/base/numerics/safe_math_shared_impl.h \
    $$PWD/pdfium/third_party/base/base_export.h \
    $$PWD/pdfium/third_party/base/bits.h \
    $$PWD/pdfium/third_party/base/compiler_specific.h \
    $$PWD/pdfium/third_party/base/immediate_crash.h \
    $$PWD/pdfium/third_party/base/logging.h \
    $$PWD/pdfium/third_party/base/no_destructor.h \
    $$PWD/pdfium/third_party/base/optional.h \
    $$PWD/pdfium/third_party/base/span.h \
    $$PWD/pdfium/third_party/base/stl_util.h \
    $$PWD/pdfium/third_party/base/sys_byteorder.h \
    $$PWD/pdfium/third_party/base/template_util.h

SOURCES += \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/address_space_randomization.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/oom_callback.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/page_allocator.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_alloc.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_bucket.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_oom.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_page.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/partition_root_base.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/random.cc \
    $$PWD/pdfium/third_party/base/allocator/partition_allocator/spin_lock.cc \
    $$PWD/pdfium/third_party/base/debug/alias.cc \
    $$PWD/pdfium/third_party/base/memory/aligned_memory.cc
