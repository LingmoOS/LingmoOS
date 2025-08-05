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

=encoding utf8

=head1 NAME

Dselect::Method - dselect method support

=head1 DESCRIPTION

This module provides support functions to implement methods.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dselect::Method 0.01;

use strict;
use warnings;

our @EXPORT = qw(
    %CONFIG
    yesno
    nb
    view_mirrors
    add_site
    edit_site
    edit_config
    read_config
    store_config
);

use Exporter qw(import);
use Carp;

eval q{
    use Data::Dumper;

    use Dpkg::File;
};
if ($@) {
    warn "Missing Dpkg modules required by the access method.\n\n";
    exit 1;
}

our %CONFIG;

sub yesno {
    my ($d, $msg) = @_;

    my ($res, $r);
    $r = -1;
    $r = 0 if $d eq 'n';
    $r = 1 if $d eq 'y';
    croak 'incorrect usage of yesno, stopped' if $r == -1;
    while (1) {
        print $msg, " [$d]: ";
        $res = <STDIN>;
        $res =~ /^[Yy]/ and return 1;
        $res =~ /^[Nn]/ and return 0;
        $res =~ /^[ \t]*$/ and return $r;
        print "Please enter one of the letters 'y' or 'n'\n";
    }
}

sub nb {
    my $nb = shift;

    if ($nb > 1024 ** 2) {
        return sprintf '%.2fM', $nb / 1024 ** 2;
    } elsif ($nb > 1024) {
        return sprintf '%.2fk', $nb / 1024;
    } else {
        return sprintf '%.2fb', $nb;
    }
}

sub read_config {
    my $vars = shift;
    my ($code, $conf);

    eval {
        $code = file_slurp($vars);
    };
    if ($@) {
        warn "$@\n";
        die "Try to relaunch the 'Access' step in dselect, thanks.\n";
    }

    my $VAR1; ## no critic (Variables::ProhibitUnusedVariables)
    $conf = eval $code;
    die "couldn't eval $vars content: $@\n" if $@;
    if (ref($conf) =~ /HASH/) {
        foreach (keys %{$conf}) {
            $CONFIG{$_} = $conf->{$_};
        }
    } else {
        print "Bad $vars file : removing it.\n";
        print "Please relaunch the 'Access' step in dselect. Thanks.\n";
        unlink $vars;
        exit 0;
    }
}

sub store_config {
    my $vars = shift;

    # Check that config is completed
    return if not $CONFIG{done};

    file_dump($vars, Dumper(\%CONFIG));
}

sub view_mirrors {
    print <<'MIRRORS';
Please see <https://www.debian.org/mirror/list> for a current
list of Debian mirror sites.
MIRRORS
}

sub edit_config {
    my ($method, $methdir) = @_;
    my $i;

    # Get a config for the sites
    while (1) {
        $i = 1;
        print "\n\nList of selected $method sites :\n";
        foreach (@{$CONFIG{site}}) {
            print "$i. $method://$_->[0]$_->[1] @{$_->[2]}\n";
            $i++;
        }
        print "\nEnter a command (a=add e=edit d=delete q=quit m=mirror list) \n";
        print 'eventually followed by a site number : ';
        chomp($_ = <STDIN>);
        /q/i && last;
        /a/i && add_site($method);
        /d\s*(\d+)/i && do {
            splice(@{$CONFIG{site}}, $1 - 1, 1) if $1 <= @{$CONFIG{site}};
            next;
        };
        /e\s*(\d+)/i && do {
            edit_site($method, $CONFIG{site}[$1 - 1]) if $1 <= @{$CONFIG{site}};
            next;
        };
        /m/i && view_mirrors();
    }

    print "\n";
    $CONFIG{use_auth_proxy} = yesno($CONFIG{use_auth_proxy} ? 'y' : 'n',
                                    'Go through an authenticated proxy');

    if ($CONFIG{use_auth_proxy}) {
        print "\nEnter proxy hostname [$CONFIG{proxyhost}] : ";
        chomp($_ = <STDIN>);
        $CONFIG{proxyhost} = $_ || $CONFIG{proxyhost};

        print "\nEnter proxy log name [$CONFIG{proxylogname}] : ";
        chomp($_ = <STDIN>);
        $CONFIG{proxylogname} = $_ || $CONFIG{proxylogname};

        print "\nEnter proxy password [$CONFIG{proxypassword}] : ";
        chomp($_ = <STDIN>);
        $CONFIG{proxypassword} = $_ || $CONFIG{proxypassword};
    }

    print "\nEnter directory to download binary package files to\n";
    print "(relative to $methdir)\n";
    while (1) {
        print "[$CONFIG{dldir}] : ";
        chomp($_ = <STDIN>);
        s{/$}{};
        $CONFIG{dldir} = $_ if $_;
        last if -d "$methdir/$CONFIG{dldir}";
        print "$methdir/$CONFIG{dldir} is not a directory !\n";
    }
}

sub add_site {
    my $method = shift;

    my $pas = 1;
    my $user = 'anonymous';
    my $email = qx(whoami);
    chomp $email;
    $email .= '@' . qx(cat /etc/mailname || dnsdomainname);
    chomp $email;
    my $dir = '/debian';

    push @{$CONFIG{site}}, [
        '',
        $dir,
        [
            'dists/stable/main',
            'dists/stable/contrib',
            'dists/stable/non-free-firmware',
            'dists/stable/non-free',
        ],
        $pas,
        $user,
        $email,
    ];
    edit_site($method, $CONFIG{site}[@{$CONFIG{site}} - 1]);
}

sub edit_site {
    my ($method, $site) = @_;

    local $_;

    print "\nEnter $method site [$site->[0]] : ";
    chomp($_ = <STDIN>);
    $site->[0] = $_ || $site->[0];

    print "\nUse passive mode [" . ($site->[3] ? 'y' : 'n') . '] : ';
    chomp($_ = <STDIN>);
    $site->[3] = (/y/i ? 1 : 0) if $_;

    print "\nEnter username [$site->[4]] : ";
    chomp($_ = <STDIN>);
    $site->[4] = $_ || $site->[4];

    print <<"EOF";

If you are using anonymous $method to retrieve files, enter your email
address for use as a password. Otherwise enter your password,
or "?" if you want the $method method to prompt you each time.

EOF

    print "Enter password [$site->[5]] : ";
    chomp($_ = <STDIN>);
    $site->[5] = $_ || $site->[5];

    print "\nEnter debian directory [$site->[1]] : ";
    chomp($_ = <STDIN>);
    $site->[1] = $_ || $site->[1];

    print "\nEnter space separated list of distributions to get\n";
    print "[@{$site->[2]}] : ";
    chomp($_ = <STDIN>);
    $site->[2] = [ split(/\s+/) ] if $_;
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;

__END__
