#!/usr/bin/python3

import itertools
import os
import pathlib
import sys

from debian_linux.config_v2 import Config
from debian_linux.kconfig import KconfigFile


class CheckSecureBootConfig:
    def __init__(self, config, dir, *_):
        self.config = config
        self.dir = pathlib.Path(dir)

    def __call__(self, out):
        fail = 0

        if self.config.build.enable_signed \
           and not os.getenv('DEBIAN_KERNEL_DISABLE_SIGNED'):
            kconfig = KconfigFile()
            with (self.dir / '.config').open() as fh:
                kconfig.read(fh)

            for name, value in [('EFI_STUB', True),
                                ('LOCK_DOWN_IN_EFI_SECURE_BOOT', True),
                                ('SYSTEM_TRUSTED_KEYS', '""')]:
                if name not in kconfig:
                    out.write(f'Secure Boot: CONFIG_{name} is not defined\n')
                    fail = 1
                elif kconfig[name].value != value:
                    out.write(f'Secure Boot: CONFIG_{name} has wrong value:'
                              f' {kconfig[name].value}\n')
                    fail = 1

        return fail


class Main(object):

    checks = {
        'setup': [CheckSecureBootConfig],
        'build': [],
    }

    def __init__(self, dir, arch, featureset, flavour, phase):
        self.args = dir, arch, featureset, flavour
        self.phase = phase

        config_dirs = [
            pathlib.Path('debian/config'),
            pathlib.Path('debian/config.local'),
        ]
        top_config = Config.read_orig(config_dirs).merged
        arch_config = next(
            ac
            for ac in itertools.chain.from_iterable(
                kac.debianarchs for kac in top_config.kernelarchs)
            if ac.name == arch
        )
        fs_config = next(fsc for fsc in arch_config.featuresets
                         if fsc.name == featureset)
        self.config = next(fc for fc in fs_config.flavours
                           if fc.name == flavour)

    def __call__(self):
        fail = 0

        for c in self.checks[self.phase]:
            fail |= c(self.config, *self.args)(sys.stdout)

        return fail


if __name__ == '__main__':
    sys.exit(Main(*sys.argv[1:])())
