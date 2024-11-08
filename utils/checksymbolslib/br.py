import os
import re


ignored_directories = [
    'support/testing/',
]
# Makefile
symbols_used_only_in_source_code = [
    'LINGMO_USE_CCACHE',
]
# package/skeleton/Config.in
symbols_used_only_for_host_variant = [
    'LINGMO_PACKAGE_SKELETON',
]
# Makefile
# package/pkg-generic.mk
symbols_defined_only_at_command_line = [
    'LINGMO_GRAPH_ALT',
    'LINGMO_GRAPH_DEPS_OPTS',
    'LINGMO_GRAPH_DOT_OPTS',
    'LINGMO_GRAPH_OUT',
    'LINGMO_GRAPH_SIZE_OPTS',
    'LINGMO_INSTRUMENTATION_SCRIPTS',
]
# Makefile
symbols_defined_only_when_using_br2_external = [
    'LINGMO_EXTERNAL',
    'LINGMO_EXTERNAL_DIRS',
    'LINGMO_EXTERNAL_MKS',
    'LINGMO_EXTERNAL_NAMES',
]
# boot/barebox/barebox.mk
symbols_defined_only_for_barebox_variant = [
    'LINGMO_TARGET_BAREBOX_AUX_BAREBOXENV',
]
# toolchain/toolchain/toolchain.mk
# toolchain/toolchain-buildroot/toolchain-buildroot.mk
# toolchain/toolchain-bare-metal-buildroot/toolchain-bare-metal-buildroot.mk
symbols_not_defined_for_fake_virtual_packages = [
    'LINGMO_PACKAGE_HAS_TOOLCHAIN',
    'LINGMO_PACKAGE_HAS_TOOLCHAIN_BUILDROOT',
    'LINGMO_PACKAGE_HAS_TOOLCHAIN_BARE_METAL_BUILDROOT',
    'LINGMO_PACKAGE_PROVIDES_TOOLCHAIN',
    'LINGMO_PACKAGE_PROVIDES_TOOLCHAIN_BUILDROOT',
    'LINGMO_PACKAGE_PROVIDES_TOOLCHAIN_BARE_METAL_BUILDROOT',
]
# fs/common.mk
suffixes_not_defined_for_all_rootfs_types = [
    '_BZIP2',
    '_GZIP',
    '_LZ4',
    '_LZMA',
    '_LZO',
    '_XZ',
    '_ZSTD',
]
# fs/common.mk
rootfs_prefix = 'LINGMO_TARGET_ROOTFS_'
# package/pkg-generic.mk
package_prefix = 'LINGMO_PACKAGE_'
# package/pkg-generic.mk
boot_prefix = 'LINGMO_TARGET_'
# package/pkg-generic.mk
toolchain_prefix = 'LINGMO_'
# boot/barebox/barebox.mk
barebox_infra_suffixes = [
    '',
    '_BAREBOXENV',
    '_BOARD_DEFCONFIG',
    '_CONFIG_FRAGMENT_FILES',
    '_CUSTOM_CONFIG_FILE',
    '_CUSTOM_EMBEDDED_ENV_PATH',
    '_CUSTOM_ENV',
    '_CUSTOM_ENV_PATH',
    '_IMAGE_FILE',
    '_USE_CUSTOM_CONFIG',
    '_USE_DEFCONFIG',
]
re_kconfig_symbol = re.compile(r'\b(LINGMO_\w+)\b')
# Example lines to be handled:
# config LINGMO_TOOLCHAIN_EXTERNAL_PREFIX
# menuconfig LINGMO_PACKAGE_GST1_PLUGINS_BASE
re_kconfig_config = re.compile(r'^\s*(menu|)config\s+(LINGMO_\w+)')
# Example lines to be handled:
# default "uclibc" if LINGMO_TOOLCHAIN_BUILDROOT_UCLIBC
# default LINGMO_TARGET_GRUB2_BUILTIN_MODULES if LINGMO_TARGET_GRUB2_BUILTIN_MODULES != ""
# default y if LINGMO_HOSTARCH = "powerpc"
re_kconfig_default = re.compile(r'^\s*default\s')
re_kconfig_default_before_conditional = re.compile(r'^.*\bif\b')
re_kconfig_default_legacy_comment = re.compile(r'#\s*legacy')
# Example lines to be handled:
# depends on !(LINGMO_TOOLCHAIN_USES_GLIBC && LINGMO_TOOLCHAIN_USES_MUSL)
# depends on LINGMO_HOSTARCH = "x86_64" || LINGMO_HOSTARCH = "x86"
re_kconfig_depends = re.compile(r'^\s*depends on\s')
# Example lines to be handled:
# select LINGMO_PACKAGE_HOST_NODEJS if LINGMO_PACKAGE_NODEJS_MODULES_ADDITIONAL != ""
# select LINGMO_PACKAGE_LIBDRM if !(LINGMO_arm && LINGMO_PACKAGE_IMX_GPU_VIV_OUTPUT_FB)
# select LINGMO_PACKAGE_OPENSSL if !(LINGMO_PACKAGE_GNUTLS || LINGMO_PACKAGE_MBEDTLS)
re_kconfig_select = re.compile(r'^\s*select\s')
re_kconfig_select_conditional = re.compile(r'\bif\s.*')
# Example lines to be handled:
# if !LINGMO_SKIP_LEGACY
# if (LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX51 || LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX53)
# if LINGMO_PACKAGE_HAS_LUAINTERPRETER && !LINGMO_STATIC_LIBS
# if LINGMO_PACKAGE_QEMU_CUSTOM_TARGETS = ""
re_kconfig_if = re.compile(r'^\s*if\s')
# Example lines to be handled:
# source "$LINGMO_BASE_DIR/.br2-external.in.jpeg"
re_kconfig_source = re.compile(r'^\s*source\b')

re_kconfig_choice = re.compile(r'^\s*choice\b')
re_kconfig_endchoice = re.compile(r'^\s*endchoice\b')
re_makefile_eval = re.compile(r'^\s*\$\(eval\b')
re_menu = re.compile(r'^\s*menu\b')
re_endmenu = re.compile(r'^\s*endmenu\b')
re_comments = re.compile(r'#.*$')
re_legacy_special_comment = re.compile(r'#.*(LINGMO_\w+)\s.*still referenced')
re_host_symbol = re.compile(r'(LINGMO_PACKAGE_HOST_\w+|LINGMO_PACKAGE_HAS_HOST_\w+)')
re_makefile_symbol_usage = re.compile(r'\$\((LINGMO_\w+)\)')
re_makefile_symbol_export = re.compile(r'export\s*(LINGMO_\w+)')
re_makefile_symbol_attribution = re.compile(r'^\s*(LINGMO_\w+)\s*[?:=]')


def get_package_from_filename(filename):
    package = os.path.basename(filename)[:-3].upper().replace('-', '_')
    return package


def is_an_optional_symbol_for_a_roofts(symbol):
    if not symbol.startswith(rootfs_prefix):
        return False
    for sufix in suffixes_not_defined_for_all_rootfs_types:
        if symbol.endswith(sufix):
            return True
    return False


def file_belongs_to_an_ignored_diretory(filename):
    for d in ignored_directories:
        if filename.startswith(d):
            return True
    return False
