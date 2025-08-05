#!/usr/bin/perl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

use strict;
use warnings;

use Test::More;
use Test::Dpkg qw(:needs);

test_needs_author();
test_needs_module('Test::Spelling');
test_needs_command('aspell');

if (qx(aspell dicts) !~ m/en_US/) {
    plan skip_all => 'aspell en_US dictionary required for spell checking POD';
}

test_needs_srcdir_switch();

my @files = Test::Dpkg::all_perl_files();

plan tests => scalar @files;

my @aspell_opts = qw(--encoding UTF-8 --lang en_US --personal /dev/null);
set_spell_cmd("aspell list @aspell_opts");
add_stopwords(<DATA>);

for my $file (@files) {
    pod_file_spelling_ok($file);
}

__DATA__
CVS
Devuan
DSC
Dpkg
IPC
ORed
OpenPGP
PureOS
RCS
XDG
ar
archqual
backport
build-indep
buildinfo
bzip2
bzr
canonicalized
changelogformat
checksum
checksums
cmdline
ctrl
debian
decompressor
dep
deps
dir
dpkg
dpkg-buildapi
dpkg-buildflags
dpkg-buildpackage
dpkg-checkbuilddeps
dpkg-dev
dpkg-genbuildinfo
dpkg-genchanges
dpkg-gencontrol
dpkg-parsechangelog
dpkg-mergechangelog
dsc
dselect
dup'ed
env
envvar
fieldnames
forceplugin
ge
getters
gettext
hurd
keyrings
le
libdir
lzma
modelines
multiarch
nocheck
objdump
qa
quiesced
reportfile
rfc822
sig
substvar
substvars
unparsed
update-buildflags
xz
