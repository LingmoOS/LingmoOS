#!/usr/bin/perl
#
# Copyright © 1996 Andy Guy <andy@cyteen.org>
# Copyright © 1998 Martin Schulze <joey@infodrom.north.de>
# Copyright © 1999, 2009 Raphaël Hertzog <hertzog@debian.org>
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

eval q{
    use Dpkg; # Dummy import to require the presence of Dpkg::*.
};
if ($@) {
    warn "Missing Dpkg modules required by the FTP access method.\n\n";
    exit 1;
}

use Dselect::Method;
use Dselect::Method::Ftp;

# deal with arguments
my $vardir = $ARGV[0];
my $method = $ARGV[1];
my $option = $ARGV[2];

if ($option eq 'manual') {
    print "Manual package installation.\n";
    exit 0;
}
#print "vardir: $vardir, method: $method, option: $option\n";

#Defaults
my $arch = qx(dpkg --print-architecture);
$arch = 'i386' if $?;
chomp $arch;

my $logname = qx(whoami);
chomp $logname;
my $host = qx(cat /etc/mailname || dnsdomainname);
chomp $host;

$CONFIG{dldir} = 'debian';
$CONFIG{use_auth_proxy} = 0;
$CONFIG{proxyhost} = '';
$CONFIG{proxylogname} = $logname;
$CONFIG{proxypassword} = '';

my $methdir = "$vardir/methods/ftp";
my $exit = 0;
my $problem = 0;

if (-f "$methdir/vars") {
  read_config("$methdir/vars");
}

chdir "$methdir";
if (! -d 'debian') {
    mkdir 'debian', 0755;
}
# get info from user

$| = 1;

print <<"EOM";

You must supply an ftp site, use of passive mode, username, password,
path to the debian directory,list of distributions you are interested
in and place to download the binary package files to (relative to
/var/lib/dpkg/methods/ftp). You can add as much sites as you like. Later
entries will always override older ones.

Supply "?" as a password to be asked each time you connect.

Eg:      ftp site: ftp.debian.org
          passive: y
         username: anonymous
         password: $logname\@$host
          ftp dir: /debian
    distributions: dists/stable/main dists/stable/contrib
     download dir: debian

You may have to use an authenticated FTP proxy in order to reach the
FTP site:

Eg:  use auth proxy: y
              proxy: proxy.isp.com
      proxy account: $CONFIG{proxylogname}
     proxy password: ?
EOM

if (! $CONFIG{done}) {
  view_mirrors() if (yesno('y', 'Would you like to see a list of ftp mirrors'));
  add_site('ftp');
}
edit_config('ftp', $methdir);

my $ftp;
sub download {
 foreach (@{$CONFIG{site}}) {
    $ftp = do_connect(ftpsite => $_->[0],
                      ftpdir => $_->[1],
                      passive => $_->[3],
                      username => $_->[4],
                      password => $_->[5],
                      useproxy => $CONFIG{use_auth_proxy},
                      proxyhost => $CONFIG{proxyhost},
                      proxylogname => $CONFIG{proxylogname},
                      proxypassword => $CONFIG{proxypassword});

    my @dists = @{$_->[2]};

    foreach my $dist (@dists) {
	my $dir = "$dist/binary-$arch";
	print "Checking $dir...\n";
#	if (!$ftp->pasv()) { print $ftp->message . "\n"; die 'error'; }
	my @dirlst = $ftp->ls("$dir/");
	my $got_pkgfile = 0;

	foreach my $line (@dirlst) {
	    if($line =~ /Packages/) {
		$got_pkgfile = 1;
	    }
	}
	if( !$got_pkgfile) {
	    print "warning: could not find a Packages file in $dir\n",
	    "This may not be a problem if the directory is a symbolic link\n";
	    $problem = 1;
	}
    }
    print "Closing ftp connection...\n";
    $ftp->quit();
  }
}

# download stuff (protect from ^C)
print "\nUsing FTP to check directories...(stop with ^C)\n\n";
eval {
    local $SIG{INT} = sub {
	die "interrupted!\n";
    };
    download();
};
if($@) {
    $ftp->quit();
    print 'FTP ERROR - ';
    if ($@ eq 'connect') {
	print "config was untested\n";
    } else {
	print "$@\n";
    }
    $exit = 1;
};

# output new vars file
$CONFIG{done} = 1;
store_config("$methdir/vars");
chmod  0600, "$methdir/vars";

if($exit || $problem) {
    print "Press <enter> to continue\n";
    <STDIN>;
}

exit $exit;
