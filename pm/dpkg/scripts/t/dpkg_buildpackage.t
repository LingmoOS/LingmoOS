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
use Test::Dpkg qw(:needs :paths test_neutralize_checksums);

use File::Spec::Functions qw(rel2abs);
use File::Compare;
use File::Path qw(make_path);
use File::Copy;

use Dpkg::File;
use Dpkg::IPC;
use Dpkg::BuildTypes;
use Dpkg::Substvars;

test_needs_command('fakeroot');

plan tests => 17;

my $srcdir = rel2abs($ENV{srcdir} || '.');
my $datadir = "$srcdir/t/dpkg_buildpackage";
my $tmpdir = test_get_temp_path();

$ENV{$_} = rel2abs($ENV{$_}) foreach qw(DPKG_DATADIR DPKG_ORIGINS_DIR);

# Any parallelization from the parent should be ignored, we are testing
# the makefiles serially anyway.
delete $ENV{MAKEFLAGS};

# Delete variables that can affect the tests.
delete $ENV{SOURCE_DATE_EPOCH};

# Delete other variables that can affect the tests.
delete $ENV{$_} foreach grep { m/^DEB_/ } keys %ENV;

# Set architecture variables to not require dpkg nor gcc.
$ENV{PATH} = "$srcdir/t/mock-bin:$ENV{PATH}";

chdir $tmpdir;

my $tmpl_format = <<'TMPL_FORMAT';
3.0 (native)
TMPL_FORMAT

my $tmpl_changelog = <<'TMPL_CHANGELOG';
${source-name} (${source-version}) ${suite}; urgency=${urgency}

  * Entry. Closes: #12345

 -- ${maintainer}  Thu, 30 Jun 2016 20:15:12 +0200
TMPL_CHANGELOG

my $tmpl_control = <<'TMPL_CONTROL';
Source: ${source-name}
Section: ${source-section}
Priority: ${source-priority}
Maintainer: ${maintainer}

Package: test-binary-all
Architecture: all
Description: architecture independent binary package

Package: test-binary-any
Architecture: any
Description: architecture dependent binary package
TMPL_CONTROL

my $tmpl_rules = <<'TMPL_RULES';
#!/usr/bin/make -f

DI := debian/${binary-name-all}
DA := debian/${binary-name-any}

# fakeroot confuses ASAN link order check.
export ASAN_OPTIONS = verify_asan_link_order=0
# Do not fail due to leaks, as the code is still using lots of
# static variables and error variables.
export LSAN_OPTIONS = exitcode=0

clean:
	rm -f debian/files
	rm -rf $(DI) $(DA)

build-indep:
build-arch:
build: build-indep build-arch

binary-indep: build-indep
	rm -rf $(DI)
	mkdir -p $(DI)/DEBIAN
	dpkg-gencontrol -P$(DI) -p${binary-name-all}
	dpkg-deb --build $(DI) ..

binary-arch: build-arch
	rm -rf $(DA)
	mkdir -p $(DA)/DEBIAN
	dpkg-gencontrol -P$(DA) -p${binary-name-any}
	dpkg-deb --build $(DA) ..

binary: binary-indep binary-arch

.PHONY: clean build-indep build-arch build binary-indexp binary-arch binary
TMPL_RULES

my %default_substvars = (
    'source-name' => 'test-source',
    'source-version' => 0,
    'source-section' => 'test',
    'source-priority' => 'optional',
    'binary-name-all' => 'test-binary-all',
    'binary-name-any' => 'test-binary-any',
    'suite' => 'unstable',
    'urgency' => 'low',
    'maintainer' => 'Dpkg Developers <debian-dpkg@lists.debian.org>',
);

sub gen_from_tmpl
{
    my ($pathname, $tmpl, $substvars) = @_;

    file_dump($pathname, $substvars->substvars($tmpl));
}

sub gen_source
{
    my (%opts) = @_;

    my $substvars = Dpkg::Substvars->new();
    foreach my $var (%default_substvars) {
        my $value = $opts{$var} // $default_substvars{$var};

        $substvars->set_as_auto($var, $value);
    }

    my $source = $substvars->get('source-name');
    my $version = $substvars->get('source-version');
    my $basename = "$source\_$version";
    my $dirname = $basename =~ tr/_/-/r;

    make_path("$dirname/debian/source");

    gen_from_tmpl("$dirname/debian/source/format", $tmpl_format, $substvars);
    gen_from_tmpl("$dirname/debian/changelog", $tmpl_changelog, $substvars);
    gen_from_tmpl("$dirname/debian/control", $tmpl_control, $substvars);
    gen_from_tmpl("$dirname/debian/rules", $tmpl_rules, $substvars);

    return $basename;
}

sub test_diff
{
    my $filename = shift;

    my $expected_file = "$datadir/$filename";
    my $generated_file =  $filename;

    test_neutralize_checksums($generated_file);

    my $res = compare($expected_file, $generated_file);
    if ($res) {
        system "diff -u $expected_file $generated_file >&2";
    }
    ok($res == 0, "generated file matches expected one ($expected_file)");
}

sub test_build
{
    my ($basename, $type) = @_;
    my $dirname = $basename =~ tr/_/-/r;

    set_build_type($type, 'buildtype', nocheck => 1);
    my $typename = get_build_options_from_type();

    my $stderr;

    my @hook_names = qw(
        preinit
        init
        preclean
        source
        build
        binary
        buildinfo
        changes
        postclean
        check
        sign
        done
    );
    my @hook_opts = map {
        "--hook-$_=$datadir/hook a=%a p=%p v=%v s=%s u=%u >>../$basename\_$typename.hook"
    } @hook_names;

    chdir $dirname;
    spawn(exec => [ $ENV{PERL}, "$srcdir/dpkg-buildpackage.pl",
                    "--admindir=$datadir/dpkgdb",
                    '--host-arch=amd64',
                    '--ignore-builtin-builddeps',
                    '--no-sign',
                    "--build=$typename",
                    '--check-command=',
                    @hook_opts,
                  ],
          error_to_string => \$stderr,
          wait_child => 1, nocheck => 1);
    chdir '..';

    ok($? == 0, "dpkg-buildpackage --build=$typename succeeded");
    diag($stderr) unless $? == 0;

    if (build_has_all(BUILD_ARCH_DEP)) {
        # Rename the file to preserve on consecutive invocations.
        move("$basename\_amd64.changes", "$basename\_$typename.changes");
    }

    test_diff("$basename\_$typename.hook");

    if (build_has_all(BUILD_SOURCE)) {
        test_diff("$basename.dsc");
    }

    test_diff("$basename\_$typename.changes");
}

my $basename = gen_source();

test_build($basename, BUILD_SOURCE);
test_build($basename, BUILD_ARCH_INDEP);
test_build($basename, BUILD_ARCH_DEP);
test_build($basename, BUILD_BINARY);
test_build($basename, BUILD_FULL);
