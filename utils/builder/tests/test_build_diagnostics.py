# SPDX-License-Identifier: GPL-3.0-or-later
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]

class Diagnostics(unittest.TestCase):
    def test_logs_collected_without_signing_keys(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); build = root/'build'; output = root/'diagnostics'
            files = {'build.log':'build failure',
                     'chroot/var/log/dpkg.log':'package state',
                     'chroot/var/lib/dkms/broadcom-sta/6.30.223.271/build/make.log':'compiler error',
                     'chroot/var/lib/dkms/mok.key':'private signing key'}
            for name, data in files.items():
                p = build/name; p.parent.mkdir(parents=True, exist_ok=True); p.write_text(data)
            commands = root/'bin';commands.mkdir()
            mock = commands/'chroot';mock.write_text('#!/bin/sh\necho mock-toolchain\n');mock.chmod(0o755)
            env = dict(os.environ, PATH=str(commands)+':'+os.environ['PATH'])
            subprocess.run(['bash',str(ROOT/'tools/ci/collect-build-diagnostics.sh'),str(build),str(output)],env=env,check=True,capture_output=True)
            self.assertEqual((output/'build.log').read_text(),'build failure')
            self.assertEqual((output/'chroot/var/lib/dkms/broadcom-sta/6.30.223.271/build/make.log').read_text(),'compiler error')
            self.assertFalse((output/'chroot/var/lib/dkms/mok.key').exists())
            self.assertIn('mock-toolchain',(output/'toolchain.txt').read_text())

    def test_missing_chroot_does_not_mask_original_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            subprocess.run(['bash',str(ROOT/'tools/ci/collect-build-diagnostics.sh'),str(Path(tmp)/'absent'),str(Path(tmp)/'out')],check=True,capture_output=True)

    def test_no_recommends_build_has_explicit_compiler(self):
        config=(ROOT/'scripts/auto/config').read_text()
        packages=(ROOT/'scripts/lingmo-config/common/package-lists/linux-headers.list.chroot').read_text().splitlines()
        self.assertIn('--apt-recommends false',config)
        self.assertIn('build-essential',packages)
        self.assertIn('gcc-14',packages)
        workflow=(ROOT/'.github/workflows/build.yml').read_text()
        self.assertLess(workflow.index('sudo ./build.sh'),workflow.index('name: Collect failed build diagnostics'))
        self.assertIn('name: Upload failed build diagnostics\n        if: failure()',workflow)

if __name__ == '__main__': unittest.main()
