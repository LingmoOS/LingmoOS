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
use Test::Dpkg qw(:needs :paths);

use Config;
use Cwd;
use IPC::Cmd qw(can_run);

if (defined $Config{bin_ELF} && $Config{bin_ELF} eq 'define') {
    plan tests => 150;
} else {
    plan skip_all => 'only ELF is currently supported';
}

$ENV{DEB_BUILD_ARCH} = 'amd64';
$ENV{DEB_HOST_ARCH} = 'amd64';

use_ok('Dpkg::Shlibs');

my $tmp;
my @tmp;
my %tmp;

my $datadir = test_get_data_path();

sub load_objdump_obj {
    my $name = shift;

    my $obj = Dpkg::Shlibs::Objdump::Object->new();
    open my $objdump, '<', "$datadir/objdump.$name"
        or die "$datadir/objdump.$name: $!";
    $obj->parse_objdump_output($objdump);
    close $objdump;
    return $obj;
}

my @librarypaths;

{
    # XXX: Keep as long as we support the deprecated LD_LIBRARY_PATH.
    local $ENV{LD_LIBRARY_PATH} = '/test-env';

    Dpkg::Shlibs::add_library_dir('/test-a');
    @librarypaths = Dpkg::Shlibs::get_library_paths();
    is($librarypaths[0], '/test-a', 'add_library_dir() does not get lost');

    Dpkg::Shlibs::add_library_dir('/test-b');
    @librarypaths = Dpkg::Shlibs::get_library_paths();
    is_deeply([ @librarypaths[0, 1] ] , [ '/test-a', '/test-b' ],
              'add_library_dir() prepends');
}

Dpkg::Shlibs::blank_library_paths();

# We want relative paths inside the ld.so.conf fragments to work, and $srcdir
# is usually a relative path, so let's temporarily switch directory.
# XXX: An alternative would be to make parse_ldso_conf relative path aware.
my $cwd = getcwd();
test_needs_srcdir_switch();
Dpkg::Shlibs::parse_ldso_conf('t/Dpkg_Shlibs/ld.so.conf');
chdir($cwd);

@librarypaths = Dpkg::Shlibs::get_library_paths();
is_deeply(\@librarypaths, [ qw(
        /nonexistent32
        /nonexistent/lib64
        /usr/local/lib
        /nonexistent/lib128
    ) ],
    'parsed library paths');

use_ok('Dpkg::Shlibs::Objdump');

my $obj;

$obj = load_objdump_obj('dbd-pg');
ok(!$obj->is_public_library(), 'Pg.so is not a public library');
ok(!$obj->is_executable(), 'Pg.so is not an executable');

$obj = load_objdump_obj('ls');
ok(!$obj->is_public_library(), 'ls is not a public library');
ok($obj->is_executable(), 'ls is an executable');

my $sym = $obj->get_symbol('optarg@GLIBC_2.0');
ok($sym, 'optarg@GLIBC_2.0 exists');
ok(!$sym->{defined}, 'R_*_COPY relocations are taken into account');

# Non-regression test for #506139
$obj = load_objdump_obj('space');
$sym = $obj->get_symbol('singlespace');
ok($sym, 'version less symbol separated by a single space are correctly parsed');

$obj = load_objdump_obj('libc6-2.6');
ok($obj->is_public_library(), 'libc6 is a public library');
ok($obj->is_executable(), 'libc6 is an executable');

is($obj->{SONAME}, 'libc.so.6', 'SONAME');
is($obj->{HASH}, '0x13d99c', 'HASH');
is($obj->{GNU_HASH}, '0x194', 'GNU_HASH');
is($obj->{format}, 'elf32-i386', 'format');
is_deeply($obj->{flags}, { DYNAMIC => 1, HAS_SYMS => 1, D_PAGED => 1 }, 'flags');
is_deeply($obj->{NEEDED}, [ 'ld-linux.so.2' ], 'NEEDED');
is_deeply([ $obj->get_needed_libraries ], [ 'ld-linux.so.2' ], 'NEEDED');

$sym = $obj->get_symbol('_sys_nerr@GLIBC_2.3');
is_deeply($sym, {
        name => '_sys_nerr',
        version => 'GLIBC_2.3',
        soname => 'libc.so.6',
        objid => 'libc.so.6',
        section => '.rodata',
        dynamic => 1,
        debug => '',
        type => 'O',
        weak => '',
        local => '',
        global => 1,
        visibility => '',
        hidden => 1,
        defined => 1,
    },
    'Symbol');
$sym = $obj->get_symbol('_IO_stdin_used');
is_deeply($sym, {
        name => '_IO_stdin_used',
        version => '',
        soname => 'libc.so.6',
        objid => 'libc.so.6',
        section => '*UND*',
        dynamic => 1,
        debug => '',
        type => ' ',
        weak => 1,
        local => '',
        global => '',
        visibility => '',
        hidden => '',
        defined => '',
    },
    'Symbol 2');

my @syms = $obj->get_exported_dynamic_symbols;
is(scalar @syms, 2231, 'defined && dynamic');
@syms = $obj->get_undefined_dynamic_symbols;
is(scalar @syms, 9, 'undefined && dynamic');


use_ok('Dpkg::Shlibs::SymbolFile');
use_ok('Dpkg::Shlibs::Symbol');

my $sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbol_file.tmp");
my $sym_file_dup = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbol_file.tmp");
my $sym_file_old = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbol_file.tmp");

my $obj_old = load_objdump_obj('libc6-2.3');

$sym_file->merge_symbols($obj_old, '2.3.6.ds1-13');
$sym_file_old->merge_symbols($obj_old, '2.3.6.ds1-13');

ok($sym_file->has_object('libc.so.6'), 'SONAME in sym file');

$sym_file->merge_symbols($obj, '2.6-1');

ok($sym_file->get_new_symbols($sym_file_old), 'has new symbols');
ok($sym_file_old->get_lost_symbols($sym_file), 'has lost symbols');

is($sym_file_old->lookup_symbol('__bss_start@Base', ['libc.so.6']),
    undef, 'internal symbols are ignored');

%tmp = $sym_file->lookup_symbol('_errno@GLIBC_2.0', ['libc.so.6'], 1);
isa_ok($tmp{symbol}, 'Dpkg::Shlibs::Symbol');
is_deeply(\%tmp, {
        symbol => Dpkg::Shlibs::Symbol->new(
            symbol => '_errno@GLIBC_2.0',
            minver => '2.3.6.ds1-13',
            dep_id => 0,
            deprecated => '2.6-1',
        ),
        soname => 'libc.so.6'
    },
    'deprecated symbol');

# Wildcard test
my $pat = $sym_file_old->create_symbol('*@GLIBC_PRIVATE 2.3.6.wildcard');
$sym_file_old->add_symbol($pat, 'libc.so.6');
$sym_file_old->merge_symbols($obj, '2.6-1');
$sym = $sym_file_old->lookup_symbol('__nss_services_lookup@GLIBC_PRIVATE', 'libc.so.6');
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => '__nss_services_lookup@GLIBC_PRIVATE',
        minver => '2.3.6.wildcard',
        dep_id => 0,
        deprecated => 0,
        tags => {
            symver => undef,
            optional => undef,
        },
        tagorder => [ 'symver', 'optional' ],
        matching_pattern => $pat,
    ),
    'wildcarded symbol');

# Save -> Load test
use File::Temp;
use File::Basename qw(basename);

sub save_load_test {
    my ($symfile, $comment, @opts) = @_;

    my $save_file = File::Temp->new();
    $symfile->save($save_file->filename, @opts);
    my $dup = Dpkg::Shlibs::SymbolFile->new(file => $save_file->filename);
    # Force sync of non-stored attributes
    $dup->{file} = $symfile->{file};
    $dup->{arch} = $symfile->{arch};

    is_deeply($dup, $symfile, $comment);
    if (-f $symfile->{file}) {
	is(system('diff', '-u', $symfile->{file}, $save_file->filename), 0,
	    basename($symfile->{file}) . ' dumped identical');
    }
}

save_load_test($sym_file, 'save -> load');


# Test ignoring internal symbols
$obj = load_objdump_obj('internal');

# Do not ignore any internal symbols
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbols.internal-filter");
$sym_file->merge_symbols($obj, '100.MISSING');

$sym = $sym_file->lookup_symbol('symbol@Base', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol@Base',
        minver => '1.0',
        dep_id => 0,
        deprecated => 0,
    ),
    'symbol unaffected w/o including internal symbols');

$sym = $sym_file->lookup_symbol('.gomp_critical_user_foo@Base', ['libinternal.so.0']);
is($sym, undef, 'gomp symbol omitted while filtering internal symbols');

$sym = $sym_file->lookup_symbol('__aeabi_lcmp@GCC_3.0', ['libinternal.so.0']);
is($sym, undef, 'known aeabi symbol omitted while filtering internal symbols');

$sym = $sym_file->lookup_symbol('__aeabi_unknown@GCC_4.0', ['libinternal.so.0']);
is($sym, undef, 'unknown aeabi symbol omitted while filtering internal symbols');

# Include internal symbols using the allow-internal tag.
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbols.internal-allow");
$sym_file->merge_symbols($obj, '100.MISSING');

$sym = $sym_file->lookup_symbol('symbol@Base', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol@Base',
        minver => '1.0',
        dep_id => 0,
        deprecated => 0,
    ),
    'symbol unaffected while including internal symbols via symbol tag');

$sym = $sym_file->lookup_symbol('.gomp_critical_user_foo@Base', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => '.gomp_critical_user_foo@Base',
        minver => '2.0',
        dep_id => 0,
        deprecated => 0,
        tags => {
            'allow-internal' => undef,
        },
        tagorder => [ 'allow-internal' ],
    ),
    'internal gomp symbol included via symbol tag');

$sym = $sym_file->lookup_symbol('__aeabi_lcmp@GCC_3.0', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => '__aeabi_lcmp@GCC_3.0',
        minver => '3.0',
        dep_id => 0,
        deprecated => 0,
        tags => {
            'allow-internal' => undef,
        },
        tagorder => [ 'allow-internal' ],
    ),
    'internal known aeabi symbol included via symbol tag');

$sym = $sym_file->lookup_symbol('__aeabi_unknown@GCC_4.0', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => '__aeabi_unknown@GCC_4.0',
        minver => '4.0',
        dep_id => 0,
        deprecated => 0,
        tags => {
            'allow-internal' => undef,
        },
        tagorder => [ 'allow-internal' ],
    ),
    'internal unknown aeabi symbol omitted via symbol tag');

# Include internal symbols using the Allow-Internal-Symbol-Groups field
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbols.internal-allow-groups");
$sym_file->merge_symbols($obj, '100.MISSING');

$sym = $sym_file->lookup_symbol('symbol@Base', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol@Base',
        minver => '1.0',
        dep_id => 0,
        deprecated => 0,
    ),
    'symbol unaffected w/o including internal symbols');

$sym = $sym_file->lookup_symbol('.gomp_critical_user_foo@Base', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => '.gomp_critical_user_foo@Base',
        minver => '2.0',
        dep_id => 0,
        deprecated => 0,
    ),
    'internal gomp symbol included via library field');

$sym = $sym_file->lookup_symbol('__aeabi_lcmp@GCC_3.0', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => '__aeabi_lcmp@GCC_3.0',
        minver => '3.0',
        dep_id => 0,
        deprecated => 0,
    ),
    'internal known aeabi symbol included via library field');

$sym = $sym_file->lookup_symbol('__aeabi_unknown@GCC_4.0', ['libinternal.so.0']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => '__aeabi_unknown@GCC_4.0',
        minver => '4.0',
        dep_id => 0,
        deprecated => 0,
    ),
    'internal unknown aeabi symbol included via library field');


# Test include mechanism of SymbolFile
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbols.include-1");

$sym = $sym_file->lookup_symbol('symbol_before@Base', ['libfake.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol_before@Base',
        minver => '0.9',
        dep_id => 0,
        deprecated => 0,
    ),
    'symbol before include not lost');

$sym = $sym_file->lookup_symbol('symbol_after@Base', ['libfake.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol_after@Base',
        minver => '1.1',
        dep_id => 0,
        deprecated => 0,
    ),
    'symbol after include not lost');

$sym = $sym_file->lookup_symbol('symbol1_fake1@Base', ['libfake.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol1_fake1@Base',
        minver => '1.0',
        dep_id => 0,
        deprecated => 0,
    ),
    'overrides order with #include');

$sym = $sym_file->lookup_symbol('symbol3_fake1@Base', ['libfake.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol3_fake1@Base',
        minver => '0',
        dep_id => 0,
        deprecated => 0,
    ),
    'overrides order with #include');

is($sym_file->get_smallest_version('libfake.so.1'), '0',
    'get_smallest_version with null version');

$sym = $sym_file->lookup_symbol('symbol_in_libdivert@Base', ['libdivert.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol_in_libdivert@Base',
        minver => '1.0~beta1',
        dep_id => 0,
        deprecated => 0,
    ),
    '#include can change current object');

$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbols.include-2");

$sym = $sym_file->lookup_symbol('symbol1_fake2@Base', ['libfake.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol1_fake2@Base',
        minver => '1.0',
        dep_id => 1,
        deprecated => 0,
    ),
    'overrides order with circular #include');

is($sym_file->get_smallest_version('libfake.so.1'), '1.0',
    'get_smallest_version');

my $io_data;
my $io;

# Check dump output
open $io, '>', \$io_data or die "cannot open io string\n";
$sym_file->output($io, package => 'libfake1');
is($io_data,
'libfake.so.1 libfake1 #MINVER#
| libvirtualfake
* Build-Depends-Package: libfake-dev
 symbol1_fake2@Base 1.0 1
 symbol2_fake2@Base 1.0
 symbol3_fake2@Base 1.1
', "Dump of $datadir/symbols.include-2");


# Check parsing of objdump output on ia64 (local symbols
# without versions and with visibility attribute)
$obj = load_objdump_obj('glib-ia64');
ok($obj->is_public_library(), 'glib-ia64 is a public library');
ok(!$obj->is_executable(), 'glib-ia64 is not an executable');

$sym = $obj->get_symbol('IA__g_free');
is_deeply($sym, {
        name => 'IA__g_free', version => '',
        soname => 'libglib-2.0.so.0',
        objid => 'libglib-2.0.so.0',
        section => '.text',
        dynamic => 1,
        debug => '',
        type => 'F',
        weak => '',
        local => 1,
        global => '',
        visibility => 'hidden',
        hidden => '',
        defined => 1,
    },
    'symbol with visibility without version');

# Check parsing of objdump output when symbol names contain spaces
$obj = load_objdump_obj('spacesyms');

sub check_spacesym {
    my ($name, $version, $visibility) = @_;

    $visibility //= '';
    $sym = $obj->get_symbol($name . "@" . $version);
    is_deeply($sym, {
            name => $name,
            version => $version,
            soname => 'libspacesyms.so.1',
            objid => 'libspacesyms.so.1',
            section => '.text',
            dynamic => 1,
            debug => '',
            type => 'F',
            weak => '',
            local => '',
            global => 1,
            visibility => $visibility,
            hidden => '',
            defined => 1,
        },
        $name);
    ok(defined $obj->{dynrelocs}{$name . "@@" . $version},
        "dynreloc found for $name");
}

check_spacesym('symdefaultvernospacedefault', 'Base');
check_spacesym('symdefaultvernospaceprotected', 'Base', 'protected');
check_spacesym('symlongvernospacedefault', 'VERY_LONG_VERSION_1');
check_spacesym('symlongvernospaceprotected', 'VERY_LONG_VERSION_1', 'protected');
check_spacesym('symshortvernospacedefault', 'V1');
check_spacesym('symshortvernospaceprotected', 'V1', 'protected');
check_spacesym('symdefaultverSPA CEdefault', 'Base');
check_spacesym('symdefaultverSPA CEprotected', 'Base', 'protected');
check_spacesym('symlongverSPA CEdefault', 'VERY_LONG_VERSION_1');
check_spacesym('symlongverSPA CEprotected', 'VERY_LONG_VERSION_1', 'protected');
check_spacesym('symshortverSPA CEdefault', 'V1');
check_spacesym('symshortverSPA CEprotected', 'V1', 'protected');

####### Test symbol tagging support  ######

# Parsing/dumping
# Template mode
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/basictags.symbols", arch => 'amd64');
save_load_test($sym_file, 'template save -> load', template_mode => 1);

# Dumping in non-template mode (amd64) (test for arch tags)
open $io, '>', \$io_data or die "cannot open io string\n";
$sym_file->output($io);
is($io_data,
'libbasictags.so.1 libbasictags1 #MINVER#
| libbasictags1 (>= 1.1)
 symbol11_optional@Base 1.1 1
 symbol21_amd64@Base 2.1
 symbol25_64@Base 2.5
 symbol26_little@Base 2.6
 symbol31_randomtag@Base 3.1
 symbol51_untagged@Base 5.1
', 'template vs. non-template on amd64');

# Dumping in non-template mode (mips) (test for arch tags)
open $io, '>', \$io_data or die "cannot open io string\n";
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/basictags.symbols", arch => 'mips');
$sym_file->output($io);
is($io_data,
'libbasictags.so.1 libbasictags1 #MINVER#
| libbasictags1 (>= 1.1)
 symbol11_optional@Base 1.1 1
 symbol23_mips@Base 2.3
 symbol24_32@Base 2.4
 symbol27_big@Base 2.7
 symbol31_randomtag@Base 3.1
 symbol42_mips_and_optional@Base 4.2
 symbol51_untagged@Base 5.1
', 'template vs. non-template on mips');

# Dumping in non-template mode (i386) (test for arch tags)
open $io, '>', \$io_data or die "cannot open io string\n";
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/basictags.symbols", arch => 'i386');
$sym_file_dup = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/basictags.symbols", arch => 'i386');
$sym_file->output($io);
is($io_data,
'libbasictags.so.1 libbasictags1 #MINVER#
| libbasictags1 (>= 1.1)
 symbol11_optional@Base 1.1 1
 symbol22_i386@Base 2.2
 symbol24_32@Base 2.4
 symbol26_little@Base 2.6
 symbol28_little_32@Base 2.8
 symbol31_randomtag@Base 3.1
 symbol41_i386_and_optional@Base 4.1
 symbol51_untagged@Base 5.1
', 'template vs. non-template on i386');

ok(defined $sym_file->{objects}{'libbasictags.so.1'}{syms}{'symbol21_amd64@Base'},
    'syms keys are symbol names without quotes');

# Preload objdumps
my $tags_obj_i386 = load_objdump_obj('basictags-i386');
$sym_file->merge_symbols($tags_obj_i386, '100.MISSING');
is_deeply($sym_file, $sym_file_dup, 'is objdump.basictags-i386 and basictags.symbols in sync');

my $tags_obj_amd64 = load_objdump_obj('basictags-amd64');

# Merge/get_{new,lost} tests for optional tag:
#  - disappeared
my $symbol11 = $tags_obj_i386->get_symbol('symbol11_optional@Base');
delete $tags_obj_i386->{dynsyms}{'symbol11_optional@Base'};
$sym_file->merge_symbols($tags_obj_i386, '100.MISSING');

$sym = $sym_file->lookup_symbol('symbol11_optional@Base', ['libbasictags.so.1'], 1);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol11_optional@Base',
        symbol_templ => 'symbol11_optional@Base',
        minver => '1.1',
        dep_id => 1,
        deprecated => '100.MISSING',
        tags => {
            optional => undef,
        },
        tagorder => [ 'optional' ],
    ),
    'disappeared optional symbol gets deprecated');

$sym_file->merge_symbols($tags_obj_i386, '101.MISSING');
$sym = $sym_file->lookup_symbol('symbol11_optional@Base', ['libbasictags.so.1'], 1);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol11_optional@Base',
        symbol_templ => 'symbol11_optional@Base',
        minver => '1.1',
        dep_id => 1,
        deprecated => '101.MISSING',
        tags => {
            optional => undef,
        },
        tagorder => [ 'optional' ],
    ),
    'deprecated text of MISSING optional symbol gets rebumped each merge');

is(scalar($sym_file->get_lost_symbols($sym_file_dup)), 0,
    'missing optional symbol is not LOST');

# - reappeared (undeprecate, minver should be 1.1, not 100.MISSED)
$tags_obj_i386->add_dynamic_symbol($symbol11);
$sym_file->merge_symbols($tags_obj_i386, '100.MISSING');
$sym = $sym_file->lookup_symbol('symbol11_optional@Base', ['libbasictags.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol11_optional@Base',
        symbol_templ => 'symbol11_optional@Base',
        minver => '1.1',
        dep_id => 1,
        deprecated => 0,
        tags => {
            optional => undef,
        },
        tagorder => [ 'optional' ],
    ),
    'reappeared optional symbol gets undeprecated + minver');
is(scalar($sym_file->get_lost_symbols($sym_file_dup) +
          $sym_file->get_new_symbols($sym_file_dup)), 0,
    'reappeared optional symbol: neither NEW nor LOST');

# Merge/get_{new,lost} tests for arch tag:
# - arch specific appears on wrong arch: 'arch' tag should be removed
my $symbol21 = $tags_obj_amd64->get_symbol('symbol21_amd64@Base');
$tags_obj_i386->add_dynamic_symbol($symbol21);
$sym_file->merge_symbols($tags_obj_i386, '100.MISSING');
$sym = $sym_file->lookup_symbol('symbol21_amd64@Base', ['libbasictags.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol21_amd64@Base',
        symbol_templ => 'symbol21_amd64@Base',
        symbol_quoted => "'",
        minver => '2.1',
        dep_id => 0,
        deprecated => 0,
    ),
    'symbol appears on foreign arch, arch tag should be removed');
@tmp = map { $_->{symbol}->get_symbolname() } $sym_file->get_new_symbols($sym_file_dup);
is_deeply(\@tmp, [ 'symbol21_amd64@Base' ], 'symbol from foreign arch is NEW');
is($sym->get_symbolspec(1), ' symbol21_amd64@Base 2.1',
    'no tags => no quotes in the symbol name');

# - arch specific symbol disappears
delete $tags_obj_i386->{dynsyms}{'symbol22_i386@Base'};
delete $tags_obj_i386->{dynsyms}{'symbol24_32@Base'};
delete $tags_obj_i386->{dynsyms}{'symbol26_little@Base'};
delete $tags_obj_i386->{dynsyms}{'symbol28_little_32@Base'};
delete $tags_obj_i386->{dynsyms}{'symbol41_i386_and_optional@Base'};
$sym_file->merge_symbols($tags_obj_i386, '100.MISSING');

$sym = $sym_file->lookup_symbol('symbol22_i386@Base', ['libbasictags.so.1'], 1);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol22_i386@Base',
        symbol_templ => 'symbol22_i386@Base',
        minver => '2.2',
        dep_id => 0,
        deprecated => '100.MISSING',
        tags => {
            arch => '!amd64 !ia64 !mips',
        },
        tagorder => [ 'arch' ],
    ),
    'disappeared arch specific symbol gets deprecated');
$sym = $sym_file->lookup_symbol('symbol24_32@Base', ['libbasictags.so.1'], 1);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol24_32@Base',
        symbol_templ => 'symbol24_32@Base',
        minver => '2.4',
        dep_id => 0,
        deprecated => '100.MISSING',
        tags => {
            'arch-bits' => '32',
        },
        tagorder => [ 'arch-bits' ],
    ),
    'disappeared arch bits specific symbol gets deprecated');
$sym = $sym_file->lookup_symbol('symbol26_little@Base', ['libbasictags.so.1'], 1);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol26_little@Base',
        symbol_templ => 'symbol26_little@Base',
        minver => '2.6',
        dep_id => 0,
        deprecated => '100.MISSING',
        tags => {
            'arch-endian' => 'little',
        },
        tagorder => [ 'arch-endian' ],
    ),
    'disappeared arch endian specific symbol gets deprecated');
$sym = $sym_file->lookup_symbol('symbol28_little_32@Base', ['libbasictags.so.1'], 1);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol28_little_32@Base',
        symbol_templ => 'symbol28_little_32@Base',
        minver => '2.8',
        dep_id => 0,
        deprecated => '100.MISSING',
        tags => {
            'arch-bits' => '32',
            'arch-endian' => 'little',
        },
        tagorder => [ 'arch-bits', 'arch-endian' ],
    ),
    'disappeared arch bits and endian specific symbol gets deprecated');
$sym = $sym_file->lookup_symbol('symbol41_i386_and_optional@Base', ['libbasictags.so.1'], 1);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol41_i386_and_optional@Base',
        symbol_templ => 'symbol41_i386_and_optional@Base',
        symbol_quoted => '"',
        minver => '4.1',
        dep_id => 0,
        deprecated => '100.MISSING',
        tags => {
            arch => 'i386',
            optional => 'reason',
        },
        tagorder => [ 'arch', 'optional' ],
    ),
    'disappeared optional arch specific symbol gets deprecated');
@tmp = sort map { $_->{symbol}->get_symbolname() } $sym_file->get_lost_symbols($sym_file_dup);
is_deeply(\@tmp, [
        'symbol22_i386@Base',
        'symbol24_32@Base',
        'symbol26_little@Base',
        'symbol28_little_32@Base',
    ],
    "missing arch specific is LOST, but optional arch specific isn't");

# Tests for tagged #includes
$sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/symbols.include-3", arch => 'i386');
$sym = $sym_file->lookup_symbol('symbol2_fake1@Base', ['libbasictags.so.2']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol2_fake1@Base',
        minver => '1.0',
        tags => {
            optional => undef,
            'random tag' => 'random value',
        },
        tagorder => [ 'optional', 'random tag' ],
    ),
    'symbols from #included file inherits tags');
$sym = $sym_file->lookup_symbol('symbol41_i386_and_optional@Base', ['libbasictags.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol41_i386_and_optional@Base',
        symbol_templ => 'symbol41_i386_and_optional@Base',
        symbol_quoted => '"',
        minver => '4.1',
        tags => {
            optional => 'reason',
            t => 'v',
            arch => 'i386',
        },
        tagorder => [ 'optional', 't', 'arch' ],
    ),
    'symbols in #included file can override tag values');
$sym = $sym_file->lookup_symbol('symbol51_untagged@Base', ['libbasictags.so.1']);
is_deeply($sym, Dpkg::Shlibs::Symbol->new(
        symbol => 'symbol51_untagged@Base',
        minver => '5.1',
        tags => {
            optional => 'from parent',
            t => 'v',
        },
        tagorder => [ 'optional', 't' ],
    ),
    'symbols are properly cloned when #including');

# Test Symbol::clone()
$sym = Dpkg::Shlibs::Symbol->new(
    symbol => 'foobar',
    testfield => 1,
    teststruct => {
        foo => 1,
    },
);
$tmp = $sym->clone();
$tmp->{teststruct}{foo} = 3;
$tmp->{testfield} = 3;
is($sym->{teststruct}{foo}, 1, 'original field "foo" not changed');
is($sym->{testfield}, 1, 'original field "testfield" not changed');

############ Test symbol patterns ###########

SKIP: {

skip 'c++filt not available', 41 if not can_run('c++filt');

sub load_patterns_obj {
    $obj = load_objdump_obj('patterns');
    return $obj;
}

sub load_patterns_symbols {
    $sym_file = Dpkg::Shlibs::SymbolFile->new(file => "$datadir/patterns.symbols");
    return $sym_file;
}

load_patterns_obj();
$sym_file_dup = load_patterns_symbols();
load_patterns_symbols();

save_load_test($sym_file, 'save -> load test of patterns template', template_mode => 1);

isnt($sym_file->get_patterns('libpatterns.so.1'), 0,
    'patterns.symbols has patterns');

$sym_file->merge_symbols($obj, '100.MISSING');

@tmp = map { $_->get_symbolname() } $sym_file->get_lost_symbols($sym_file_dup);
is_deeply(\@tmp, [], 'no LOST symbols if all patterns matched.');
@tmp = map { $_->get_symbolname() } $sym_file->get_new_symbols($sym_file_dup);
is_deeply(\@tmp, [], 'no NEW symbols if all patterns matched.');

# Pattern resolution order: aliases (c++, symver), generic
$sym = $sym_file->lookup_symbol('SYMVER_1@SYMVER_1', 'libpatterns.so.1');
is($sym->{minver}, '1', 'specific SYMVER_1 symbol');

$sym = $sym_file->lookup_symbol('_ZN3NSB6Symver14symver_method1Ev@SYMVER_1', 'libpatterns.so.1');
is($sym->{minver}, '1.method1', 'specific symbol preferred over pattern');

$sym = $sym_file->lookup_symbol('_ZN3NSB6Symver14symver_method2Ev@SYMVER_1', 'libpatterns.so.1');
is($sym->{minver}, '1.method2', 'c++ alias pattern preferred over generic pattern');
is($sym->get_pattern()->get_symbolname(), 'NSB::Symver::symver_method2()@SYMVER_1',
    'c++ alias pattern preferred over generic pattern, on demangled name');

$sym = $sym_file->lookup_symbol('_ZN3NSB6SymverD1Ev@SYMVER_1', 'libpatterns.so.1');
is($sym->{minver}, '1.generic',
    'generic (c++ & symver) pattern covers the rest (destructor)');
ok($sym->get_pattern()->equals($sym_file->create_symbol('(c++|symver)SYMVER_1 1.generic')),
    'generic (c++ & symver) pattern covers the rest (destructor), compared');

# Test old style wildcard support
load_patterns_symbols();
$sym = $sym_file->create_symbol('*@SYMVEROPT_2 2');
ok($sym->is_optional(), 'Old style wildcard is optional');
is($sym->get_alias_type(), 'symver', 'old style wildcard is a symver pattern');
is($sym->get_symbolname(), 'SYMVEROPT_2', 'wildcard pattern got renamed');

$pat = $sym_file->lookup_pattern('(symver|optional)SYMVEROPT_2', 'libpatterns.so.1');
$sym->{symbol_templ} = $pat->{symbol_templ};
is_deeply($pat, $sym, 'old style wildcard is the same as (symver|optional)');

# Get rid of all SymverOptional symbols
foreach my $tmp (keys %{$obj->{dynsyms}}) {
    delete $obj->{dynsyms}{$tmp} if $tmp =~ /SymverOptional/;
}
$sym_file->merge_symbols($obj, '100.MISSING');
is_deeply([ map { $_->get_symbolname() } $pat->get_pattern_matches() ],
    [], 'old style wildcard matches nothing.');
is($pat->{deprecated}, '100.MISSING', 'old style wildcard gets deprecated.');
@tmp = map { $_->{symbol}->get_symbolname() } $sym_file->get_lost_symbols($sym_file_dup);
is_deeply(\@tmp, [], 'but old style wildcard is not LOST.');

# 'Internal' pattern covers all internal symbols
load_patterns_obj();
@tmp = grep { $_->get_symbolname() =~ /Internal/ } $sym_file->get_symbols('libpatterns.so.1');
$sym = $sym_file->create_symbol('(regex|c++)^_Z(T[ISV])?N3NSA6ClassA8Internal.*@Base$ 1.internal');
$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1');
is_deeply([ sort $pat->get_pattern_matches() ], [ sort @tmp ],
    'Pattern covers all internal symbols');
is($tmp[0]->{minver}, '1.internal', 'internal pattern covers first symbol');

# Lookup private pattern
my @private_symnames = sort qw(
    _ZN3NSA6ClassA7Private11privmethod1Ei@Base
    _ZN3NSA6ClassA7Private11privmethod2Ei@Base
    _ZN3NSA6ClassA7PrivateC1Ev@Base
    _ZN3NSA6ClassA7PrivateC2Ev@Base
    _ZN3NSA6ClassA7PrivateD0Ev@Base
    _ZN3NSA6ClassA7PrivateD1Ev@Base
    _ZN3NSA6ClassA7PrivateD2Ev@Base
    _ZTIN3NSA6ClassA7PrivateE@Base
    _ZTSN3NSA6ClassA7PrivateE@Base
    _ZTVN3NSA6ClassA7PrivateE@Base
);
$sym = $sym_file->create_symbol('(c++|regex|optional)NSA::ClassA::Private(::.*)?@Base 1');
$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1');
isnt($pat, undef, 'pattern for private class has been found');
is_deeply([ sort map { $_->get_symbolname() } $pat->get_pattern_matches() ],
    \@private_symnames, 'private pattern matched expected symbols');
ok(($pat->get_pattern_matches())[0]->is_optional(),
    'private symbol is optional like its pattern');
ok($sym_file->lookup_symbol(($pat->get_pattern_matches())[0], 'libpatterns.so.1'),
    'lookup_symbol() finds symbols matched by pattern (after merge)');

# Get rid of a private symbol, it should not be lost
delete $obj->{dynsyms}{$private_symnames[0]};
load_patterns_symbols();
$sym_file->merge_symbols($obj, '100.MISSING');

$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1');
@tmp = map { $_->{symbol}->get_symbolname() } $sym_file->get_lost_symbols($sym_file_dup);
is_deeply(\@tmp, [], 'no LOST symbols when got rid of patterned optional symbol.');
ok(!$pat->{deprecated}, 'there are still matches, pattern is not deprecated.');

# Get rid of all private symbols, the pattern should be deprecated.
foreach my $tmp (@private_symnames) {
    delete $obj->{dynsyms}{$tmp};
}
load_patterns_symbols();
$sym_file->merge_symbols($obj, '100.MISSING');

$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1', 1);
@tmp = $sym_file->get_lost_symbols($sym_file_dup);
is_deeply(\@tmp, [ ],
    'All private symbols gone, but pattern is not LOST because it is optional.');
is($pat->{deprecated}, '100.MISSING',
    'All private symbols gone - pattern deprecated.');

# Internal symbols. All covered by the pattern?
@tmp = grep { $_->get_symbolname() =~ /Internal/ } values %{$sym_file->{objects}{'libpatterns.so.1'}{syms}};
$sym = $sym_file->create_symbol('(regex|c++)^_Z(T[ISV])?N3NSA6ClassA8Internal.*@Base$ 1.internal');
$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1');
is_deeply([ sort $pat->get_pattern_matches() ], [ sort @tmp ],
    'Pattern covers all internal symbols');
is($tmp[0]->{minver}, '1.internal', 'internal pattern covers first symbol');

# Delete matches of the non-optional pattern
$sym = $sym_file->create_symbol('(c++)"non-virtual thunk to NSB::ClassD::generate_vt(char const*) const@Base" 1');
$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1');
isnt($pat, undef, 'lookup_pattern() finds alias-based pattern');

is(scalar($pat->get_pattern_matches()), 2, 'two matches for the generate_vt pattern');
foreach my $tmp ($pat->get_pattern_matches()) {
    delete $obj->{dynsyms}{$tmp->get_symbolname()};
}
load_patterns_symbols();
$sym_file->merge_symbols($obj, '100.MISSING');

$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1', 1);
@tmp = map { scalar $sym_file->lookup_pattern($_->{symbol}, 'libpatterns.so.1', 1) }
             $sym_file->get_lost_symbols($sym_file_dup);
is_deeply(\@tmp, [ $pat ], 'No matches - generate_vt() pattern is LOST.');
is($pat->{deprecated}, '100.MISSING',
    'No matches - generate_vt() pattern is deprecated.');

# Pattern undeprecation when matches are discovered
load_patterns_obj();
load_patterns_symbols();

$pat = $sym_file_dup->lookup_pattern($sym, 'libpatterns.so.1');
$pat->{deprecated} = '0.1-1';
$pat = $sym_file->lookup_pattern($sym, 'libpatterns.so.1');
$pat->{deprecated} = '0.1-1';

$sym_file->merge_symbols($obj, '100.FOUND');
ok(! $pat->{deprecated},
    'Previously deprecated pattern with matches got undeprecated');
is($pat->{minver}, '100.FOUND',
    'Previously deprecated pattern with matches got minver bumped');
@tmp = map { $_->{symbol}->get_symbolspec(1) } $sym_file->get_new_symbols($sym_file_dup);
is_deeply(\@tmp, [ $pat->get_symbolspec(1) ],
    'Previously deprecated pattern with matches is NEW. Matches themselves are not NEW.');
foreach my $sym ($pat->get_pattern_matches()) {
    ok(!$sym->{deprecated}, $sym->get_symbolname() . ': not deprecated');
    is($sym->{minver}, '100.FOUND', $sym->get_symbolname() . ': version bumped');
}

}
