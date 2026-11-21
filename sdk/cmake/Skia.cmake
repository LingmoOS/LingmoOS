# Skia is built by its own GN build inside this build tree and linked
# statically. scripts/fetch-skia.sh puts the pinned sources and GN in place.
set(SKIA_SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/skia)
set(SKIA_BUILD_DIR ${CMAKE_BINARY_DIR}/skia)

if(NOT EXISTS ${SKIA_SOURCE_DIR}/BUILD.gn)
    message(FATAL_ERROR "Skia is missing; run scripts/fetch-skia.sh")
endif()
find_program(SKIA_GN gn PATHS ${SKIA_SOURCE_DIR}/bin NO_DEFAULT_PATH REQUIRED)
find_program(SKIA_NINJA ninja REQUIRED)
find_program(SKIA_CC clang REQUIRED)
find_program(SKIA_CXX clang++ REQUIRED)

# Codecs and fonts come from the system; everything Cutefish does not draw
# with is left out.
set(SKIA_ARGS
    "is_official_build = true"
    "is_debug = false"
    "cc = \"${SKIA_CC}\""
    "cxx = \"${SKIA_CXX}\""
    # Classes derived from Skia types need its typeinfo.
    "extra_cflags_cc = [ \"-frtti\" ]"
    # Skia passes AVX vectors between functions built for AVX on purpose.
    "extra_cflags = [ \"-Wno-psabi\" ]"
    "skia_use_partition_alloc = false"
    "skia_use_gl = true"
    "skia_use_egl = true"
    "skia_use_vulkan = false"
    "skia_enable_graphite = false"
    "skia_enable_pdf = false"
    "skia_enable_skottie = false"
    "skia_enable_svg = true"
    "skia_use_wuffs = false"
    "skia_use_dng_sdk = false"
    "skia_use_piex = false"
    "skia_use_libwebp_decode = false"
    "skia_use_libwebp_encode = false"
    "skia_use_expat = true"
    "skia_use_perfetto = false"
    "skia_enable_fontmgr_custom_directory = false"
    "skia_enable_fontmgr_custom_embedded = false"
    "skia_enable_fontmgr_custom_empty = false"
)
list(JOIN SKIA_ARGS "\n" SKIA_ARGS_CONTENT)
# GN picks up args.gn on its own and regenerates when it changes; CONFIGURE
# keeps the file untouched while the arguments stay the same.
file(CONFIGURE OUTPUT ${SKIA_BUILD_DIR}/args.gn CONTENT "${SKIA_ARGS_CONTENT}\n")

# In link order: each library before the ones it uses.
set(SKIA_LIBRARY_NAMES svg skresources skparagraph skshaper skunicode_icu skunicode_core skia skcms)
set(SKIA_LIBRARIES)
foreach(name ${SKIA_LIBRARY_NAMES})
    list(APPEND SKIA_LIBRARIES ${SKIA_BUILD_DIR}/lib${name}.a)
endforeach()

include(ExternalProject)
ExternalProject_Add(skia-build
    SOURCE_DIR ${SKIA_SOURCE_DIR}
    BINARY_DIR ${SKIA_BUILD_DIR}
    CONFIGURE_COMMAND ${SKIA_GN} gen ${SKIA_BUILD_DIR} --root=${SKIA_SOURCE_DIR}
    BUILD_COMMAND ${SKIA_NINJA} -C ${SKIA_BUILD_DIR}
        skia modules/svg modules/skresources modules/skparagraph modules/skshaper
        modules/skunicode
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
    BUILD_BYPRODUCTS ${SKIA_LIBRARIES}
    USES_TERMINAL_BUILD ON
)

add_library(skia INTERFACE)
add_dependencies(skia skia-build)
target_include_directories(skia SYSTEM INTERFACE ${SKIA_SOURCE_DIR})
# Must match the public defines of the GN build: some change class layouts.
target_compile_definitions(skia INTERFACE
    SK_RELEASE
    SK_GANESH
    SK_GL
    SK_R32_SHIFT=16
    SK_ENABLE_PRECOMPILE
    SK_DISABLE_TRACING
    SK_TYPEFACE_FACTORY_FREETYPE
    SK_FONTMGR_FONTCONFIG_AVAILABLE
    SK_ENABLE_PARAGRAPH
    SK_UNICODE_AVAILABLE
    SK_UNICODE_ICU_IMPLEMENTATION
    SK_SHAPER_PRIMITIVE_AVAILABLE
    SK_SHAPER_HARFBUZZ_AVAILABLE
    SK_SHAPER_UNICODE_AVAILABLE
)
target_link_libraries(skia INTERFACE
    ${SKIA_LIBRARIES}
    PkgConfig::SkiaDependencies
    PkgConfig::Fontconfig
    PkgConfig::Egl
    PkgConfig::GlesV2
    ${CMAKE_DL_LIBS}
)
