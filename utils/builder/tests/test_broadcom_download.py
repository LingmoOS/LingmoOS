# SPDX-License-Identifier: GPL-3.0-or-later
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'tools/ci/download-broadcom.sh'
FILENAME = 'broadcom-sta-dkms_6.30.223.271-32_amd64.deb'

class BroadcomDownload(unittest.TestCase):
    def fixture(self, directory):
        commands = directory / 'bin'
        commands.mkdir()
        mocks = {
            'curl': '''#!/usr/bin/python3
import os, pathlib, sys
if os.environ.get('FAIL_DOWNLOAD'): sys.exit(22)
args=sys.argv[1:]
assert 'https://deb.debian.org/debian/pool/non-free/b/broadcom-sta/broadcom-sta-dkms_6.30.223.271-32_amd64.deb' in args
pathlib.Path(args[args.index('--output')+1]).write_bytes(b'fixture')
''',
            'sha256sum': '''#!/usr/bin/python3
import os,sys
line=sys.stdin.read()
assert line.startswith('0b4e801fda736715cfd56d98a4ddd6a0564c6ebc77c041a58f74642f7630d870  ')
sys.exit(1 if os.environ.get('BAD_HASH') else 0)
''',
            'dpkg-deb': '''#!/usr/bin/python3
import os,sys
values={'Package':'broadcom-sta-dkms','Version':'6.30.223.271-32','Architecture':'amd64'}
field=sys.argv[-1]
print('wrong' if os.environ.get('BAD_FIELD') == field else values[field])
'''
        }
        for name, text in mocks.items():
            file = commands / name
            file.write_text(text)
            file.chmod(0o755)
        return dict(os.environ, PATH=str(commands)+':'+os.environ['PATH'])

    def test_verified_package_and_rerun(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory=Path(tmp); env=self.fixture(directory); output=directory/'local packages'
            for _ in range(2):
                subprocess.run(['bash',str(SCRIPT),str(output)],env=env,check=True,capture_output=True)
                self.assertEqual((output/FILENAME).read_bytes(),b'fixture')
                self.assertEqual(list(output.iterdir()),[output/FILENAME])

    def test_failed_download_or_validation_keeps_existing_package(self):
        for failure in ({'FAIL_DOWNLOAD':'1'},{'BAD_HASH':'1'},
                        *({'BAD_FIELD':field} for field in ('Package','Version','Architecture'))):
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as tmp:
                directory=Path(tmp);env=self.fixture(directory);env.update(failure)
                output=directory/'packages';output.mkdir()
                (output/FILENAME).write_bytes(b'existing')
                result=subprocess.run(['bash',str(SCRIPT),str(output)],env=env,capture_output=True)
                self.assertNotEqual(result.returncode,0)
                self.assertEqual((output/FILENAME).read_bytes(),b'existing')
                self.assertEqual(list(output.iterdir()),[output/FILENAME])

    def test_download_precedes_copy_and_build(self):
        workflow=(ROOT/'.github/workflows/build.yml').read_text()
        self.assertLess(workflow.index('run: bash tools/ci/download-broadcom.sh'),
                        workflow.index('cp -r scripts/* build/'))
        self.assertLess(workflow.index('cp -r scripts/* build/'),workflow.index('sudo ./build.sh'))
        self.assertIn('scripts/lingmo-config/common/packages.chroot',SCRIPT.read_text())

if __name__ == '__main__': unittest.main()
