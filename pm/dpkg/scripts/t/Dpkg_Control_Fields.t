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
use Test::Dpkg qw(:paths);

BEGIN {
    plan tests => 2606;

    use_ok('Dpkg::Control::Types');
    use_ok('Dpkg::Control::FieldsCore');
    use_ok('Dpkg::Control');
}

#my $datadir = test_get_data_path();

my @src_dep_fields = qw(
    Build-Depends
    Build-Depends-Arch
    Build-Depends-Indep
    Build-Conflicts
    Build-Conflicts-Arch
    Build-Conflicts-Indep
);
my @bin_dep_normal_fields = qw(
    Pre-Depends
    Depends
    Recommends
    Suggests
    Enhances
);
my @bin_dep_union_fields = qw(
    Conflicts
    Breaks
    Replaces
    Provides
    Built-Using
    Static-Built-Using
);
my @bin_dep_fields = (
    @bin_dep_normal_fields,
    @bin_dep_union_fields,
);
my @src_checksums = qw(
    Checksums-Md5
    Checksums-Sha1
    Checksums-Sha256
);
my @bin_checksums = qw(
    MD5sum
    SHA1
    SHA256
);
my @src_files = (
    @src_checksums,
    qw(
        Files
    ),
);
my @bin_files = (
    qw(
        Filename
        Size
    ),
    @bin_checksums,
);
my @vcs_fields = qw(
    Vcs-Browser
    Vcs-Arch
    Vcs-Bzr
    Vcs-Cvs
    Vcs-Darcs
    Vcs-Git
    Vcs-Hg
    Vcs-Mtn
    Vcs-Svn
);
my @test_fields = qw(
    Testsuite
    Testsuite-Triggers
);

my %fields = (
    CTRL_TMPL_SRC() => {
        name => 'debian/control source stanza',
        fields => [
            qw(
                Source
                Section
                Priority
                Maintainer
                Uploaders
                Origin
                Bugs
            ),
            @vcs_fields,
            qw(
                Homepage
                Standards-Version
                Rules-Requires-Root
            ),
            @src_dep_fields,
            @test_fields,
            qw(
                Description
            ),
        ],
    },
    CTRL_TMPL_PKG() => {
        name => 'debian/control binary stanza',
        fields => [
            qw(
                Package
                Package-Type
                Section
                Priority
                Architecture
                Subarchitecture
                Multi-Arch
                Essential
                Protected
                Build-Essential
                Build-Profiles
                Built-For-Profiles
                Kernel-Version
            ),
            @bin_dep_fields,
            qw(
                Homepage
                Installer-Menu-Item
                Task
                Tag
                Description
            ),
        ],
    },
    CTRL_DSC() => {
        name => '.dsc',
        fields => [
            qw(
                Format
                Source
                Binary
                Architecture
                Version
                Origin
                Maintainer
                Uploaders
                Homepage
                Description
                Standards-Version
            ),
            @vcs_fields,
            @test_fields,
            @src_dep_fields,
            qw(
                Package-List
            ),
            @src_files,
        ],
    },
    CTRL_DEB() => {
        name => 'DEBIAN/control',
        fields => [
            qw(
                Package
                Package-Type
                Source
                Version
                Kernel-Version
                Built-For-Profiles
                Auto-Built-Package
                Architecture
                Subarchitecture
                Installer-Menu-Item
                Build-Essential
                Essential
                Protected
                Origin
                Bugs
                Maintainer
                Installed-Size
            ),
            @bin_dep_fields,
            qw(
                Section
                Priority
                Multi-Arch
                Homepage
                Description
                Tag
                Task
            ),
        ],
    },
    CTRL_REPO_SRC() => {
        name => 'Sources',
        fields => [
            qw(
                Format
                Package
                Binary
                Architecture
                Version
                Priority
                Section
                Origin
                Maintainer
                Uploaders
                Homepage
                Description
                Standards-Version
            ),
            @vcs_fields,
            @test_fields,
            @src_dep_fields,
            qw(
                Package-List
                Directory
            ),
            @src_files,
        ],
    },
    CTRL_REPO_PKG() => {
        name => 'Packages',
        fields => [
            qw(
                Package
                Package-Type
                Source
                Version
                Kernel-Version
                Built-For-Profiles
                Auto-Built-Package
                Architecture
                Subarchitecture
                Installer-Menu-Item
                Build-Essential
                Essential
                Protected
                Origin
                Bugs
                Maintainer
                Installed-Size
            ),
            @bin_dep_fields,
            @bin_files,
            qw(
                Section
                Priority
                Multi-Arch
                Homepage
                Description
                Tag
                Task
            ),
        ],
    },
    CTRL_REPO_RELEASE() => {
        name => 'Release',
        fields => [
            qw(
                Origin
                Label
                Suite
                Version
                Codename
                Changelogs
                Date
                Valid-Until
                NotAutomatic
                ButAutomaticUpgrades
                Acquire-By-Hash
                No-Support-for-Architecture-all
                Architectures
                Components
                Description
            ),
            @bin_checksums,
        ],
    },
    CTRL_CHANGELOG() => {
        name => 'debian/changelog',
        fields => [
            qw(
                Source
                Binary-Only
                Version
                Distribution
                Urgency
                Maintainer
                Timestamp
                Date
                Closes
                Changes
            ),
        ],
    },
    CTRL_COPYRIGHT_HEADER() => {
        name => 'debian/copyright Format stanza',
        fields => [
            qw(
                Format
                Upstream-Name
                Upstream-Contact
                Source
                Disclaimer
                Comment
                License
                Copyright
            ),
        ],
    },
    CTRL_COPYRIGHT_FILES() => {
        name => 'debian/copyright Files stanza',
        fields => [
            qw(
                Files
                Copyright
                License
                Comment
            ),
        ],
    },
    CTRL_COPYRIGHT_LICENSE() => {
        name => 'debian/copyright License stanza',
        fields => [
            qw(
                License
                Comment
            ),
        ],
    },
    CTRL_TESTS() => {
        name => 'debian/tests/control',
        fields => [
            qw(
                Test-Command
                Tests
                Tests-Directory
                Architecture
                Restrictions
                Features
                Classes
                Depends
            ),
        ],
    },
    CTRL_FILE_BUILDINFO() => {
        name => '.buildinfo',
        fields => [
            qw(
                Format
                Source
                Binary
                Architecture
                Version
                Binary-Only-Changes
            ),
            @src_checksums,
            qw(
                Build-Origin
                Build-Architecture
                Build-Kernel-Version
                Build-Date
                Build-Path
                Build-Tainted-By
                Installed-Build-Depends
                Environment
            ),
        ],
    },
    CTRL_FILE_CHANGES() => {
        name => '.changes',
        fields => [
            qw(
                Format
                Date
                Source
                Binary
                Binary-Only
                Built-For-Profiles
                Architecture
                Version
                Distribution
                Urgency
                Maintainer
                Changed-By
                Description
                Closes
                Changes
            ),
            @src_files,
        ],
    },
    CTRL_FILE_VENDOR() => {
        name => 'dpkg origin',
        fields => [
            qw(
                Vendor
                Vendor-Url
                Bugs
                Parent
            ),
        ],
    },
    CTRL_FILE_STATUS() => {
        name => 'dpkg status',
        fields => [
            qw(
                Package
                Essential
                Protected
                Status
                Priority
                Section
                Installed-Size
                Origin
                Maintainer
                Bugs
                Architecture
                Multi-Arch
                Source
                Version
                Config-Version
                Replaces
                Provides
                Depends
                Pre-Depends
                Recommends
                Suggests
                Breaks
                Conflicts
                Enhances
                Conffiles
                Description
                Triggers-Pending
                Triggers-Awaited
                Auto-Built-Package
                Build-Essential
                Built-For-Profiles
                Built-Using
                Static-Built-Using
                Homepage
                Installer-Menu-Item
                Kernel-Version
                Package-Type
                Subarchitecture
                Tag
                Task
            ),
        ],
    },
);

is_deeply([ field_list_src_dep() ],
          [ @src_dep_fields ],
          'List of build dependencies');
is_deeply([ field_list_pkg_dep() ],
          [ @bin_dep_fields ],
          'List of build dependencies');

is(field_get_default_value('Source'), undef,
    'no default value for Source field');
is(field_get_default_value('Section'), 'unknown',
    'default value for Section field');
is(field_get_default_value('Priority'), 'optional',
    'default value for Priority field');

is(field_capitalize('invented-field'), 'Invented-Field',
   'Field Invented-Field capitalization');
ok(!field_is_official('invented-field'),
   'Field Invented-Field is not official');

my %known_fields;
foreach my $type (sort keys %fields) {
    if (not $fields{$type}->{unordered}) {
        is_deeply([ field_ordered_list($type) ], $fields{$type}->{fields},
                  "List of $fields{$type}->{name} fields");
    }

    foreach my $field (@{$fields{$type}->{fields}}) {
        $known_fields{$field} = 1;
    }
}

foreach my $field (sort keys %known_fields) {
    is(field_capitalize($field), $field, "Field $field capitalization");
    is(field_capitalize(lc $field), $field, "Field lc($field) capitalization");
    is(field_capitalize(uc $field), $field, "Field uc($field) capitalization");

    ok(field_is_official($field), "Field $field is official");
    ok(field_is_official(lc $field), "Field lc($field) is official");
    ok(field_is_official(uc $field), "Field uc($field) is official");
}

foreach my $type (sort keys %fields) {
    my %allowed_fields = map { $_ => 1 } @{$fields{$type}->{fields}};

    foreach my $field (sort keys %known_fields) {
        if ($allowed_fields{$field}) {
            ok(field_is_allowed_in($field, $type),
               "Field $field allowed for type $fields{$type}->{name}");
        } else {
            ok(!field_is_allowed_in($field, $type),
               "Field $field not allowed for type $fields{$type}->{name}");
        }
    }
}

# Check deb822 field parsers

my $ctrl = Dpkg::Control->new(type => CTRL_DEB);

my ($source, $version);

$ctrl->{Package} = 'test-binary';
$ctrl->{Version} = '2.0-1';
$ctrl->{Source} = 'test-source (1.0)';
($source, $version) = field_parse_binary_source($ctrl);
is($source, 'test-source', 'Source package from binary w/ Source field');
is($version, '1.0', 'Source version from binary w/ Source field');

$ctrl->{Source} = 'test-source';
($source, $version) = field_parse_binary_source($ctrl);
is($source, 'test-source', 'Source package from binary w/ Source field w/o version');
is($version, '2.0-1', 'Source version from binary w/ Source field w/o version');

delete $ctrl->{Source};
($source, $version) = field_parse_binary_source($ctrl);
is($source, 'test-binary', 'Source package from binary w/o Source field');
is($version, '2.0-1', 'Source version from binary w/o Source field');
