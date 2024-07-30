# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import tempfile
import subprocess
import unittest
import shutil
import tarfile
import sys

import Namcap.package

makepkg_conf = """
DLAGENTS=('ftp::/usr/bin/wget -c --passive-ftp -t 3 --waitretry=3 -O %%o %%u'
          'http::/usr/bin/wget -c -t 3 --waitretry=3 -O %%o %%u'
          'https::/usr/bin/wget -c -t 3 --waitretry=3 --no-check-certificate -O %%o %%u'
          'rsync::/usr/bin/rsync -z %%u %%o'
          'scp::/usr/bin/scp -C %%u %%o')
CARCH="%(arch)s"
CHOST="%(arch)s-pc-linux-gnu"
CFLAGS="-march=%(arch)s -mtune=generic -O2 -pipe"
CXXFLAGS="-march=%(arch)s -mtune=generic -O2 -pipe"
LDFLAGS="-Wl,--hash-style=gnu -Wl,--as-needed"
BUILDENV=(fakeroot !distcc color !ccache)
OPTIONS=(strip docs libtool emptydirs zipman purge)
INTEGRITY_CHECK=(md5)
STRIP_BINARIES="--strip-all"
STRIP_SHARED="--strip-unneeded"
STRIP_STATIC="--strip-debug"
MAN_DIRS=({usr{,/local}{,/share},opt/*}/{man,info})
DOC_DIRS=(usr/{,local/}{,share/}{doc,gtk-doc} opt/*/{doc,gtk-doc})
STRIP_DIRS=(bin lib sbin usr/{bin,lib,sbin,local/{bin,lib,sbin}} opt/*/{bin,lib,sbin})
PURGE_TARGETS=(usr/{,share}/info/dir .packlist *.pod)
PKGEXT='.pkg.tar.xz'
SRCEXT='.src.tar.gz'
"""


class MakepkgTest(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.mkdtemp()
        self.arch = os.uname()[-1]
        with open(os.path.join(self.tmpdir, "makepkg.conf"), "w") as f:
            f.write(makepkg_conf % {"arch": self.arch})

    def tearDown(self):
        shutil.rmtree(self.tmpdir)

    def run_makepkg(self):
        pwd = os.getcwd()
        os.chdir(self.tmpdir)
        environment = os.environ
        environment.update({"MAKEPKG_CONF": os.path.join(self.tmpdir, "makepkg.conf")})
        p = subprocess.Popen(["makepkg", "-f", "-d"], env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out1, out2 = p.communicate()
        if p.returncode != 0:
            sys.stderr.buffer.write(out1)
            sys.stderr.buffer.write(out2)
            raise RuntimeError("makepkg returned an error")
        os.chdir(pwd)

    def run_rule_on_tarball(self, filename, rule):
        # process PKGINFO
        pkg = Namcap.package.load_from_tarball(filename + ".xz")

        tar = tarfile.open(filename + ".xz")
        r = rule()
        r.analyze(pkg, tar)
        tar.close()
        return pkg, r
