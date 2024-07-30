# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

import unittest
from unittest.mock import MagicMock, patch
from chai import Chai
import sys
import os
from pathlib import Path

os.environ['DRKONQI_VERSION'] = '1.2.3'

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(f'{SCRIPT_DIR}/python/')
sys.path.append(f'{os.path.dirname(SCRIPT_DIR)}/data/')

import gdb_preamble.preamble as preamble
import gdb

class PreambleTest(Chai):
    @classmethod
    def setUpClass(self):
        pass

    def setUp(self):
        preamble.SentryImage._objfiles = {}
        super(PreambleTest, self).setUp()

    def frame(self):
        self.mock(gdb, 'solib_name')
        self.expect(gdb.solib_name).args(0x1).returns(
            f'{Path.home()}/foo.so').at_least(0)

        symbol = MagicMock()
        symbol.__str__.return_value = 'i'
        symbol.value.return_value = 123

        symtab = MagicMock()
        symtab.fullname = MagicMock(return_value=f'{Path.home()}/foo.so')

        sal = MagicMock()
        sal.symtab = symtab
        sal.line = 0

        rax = self.mock(MagicMock)
        rax.name = "rax"
        self.expect(rax, 'startswith').args('ymm').returns(False).at_least(0)

        ymm0 = self.mock(MagicMock)
        ymm0.name = "ymm0"
        self.expect(ymm0, 'startswith').args('ymm').returns(True).at_least(0)

        architecture = self.mock()
        self.expect(architecture, 'registers').returns(['ymm0', rax]).at_least(0)

        rax_value = self.mock()
        self.expect(rax_value, 'format_string').args(format='x').returns('0x2').at_least(0)

        f = self.mock()
        self.expect(f, 'block').returns([symbol]).at_least(0)
        self.expect(f, 'find_sal').returns(sal).at_least(0)
        self.expect(f, 'name').returns('main').at_least(0)
        self.expect(f, 'pc').returns(0x1).at_least(0)
        self.expect(f, 'architecture').returns(architecture).at_least(0)
        self.expect(f, 'read_register').args(rax).returns(rax_value).at_least(0)
        self.expect(f, 'read_register').args(ymm0).times(0) # never
        self.expect(f, 'type').returns(0).at_least(0) # NORMAL_FRAME
        return f

    def test_sentry_variables(self):
        variables = preamble.SentryVariables(self.frame())
        self.assert_equal({'i': '123'}, variables.to_dict())

    def test_sentry_frame(self):
        sentry_frame = preamble.SentryFrame(self.frame())
        self.assert_equal({'filename': '$HOME/foo.so',
                           'function': 'main',
                           'instruction_addr': '0x1',
                           'lineno': 1, # NOTE: sentry starts at 1, gdb at 0
                           'package': '$HOME/foo.so',
                           'vars': {'i': '123'}}, sentry_frame.to_dict())

    def test_sentry_registers(self):
        registers = preamble.SentryRegisters(self.frame())
        self.assert_equal({'rax': '0x2'}, registers.to_dict())

    def test_sentry_thread(self):
        the_frame = self.frame()

        thread = self.mock()
        self.expect(thread.switch)
        thread.ptid = [123, 456, 789]
        thread.name = "TheThread"
        self.expect(thread.is_exited).returns(False)

        self.mock(gdb, 'newest_frame')
        self.expect(gdb.newest_frame).returns(the_frame)
        gdb.SIGTRAMP_FRAME = 4

        self.mock(gdb, 'FrameIterator')
        self.mock(gdb, 'FrameIterator.FrameIterator')
        self.expect(gdb.FrameIterator.FrameIterator).args(the_frame).returns([the_frame])

        thread = preamble.SentryThread(thread, is_crashed=False)
        self.assert_equal({'crashed': False,
                           'current': False,
                           'held_locks': {},
                           'state': None,
                            'id': 456,
                            'main': False,
                            'name': 'TheThread',
                            'stacktrace': {'frames': [{'filename': '$HOME/foo.so',
                                                        'function': 'main',
                                                        'instruction_addr': '0x1',
                                                        'lineno': 1, # NOTE: sentry starts at 1, gdb at 0
                                                        'package': '$HOME/foo.so',
                                                        'vars': {'i': '123'}}],
                                            'registers': {'rax': '0x2'}}}, thread.to_dict())

    def test_sentry_images(self):
        proc_mappings = """
process 207700
Mapped address spaces:

          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555554000     0x555555556000     0x2000        0x0  r--p   /usr/bin/kwrite
      0x555555556000     0x555555558000     0x2000     0x2000  r-xp   /usr/bin/kwrite
      0x555555558000     0x555555559000     0x1000     0x4000  r--p   /usr/bin/kwrite
      0x55555555a000     0x55555555b000     0x1000     0x5000  r--p   /usr/bin/kwrite
      0x55555555b000     0x55555555c000     0x1000     0x6000  rw-p   /usr/bin/kwrite
      0x55555555c000     0x55555637e000   0xe22000        0x0  rw-p   [heap]
      0x7fffc0000000     0x7fffc0021000    0x21000        0x0  rw-p
      0x7fffc0021000     0x7fffc4000000  0x3fdf000        0x0  ---p
      0x7fffc7000000     0x7fffc703c000    0x3c000        0x0  r--p   /usr/lib/x86_64-linux-gnu/libx265.so.199
      0x7fffc703c000     0x7fffc7ed4000   0xe98000    0x3c000  r-xp   /usr/lib/x86_64-linux-gnu/libx265.so.199
      0x7fffc7ed4000     0x7fffc7f4f000    0x7b000   0xed4000  r--p   /usr/lib/x86_64-linux-gnu/libx265.so.199
      0x7fffc7f4f000     0x7fffc7f52000     0x3000   0xf4e000  r--p   /usr/lib/x86_64-linux-gnu/libx265.so.199
      0x7fffc7f52000     0x7fffc7f55000     0x3000   0xf51000  rw-p   /usr/lib/x86_64-linux-gnu/libx265.so.199
      0x7fffd5ff6000     0x7fffd5ffc000     0x6000        0x0  r--s   /var/cache/fontconfig/68a526f6-7ccc-4a32-af00-647cd10884c1-le64.cache-7
      0x7fffdc000000     0x7fffdc001000     0x1000        0x0  r--s   /home/me/.local/share/mime/mime.cache
      0x7fffdc476000     0x7fffdc676000   0x200000 0x118ded000  rw-s   /dev/dri/renderD128
      0x7ffff6200000     0x7ffff6228000    0x28000        0x0  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff6228000     0x7ffff63bd000   0x195000    0x28000  r-xp   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff63bd000     0x7ffff6415000    0x58000   0x1bd000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff6415000     0x7ffff6419000     0x4000   0x214000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff6419000     0x7ffff641b000     0x2000   0x218000  rw-p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7fbd000     0x7ffff7fc1000     0x4000        0x0  r--p   [vvar]
      0x7ffff7fc1000     0x7ffff7fc3000     0x2000        0x0  r-xp   [vdso]
      0x7ffff7fc3000     0x7ffff7fc5000     0x2000        0x0  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7fc5000     0x7ffff7fef000    0x2a000     0x2000  r-xp   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7fef000     0x7ffff7ffa000     0xb000    0x2c000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffa000     0x7ffff7ffb000     0x1000 0x10663c000  rw-s   /dev/dri/renderD128
      0x7ffff7ffb000     0x7ffff7ffd000     0x2000    0x37000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffd000     0x7ffff7fff000     0x2000    0x39000  rw-p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffffffdd000     0x7ffffffff000    0x22000        0x0  rw-p   [stack]
  0xffffffffff600000 0xffffffffff601000     0x1000        0x0  --xp   [vsyscall]
        """

        self.mock(gdb, 'execute')
        self.expect(gdb.execute).args('info proc mappings', to_string=True).returns(proc_mappings)

        obj1 = self.mock()
        obj1.filename = '/usr/lib/x86_64-linux-gnu/libc.so.6'
        obj1.build_id = '161dd025ad435f0e873aafd55dc65d8a4cb1d93f'

        obj2 = self.mock()
        obj2.filename = '/usr/lib/x86_64-linux-gnu/libx265.so.199'
        obj2.build_id = '272ee025ad435f0e873aafd55dc65d8a4cb1d93f'

        self.mock(gdb, 'objfiles')
        self.expect(gdb.objfiles).returns([obj1, obj2])

        images = preamble.SentryImages()
        self.assert_equal([{'arch': 'x86_64',
            'code_file': '/usr/lib/x86_64-linux-gnu/libx265.so.199',
            'code_id': '272ee025ad435f0e873aafd55dc65d8a4cb1d93f',
            'debug_id': '25e02e27-43ad-0e5f-873a-afd55dc65d8a',
            'image_addr': '0x7fffc7000000',
            'image_size': 16076800,
            'type': 'elf'},
            {'arch': 'x86_64',
            'code_file': '/usr/lib/x86_64-linux-gnu/libc.so.6',
            'code_id': '161dd025ad435f0e873aafd55dc65d8a4cb1d93f',
            'debug_id': '25d01d16-43ad-0e5f-873a-afd55dc65d8a',
            'image_addr': '0x7ffff6200000',
            'image_size': 2207744,
            'type': 'elf'}]
            , images.to_list())

    def test_sentry_qml(self):
        thread = self.mock()
        self.expect(thread.is_valid).returns(True)
        self.expect(thread.switch)

        inferior = self.mock()
        self.expect(inferior.threads).returns([thread])

        symbol0 = self.mock()
        symbol0.is_variable = False
        symbol0.is_argument = False

        frame0 = self.mock(MagicMock)

        dereferenced_type = self.mock()
        dereferenced_type.name = 'QV4::ExecutionEngine'

        typeobj_target = self.mock()
        self.expect(typeobj_target.unqualified).returns(dereferenced_type)

        typeobj = self.mock()
        typeobj.code = 1
        self.expect(typeobj.target).returns(typeobj_target)

        value1 = MagicMock()
        value1.is_optimized_out = False
        value1.type = typeobj
        value1.__int__.return_value = 0x3

        symbol1 = self.mock()
        symbol1.is_variable = True
        symbol1.is_argument = False
        self.expect(symbol1.value).args(frame0).returns(value1)

        self.expect(frame0.__len__).returns(5)
        self.expect(frame0.block).returns([symbol0, symbol1])

        frame1 = self.mock(MagicMock)
        self.expect(frame1.__len__).returns(5)
        self.expect(frame1.older).returns(frame0)

        self.mock(gdb, 'selected_inferior')
        self.expect(gdb.selected_inferior).returns(inferior)
        self.mock(gdb, 'selected_thread')
        self.expect(gdb.selected_thread).returns(thread)
        self.mock(gdb, 'newest_frame')
        self.expect(gdb.newest_frame).returns(frame1)
        self.mock(gdb, 'parse_and_eval')
        qml_trace = '''
"stack=[frame={level=\\"0\\",func=\\"expression for onCompleted\\",file=\\"qrc:/ui/main.qml\\",fullname=\\"qrc:/ui/main.qml\\",line=\\"10\\",language=\\"js\\"}]"
        '''
        self.expect(gdb.parse_and_eval).args('qt_v4StackTraceForEngine((void*)0x3)').returns(qml_trace)
        gdb.TYPE_CODE_PTR = 1

        thread = preamble.SentryQMLThread()
        self.assert_equal({'crashed': True,
            'id': 'QML',
            'name': 'QML',
            'stacktrace': {'frames': [{'filename': 'qrc:/ui/main.qml',
            'function': 'expression for onCompleted',
            'in_app': True,
            'lineno': 10,
            'platform': 'other'}]}}, thread.to_dict())

    def test_sentry_image(self):
        objfile = self.mock()
        objfile.filename = '/usr/bin/true'
        objfile.build_id = '272ee025ad435f0e873aafd55dc65d8a4cb1d93f'

        self.mock(gdb, 'objfiles')
        self.expect(gdb.objfiles).returns([objfile])

        image = preamble.SentryImage('/usr/bin/true', 1000, 2000)
        self.assert_equal({'arch': 'x86_64',
            'code_file': '/usr/bin/true',
            'code_id': '272ee025ad435f0e873aafd55dc65d8a4cb1d93f',
            'debug_id': '25e02e27-43ad-0e5f-873a-afd55dc65d8a',
            'image_addr': '0x3e8',
            'image_size': 1000,
            'type': 'elf'}, image.to_dict())

    def test_sentry_image_mapping_fail(self):
        try:
            import sentry_sdk

            objfile = self.mock()
            objfile.filename = '/usr/bin/true'
            objfile.build_id = '272ee025ad435f0e873aafd55dc65d8a4cb1d93f'

            self.mock(gdb, 'objfiles')
            self.expect(gdb.objfiles).returns([objfile])

            self.mock(gdb, 'lookup_objfile')
            self.expect(gdb.lookup_objfile).returns(None)

            self.mock(sentry_sdk, 'capture_exception')
            self.expect(sentry_sdk.capture_exception)
            preamble.SentryImage('/usr/bin/true.so', 1000, 2000)
        except Exception as err:
            pass

    def test_print_qml_frames(self):
        # the payload is missing a line -> we expect no assertions of any kind!
        preamble.print_qml_frames('frame={level="0",func="saveConfig",file="/data/projects/kde/usr/share/lingmo/shells/org.kde.lingmo.desktop/contents/configuration/ConfigurationContainmentAppearance.qml",fullname="/data/project".')

if __name__ == '__main__':
    unittest.main()

