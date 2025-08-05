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

use Test::More tests => 17;
use Test::Dpkg qw(:paths);

$ENV{DEB_HOST_ARCH} = 'amd64';

use Dpkg::Control;
use Dpkg::Control::Info;

BEGIN {
    use_ok('Dpkg::BuildAPI', qw(get_build_api reset_build_api));
}

my $datadir = test_get_data_path();

sub test_load_ctrl {
    my $file = shift;

    my $ctrl = Dpkg::Control::Info->new(type => CTRL_TMPL_SRC);
    $ctrl->load("$datadir/$file");

    return $ctrl;
}

sub test_parse_env_invalid {
    my ($value, $desc) = @_;

    my $api;

    eval {
        local $ENV{DPKG_BUILD_API} = $value;
        reset_build_api();
        $api = get_build_api();
    };

    ok($@, "failed to parse build API $desc: $@");
    is($api, undef, "parsing invalid build API $desc returns undef");
}

sub test_parse_ctrl_invalid {
    my ($file, $desc) = @_;

    my $ctrl = test_load_ctrl($file);
    my $api;

    eval {
        reset_build_api();
        $api = get_build_api($ctrl);
    };
    ok($@, "failed to parse build API $desc from $file: $@");
    is($api, undef, "parsing invalid build API $desc from $file returns undef");
}

sub test_parse_ctrl {
    my ($file, $exp_api, $desc) = @_;

    my $ctrl = test_load_ctrl($file);

    reset_build_api();
    my $api = get_build_api($ctrl);
    is($api, $exp_api, "build API $desc matches");
}

test_parse_env_invalid('aa', 'text level');
test_parse_env_invalid('999999999', 'out of range');

test_parse_ctrl_invalid('ctrl-api-desync', 'multiple build API levels out of sync');
test_parse_ctrl_invalid('ctrl-api-no-ver', 'build API with no level');
test_parse_ctrl_invalid('ctrl-api-rel-noeq', 'API level with non-eq operator');
test_parse_ctrl_invalid('ctrl-api-gt-max', 'API level > max');
test_parse_ctrl_invalid('ctrl-api-no-int', 'API level is not an integer');

test_parse_ctrl('ctrl-api-default', 0, 'default API level');
test_parse_ctrl('ctrl-api-explicit', 1, 'explicit API level');

# TODO: Add more test cases.
