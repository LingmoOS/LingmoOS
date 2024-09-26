#!/usr/bin/python3

import sys
from debian import deb822
import glob
import os
import os.path
import shutil
import subprocess
import time
import warnings

from debian_linux.debian import Changelog, VersionLinux


class Main(object):
    def __init__(self, input_repo, override_version):
        self.log = sys.stdout.write

        self.input_repo = input_repo

        changelog = Changelog(version=VersionLinux)[0]
        source = changelog.source
        version = changelog.version

        if override_version:
            version = VersionLinux('%s-0' % override_version)

        self.version_dfsg = version.linux_dfsg
        if self.version_dfsg is None:
            self.version_dfsg = '0'

        self.log('Using source name %s, version %s, dfsg %s\n' %
                 (source, version.upstream, self.version_dfsg))

        self.orig = '%s-%s' % (source, version.upstream)
        self.orig_tar = '%s_%s.orig.tar.xz' % (source, version.upstream)
        self.tag = 'v' + version.linux_upstream_full

    def __call__(self):
        import tempfile
        temp_dir = tempfile.mkdtemp(prefix='genorig', dir='debian')
        old_umask = os.umask(0o022)
        try:
            # When given a remote repo, we need a local copy.
            if not self.input_repo.startswith('/') and ':' in self.input_repo:
                temp_repo = os.path.join(temp_dir, 'git')
                subprocess.run(
                    ['git', 'clone', '--bare', '--depth=1', '-b', self.tag,
                     self.input_repo, temp_repo],
                    check=True)
                self.input_repo = temp_repo

            self.dir = os.path.join(temp_dir, 'export')
            os.mkdir(self.dir)
            self.upstream_export(self.input_repo)

            # exclude_files() will change dir mtimes.  Capture the
            # original release time so we can apply it to the final
            # tarball.
            orig_date = time.strftime(
                "%a, %d %b %Y %H:%M:%S +0000",
                time.gmtime(
                    os.stat(os.path.join(self.dir, self.orig, 'Makefile'))
                    .st_mtime))

            self.exclude_files()
            os.umask(old_umask)
            self.tar(orig_date)
        finally:
            os.umask(old_umask)
            shutil.rmtree(temp_dir)

    def upstream_export(self, input_repo):
        self.log("Exporting %s from %s\n" % (self.tag, input_repo))

        gpg_wrapper = os.path.join(os.getcwd(),
                                   "debian/bin/git-tag-gpg-wrapper")
        verify_proc = subprocess.Popen(['git',
                                        '-c', 'gpg.program=%s' % gpg_wrapper,
                                        'tag', '-v', self.tag],
                                       cwd=input_repo)
        if verify_proc.wait():
            raise RuntimeError("GPG tag verification failed")

        archive_proc = subprocess.Popen(['git', 'archive', '--format=tar',
                                         '--prefix=%s/' % self.orig, self.tag],
                                        cwd=input_repo,
                                        stdout=subprocess.PIPE)
        extract_proc = subprocess.Popen(['tar', '-xaf', '-'], cwd=self.dir,
                                        stdin=archive_proc.stdout)

        ret1 = archive_proc.wait()
        ret2 = extract_proc.wait()
        if ret1 or ret2:
            raise RuntimeError("Can't create archive")

    def exclude_files(self):
        self.log("Excluding file patterns specified in debian/copyright\n")
        with open("debian/copyright") as f:
            header = deb822.Deb822(f)
        patterns = header.get("Files-Excluded", '').strip().split()
        for pattern in patterns:
            matched = False
            for name in glob.glob(os.path.join(self.dir, self.orig, pattern)):
                try:
                    shutil.rmtree(name)
                except NotADirectoryError:
                    os.unlink(name)
                matched = True
            if not matched:
                warnings.warn("Exclusion pattern '%s' did not match anything"
                              % pattern,
                              RuntimeWarning)

    def tar(self, orig_date):
        out = os.path.join("../orig", self.orig_tar)
        try:
            os.mkdir("../orig")
        except OSError:
            pass
        try:
            os.stat(out)
            raise RuntimeError("Destination already exists")
        except OSError:
            pass
        self.log("Generate tarball %s\n" % out)

        env = os.environ.copy()
        env.update({
            'LC_ALL': 'C',
        })
        cmd = [
            'tar',
            '-C', self.dir,
            '--sort=name',
            '--mtime={}'.format(orig_date),
            '--owner=root',
            '--group=root',
            '--use-compress-program=xz -T0',
            '-cf',
            out, self.orig,
        ]

        try:
            subprocess.run(cmd, env=env, check=True)
            os.chmod(out, 0o644)
        except BaseException:
            try:
                os.unlink(out)
            except OSError:
                pass
            raise
        try:
            os.symlink(os.path.join('orig', self.orig_tar),
                       os.path.join('..', self.orig_tar))
        except OSError:
            pass


if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [OPTION]... REPO")
    parser.add_option("-V", "--override-version", dest="override_version",
                      help="Override version", metavar="VERSION")
    options, args = parser.parse_args()

    assert len(args) == 1
    Main(args[0], options.override_version)()
