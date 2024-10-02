#!/usr/bin/python3

from __future__ import annotations

import dataclasses
import json
import locale
import os
import os.path
import pathlib
import subprocess
import re
import tempfile
from typing import cast

from debian_linux.config_v2 import (
    Config,
    ConfigMerged,
    ConfigMergedDebianarch,
    ConfigMergedFeatureset,
    ConfigMergedFlavour,
)
from debian_linux.dataclasses_deb822 import read_deb822, write_deb822
from debian_linux.debian import \
    PackageBuildprofile, \
    PackageRelationEntry, PackageRelationGroup, \
    VersionLinux, BinaryPackage
from debian_linux.gencontrol import Gencontrol as Base, PackagesBundle, \
    MakeFlags
from debian_linux.utils import Templates

locale.setlocale(locale.LC_CTYPE, "C.UTF-8")


class Gencontrol(Base):
    disable_installer: bool
    disable_signed: bool

    env_flags = [
        ('DEBIAN_KERNEL_DISABLE_INSTALLER', 'disable_installer', 'installer modules'),
        ('DEBIAN_KERNEL_DISABLE_SIGNED', 'disable_signed', 'signed code'),
    ]

    def __init__(
        self,
        config_dirs=[
            pathlib.Path('debian/config'),
            pathlib.Path('debian/config.local'),
        ],
        template_dirs=["debian/templates"],
    ) -> None:
        super().__init__(
            Config.read_orig(config_dirs).merged,
            Templates(template_dirs),
            VersionLinux)
        self.config_dirs = config_dirs
        self.process_changelog()

        for env, attr, desc in self.env_flags:
            setattr(self, attr, False)
            if os.getenv(env):
                if self.changelog[0].distribution == 'UNRELEASED':
                    import warnings
                    warnings.warn(f'Disable {desc} on request ({env} set)')
                    setattr(self, attr, True)
                else:
                    raise RuntimeError(
                        f'Unable to disable {desc} in release build ({env} set)')

    def _setup_makeflags(self, names, makeflags, data) -> None:
        for src, dst, optional in names:
            if src in data or not optional:
                makeflags[dst] = data[src]

    def do_main_setup(
        self,
        config: ConfigMerged,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        super().do_main_setup(config, vars, makeflags)
        makeflags.update({
            'VERSION': self.version.linux_version,
            'UPSTREAMVERSION': self.version.linux_upstream,
            'ABINAME': self.abiname,
            'SOURCEVERSION': self.version.complete,
        })
        makeflags['SOURCE_BASENAME'] = vars['source_basename']
        makeflags['SOURCE_SUFFIX'] = vars['source_suffix']

        # Prepare to generate debian/tests/control
        self.tests_control = list(self.templates.get_tests_control('main.tests-control', vars))

    def do_main_makefile(
        self,
        config: ConfigMerged,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        for featureset in self.config.root_featuresets:
            makeflags_featureset = makeflags.copy()
            makeflags_featureset['FEATURESET'] = featureset.name

            self.bundle.makefile.add_rules(f'source_{featureset.name}',
                                           'source', makeflags_featureset)
            self.bundle.makefile.add_deps('source', [f'source_{featureset.name}'])

        makeflags = makeflags.copy()
        makeflags['ALL_FEATURESETS'] = ' '.join(i.name for i in self.config.root_featuresets)
        super().do_main_makefile(config, vars, makeflags)

    def do_main_packages(
        self,
        config: ConfigMerged,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        self.bundle.add('main', (), makeflags, vars)

        # Only build the metapackages if their names won't exactly match
        # the packages they depend on
        do_meta = config.packages.meta \
            and vars['source_suffix'] != '-' + vars['version']

        if config.packages.docs:
            self.bundle.add('docs', (), makeflags, vars)
            if do_meta:
                self.bundle.add('docs.meta', (), makeflags, vars)
        if config.packages.source:
            self.bundle.add('sourcebin', (), makeflags, vars)
            if do_meta:
                self.bundle.add('sourcebin.meta', (), makeflags, vars)

        if config.packages.libc_dev:
            libcdev_debianarches = set()
            libcdev_kernelarches = set()
            libcdev_multiarches = set()
            for kernelarch in self.config.kernelarchs:
                libcdev_kernelarches.add(kernelarch.name)
                for debianarch in kernelarch.debianarchs:
                    libcdev_debianarches.add(debianarch.name)
                    libcdev_multiarches.add(
                        f'{debianarch.defs_debianarch.multiarch}:{kernelarch.name}'
                    )

            libcdev_makeflags = makeflags.copy()
            libcdev_makeflags['ALL_LIBCDEV_KERNELARCHES'] = ' '.join(sorted(libcdev_kernelarches))
            libcdev_makeflags['ALL_LIBCDEV_MULTIARCHES'] = ' '.join(sorted(libcdev_multiarches))

            for package in self.bundle.add('libc-dev', (), libcdev_makeflags, vars):
                package.provides.extend([
                    PackageRelationGroup(
                        f'{package.name}-{arch}-cross (= ${{binary:Version}})'
                    )
                    for arch in sorted(libcdev_debianarches)
                ])

    def do_indep_featureset_setup(
        self,
        config: ConfigMergedFeatureset,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        makeflags['LOCALVERSION'] = vars['localversion']
        kernel_arches = set()
        for kernelarch in self.config.kernelarchs:
            for debianarch in kernelarch.debianarchs:
                for featureset in debianarch.featuresets:
                    if config.name_featureset in featureset.name:
                        kernel_arches.add(kernelarch.name)
        makeflags['ALL_KERNEL_ARCHES'] = ' '.join(sorted(list(kernel_arches)))

        vars['featureset_desc'] = ''
        if config.name_featureset != 'none':
            desc = config.description
            vars['featureset_desc'] = (' with the %s featureset' %
                                       desc.short[desc.parts[0]])

    def do_indep_featureset_packages(
        self,
        config: ConfigMergedFeatureset,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        self.bundle.add('headers.featureset', (config.name_featureset, ), makeflags, vars)

    def do_arch_setup(
        self,
        config: ConfigMergedDebianarch,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        makeflags['KERNEL_ARCH'] = config.name_kernelarch

    def do_arch_packages(
        self,
        config: ConfigMergedDebianarch,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        arch = config.name

        if not self.disable_signed:
            build_signed = config.build.enable_signed
        else:
            build_signed = False

        if build_signed:
            # Make sure variables remain
            vars['signedtemplate_binaryversion'] = '@signedtemplate_binaryversion@'
            vars['signedtemplate_sourceversion'] = '@signedtemplate_sourceversion@'

            self.bundle.add('signed-template', (arch,), makeflags, vars, arch=arch)

            bundle_signed = self.bundles[f'signed-{arch}'] = \
                PackagesBundle(f'signed-{arch}', 'signed.source.control', vars, self.templates)

            with bundle_signed.open('source/lintian-overrides', 'w') as f:
                f.write(self.substitute(
                    self.templates.get('signed.source.lintian-overrides'), vars))

            with bundle_signed.open('changelog.head', 'w') as f:
                dist = self.changelog[0].distribution
                urgency = self.changelog[0].urgency
                f.write(f'''\
linux-signed-{vars['arch']} (@signedtemplate_sourceversion@) {dist}; urgency={urgency}

  * Sign kernel from {self.changelog[0].source} @signedtemplate_binaryversion@
''')

        if config.packages.source and list(config.featuresets):
            self.bundle.add('config', (arch, ), makeflags, vars)

        if config.packages.tools_unversioned:
            self.bundle.add('tools-unversioned', (arch, ), makeflags, vars)

        if config.packages.tools_versioned:
            self.bundle.add('tools-versioned', (arch, ), makeflags, vars)

    def do_featureset_setup(
        self,
        featureset: ConfigMergedFeatureset,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        vars['localversion_headers'] = vars['localversion']
        makeflags['LOCALVERSION_HEADERS'] = vars['localversion_headers']

    def do_flavour_setup(
        self,
        config: ConfigMergedFlavour,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        vars['flavour'] = vars['localversion'][1:]
        vars['class'] = config.description.hardware or ''
        vars['longclass'] = config.description.hardware_long or vars['class']

        vars['localversion-image'] = vars['localversion']

        vars['image-stem'] = cast(str, config.build.kernel_stem)

        if t := config.build.cflags:
            makeflags['KCFLAGS'] = t
        makeflags['COMPILER'] = config.build.compiler
        if t := config.build.compiler_gnutype:
            makeflags['KERNEL_GNU_TYPE'] = t
        if t := config.build.compiler_gnutype_compat:
            makeflags['COMPAT_GNU_TYPE'] = t
        makeflags['IMAGE_FILE'] = config.build.kernel_file
        makeflags['IMAGE_INSTALL_STEM'] = config.build.kernel_stem

        makeflags['LOCALVERSION'] = vars['localversion']
        makeflags['LOCALVERSION_IMAGE'] = vars['localversion-image']

    def do_flavour_packages(
        self,
        config: ConfigMergedFlavour,
        vars: dict[str, str],
        makeflags: MakeFlags,
    ) -> None:
        arch = config.name_debianarch
        ruleid = (arch, config.name_featureset, config.name_flavour)

        packages_headers = (
            self.bundle.add('headers', ruleid, makeflags, vars, arch=arch)
        )
        assert len(packages_headers) == 1

        do_meta = config.packages.meta

        relation_compiler = PackageRelationEntry(cast(str, config.build.compiler))

        relation_compiler_header = PackageRelationGroup([relation_compiler])

        # Generate compiler build-depends for native:
        # gcc-13 [arm64] <!cross !pkg.linux.nokernel>
        self.bundle.source.build_depends_arch.merge([
            PackageRelationEntry(
                relation_compiler,
                arches={arch},
                restrictions='<!cross !pkg.linux.nokernel>',
            )
        ])

        # Generate compiler build-depends for cross:
        # gcc-13-aarch64-linux-gnu [arm64] <cross !pkg.linux.nokernel>
        self.bundle.source.build_depends_arch.merge([
            PackageRelationEntry(
                relation_compiler,
                name=f'{relation_compiler.name}-{config.defs_debianarch.gnutype_package}',
                arches={arch},
                restrictions='<cross !pkg.linux.nokernel>',
            )
        ])

        # Generate compiler build-depends for kernel:
        # gcc-13-hppa64-linux-gnu [hppa] <!pkg.linux.nokernel>
        if gnutype := config.build.compiler_gnutype:
            if gnutype != config.defs_debianarch.gnutype:
                self.bundle.source.build_depends_arch.merge([
                    PackageRelationEntry(
                        relation_compiler,
                        name=f'{relation_compiler.name}-{gnutype.replace("_", "-")}',
                        arches={arch},
                        restrictions='<!pkg.linux.nokernel>',
                    )
                ])

        # Generate compiler build-depends for compat:
        # gcc-arm-linux-gnueabihf [arm64] <!pkg.linux.nokernel>
        # XXX: Linux uses various definitions for this, all ending with "gcc", not $CC
        if gnutype := config.build.compiler_gnutype_compat:
            if gnutype != config.defs_debianarch.gnutype:
                self.bundle.source.build_depends_arch.merge([
                    PackageRelationEntry(
                        f'gcc-{gnutype.replace("_", "-")}',
                        arches={arch},
                        restrictions='<!pkg.linux.nokernel>',
                    )
                ])

        packages_own = []

        if not self.disable_signed:
            build_signed = config.build.enable_signed
        else:
            build_signed = False

        if build_signed:
            bundle_signed = self.bundles[f'signed-{arch}']
        else:
            bundle_signed = self.bundle

        vars.setdefault('desc', '')

        if build_signed:
            packages_image_unsigned = (
                self.bundle.add('image-unsigned', ruleid, makeflags, vars, arch=arch)
            )
            packages_image = packages_image_unsigned[:]
            packages_image.extend(
                bundle_signed.add('signed.image', ruleid, makeflags, vars, arch=arch)
            )

        else:
            packages_image = packages_image_unsigned = (
                bundle_signed.add('image', ruleid, makeflags, vars, arch=arch)
            )

        for field in ('Depends', 'Provides', 'Suggests', 'Recommends',
                      'Conflicts', 'Breaks'):
            for i in getattr(config.relations.image, field.lower(), []):
                for package_image in packages_image:
                    getattr(package_image, field.lower()).merge(
                        PackageRelationGroup(i, arches={arch})
                    )

        for field in ('Depends', 'Suggests', 'Recommends'):
            for i in getattr(config.relations.image, field.lower(), []):
                group = PackageRelationGroup(i, arches={arch})
                for entry in group:
                    if entry.operator is not None:
                        entry.operator = -entry.operator
                        for package_image in packages_image:
                            package_image.breaks.append(PackageRelationGroup([entry]))

        if desc_parts := config.description.parts:
            # XXX: Workaround, we need to support multiple entries of the same
            # name
            parts = list(set(desc_parts))
            parts.sort()
            for package_image in packages_image:
                desc = package_image.description
                for part in parts:
                    desc.append(config.description.long[part])
                    desc.append_short(config.description.short[part])

        packages_headers[0].depends.merge(relation_compiler_header)
        packages_own.extend(packages_image)
        packages_own.extend(packages_headers)

        # The image meta-packages will depend on signed linux-image
        # packages where applicable, so should be built from the
        # signed source packages The header meta-packages will also be
        # built along with the signed packages, to create a dependency
        # relationship that ensures src:linux and src:linux-signed-*
        # transition to testing together.
        if do_meta:
            packages_meta = (
                bundle_signed.add('image.meta', ruleid, makeflags, vars, arch=arch)
            )
            assert len(packages_meta) == 1
            packages_meta += (
                bundle_signed.add(build_signed and 'signed.headers.meta' or 'headers.meta',
                                  ruleid, makeflags, vars, arch=arch)
            )
            assert len(packages_meta) == 2

            if (
                config.defs_flavour.is_default
                and not self.vars['source_suffix']
            ):
                packages_meta[0].provides.append('linux-image-generic')
                packages_meta[1].provides.append('linux-headers-generic')

            packages_own.extend(packages_meta)

        if config.build.enable_vdso:
            makeflags['VDSO'] = True

        packages_own.extend(
            self.bundle.add('image-dbg', ruleid, makeflags, vars, arch=arch)
        )
        if do_meta:
            packages_own.extend(
                self.bundle.add('image-dbg.meta', ruleid, makeflags, vars, arch=arch)
            )

        if (
            config.defs_flavour.is_default
            # XXX
            and not self.vars['source_suffix']
        ):
            packages_own.extend(
                self.bundle.add('image-extra-dev', ruleid, makeflags, vars, arch=arch)
            )

        # In a quick build, only build the quick flavour (if any).
        if not config.defs_flavour.is_quick:
            for package in packages_own:
                package.build_profiles[0].neg.add('pkg.linux.quick')

        tests_control_image = self.templates.get_tests_control('image.tests-control', vars)
        for c in tests_control_image:
            c.depends.extend(
                [i.name for i in packages_image_unsigned]
            )

        tests_control_headers = self.templates.get_tests_control('headers.tests-control', vars)
        for c in tests_control_headers:
            c.depends.extend(
                [i.name for i in packages_headers] +
                [i.name for i in packages_image_unsigned]
            )

        self.tests_control.extend(tests_control_image)
        self.tests_control.extend(tests_control_headers)

        kconfig = []
        for c in config.config:
            for d in self.config_dirs:
                if (f := d / c).exists():
                    kconfig.append(str(f))
        makeflags['KCONFIG'] = ' '.join(kconfig)
        makeflags['KCONFIG_OPTIONS'] = ''
        # Add "salt" to fix #872263
        makeflags['KCONFIG_OPTIONS'] += \
            ' -o "BUILD_SALT=\\"%(abiname)s%(localversion)s\\""' % vars

        merged_config = ('debian/build/config.%s_%s_%s' %
                         (config.name_debianarch, config.name_featureset, config.name_flavour))
        self.bundle.makefile.add_cmds(merged_config,
                                      ["$(MAKE) -f debian/rules.real %s %s" %
                                       (merged_config, makeflags)])

        if (
            config.name_featureset == 'none'
            and not self.disable_installer
            and config.packages.installer
        ):
            with tempfile.TemporaryDirectory(prefix='linux-gencontrol') as config_dir:
                base_path = pathlib.Path('debian/installer').absolute()
                config_path = pathlib.Path(config_dir)
                (config_path / 'modules').symlink_to(base_path / 'modules')
                (config_path / 'package-list').symlink_to(base_path / 'package-list')

                with (config_path / 'kernel-versions').open('w') as versions:
                    versions.write(f'{arch} - {vars["flavour"]} - - -\n')

                # Add udebs using kernel-wedge
                kw_env = os.environ.copy()
                kw_env['KW_DEFCONFIG_DIR'] = config_dir
                kw_env['KW_CONFIG_DIR'] = config_dir
                kw_proc = subprocess.Popen(
                    ['kernel-wedge', 'gen-control', vars['abiname']],
                    stdout=subprocess.PIPE,
                    text=True,
                    env=kw_env)
                assert kw_proc.stdout is not None
                udeb_packages_base = list(read_deb822(BinaryPackage, kw_proc.stdout))
                kw_proc.wait()
                if kw_proc.returncode != 0:
                    raise RuntimeError('kernel-wedge exited with code %d' %
                                       kw_proc.returncode)

            udeb_packages = [
                dataclasses.replace(
                    package_base,
                    # kernel-wedge currently chokes on Build-Profiles so add it now
                    build_profiles=PackageBuildprofile.parse(
                        '<!noudeb !pkg.linux.nokernel !pkg.linux.quick>',
                    ),
                    meta_rules_target='installer',
                )
                for package_base in udeb_packages_base
            ]

            makeflags_local = makeflags.copy()
            makeflags_local['IMAGE_PACKAGE_NAME'] = udeb_packages[0].name

            bundle_signed.add_packages(
                udeb_packages,
                (config.name_debianarch, config.name_featureset, config.name_flavour),
                makeflags_local, arch=arch,
            )

            if build_signed:
                # XXX This is a hack to exclude the udebs from
                # the package list while still being able to
                # convince debhelper and kernel-wedge to go
                # part way to building them.
                udeb_packages = [
                    dataclasses.replace(
                        package_base,
                        # kernel-wedge currently chokes on Build-Profiles so add it now
                        build_profiles=PackageBuildprofile.parse(
                            '<pkg.linux.udeb-unsigned-test-build !noudeb'
                            ' !pkg.linux.nokernel !pkg.linux.quick>',
                        ),
                        meta_rules_target='installer-test',
                    )
                    for package_base in udeb_packages_base
                ]

                self.bundle.add_packages(
                    udeb_packages,
                    (config.name_debianarch, config.name_featureset, config.name_flavour),
                    makeflags_local, arch=arch, check_packages=False,
                )

    def process_changelog(self) -> None:
        version = self.version = self.changelog[0].version

        if self.changelog[0].distribution == 'UNRELEASED':
            self.abiname = f'{version.linux_upstream}+unreleased'
        elif self.changelog[0].distribution == 'experimental':
            self.abiname = f'{version.linux_upstream}'
        elif version.linux_revision_backports:
            self.abiname = f'{version.linux_upstream_full}+bpo'
        else:
            self.abiname = f'{version.linux_upstream_full}'

        self.vars = {
            'upstreamversion': self.version.linux_upstream,
            'version': self.version.linux_version,
            'version_complete': self.version.complete,
            'source_basename': re.sub(r'-[\d.]+$', '',
                                      self.changelog[0].source),
            'source_upstream': self.version.upstream,
            'source_package': self.changelog[0].source,
            'abiname': self.abiname,
        }
        self.vars['source_suffix'] = \
            self.changelog[0].source[len(self.vars['source_basename']):]

        distribution = self.changelog[0].distribution
        if distribution in ('unstable', ):
            if version.linux_revision_experimental or \
               version.linux_revision_backports or \
               version.linux_revision_other:
                raise RuntimeError("Can't upload to %s with a version of %s" %
                                   (distribution, version))
        if distribution in ('experimental', ):
            if not version.linux_revision_experimental:
                raise RuntimeError("Can't upload to %s with a version of %s" %
                                   (distribution, version))
        if distribution.endswith('-security') or distribution.endswith('-lts'):
            if version.linux_revision_backports or \
               version.linux_revision_other:
                raise RuntimeError("Can't upload to %s with a version of %s" %
                                   (distribution, version))
        if distribution.endswith('-backports'):
            if not version.linux_revision_backports:
                raise RuntimeError("Can't upload to %s with a version of %s" %
                                   (distribution, version))

    def write(self) -> None:
        super().write()
        self.write_tests_control()
        self.write_signed()

    def write_signed(self) -> None:
        for bundle in self.bundles.values():
            pkg_sign_entries = {}

            for p in bundle.packages.values():
                if not isinstance(p, BinaryPackage):
                    continue

                if pkg_sign_pkg := p.meta_sign_package:
                    pkg_sign_entries[pkg_sign_pkg] = {
                        'trusted_certs': [],
                        'files': [
                            {
                                'sig_type': e.split(':', 1)[-1],
                                'file': e.split(':', 1)[0],
                            }
                            for e in p.meta_sign_files
                        ],
                    }

            if pkg_sign_entries:
                with bundle.path('files.json').open('w') as f:
                    json.dump({'packages': pkg_sign_entries}, f, indent=2)

    def write_tests_control(self) -> None:
        with open("debian/tests/control", 'w') as f:
            write_deb822(self.tests_control, f)


if __name__ == '__main__':
    Gencontrol()()
