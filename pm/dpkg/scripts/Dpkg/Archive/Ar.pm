# Copyright © 2023-2024 Guillem Jover <guillem@debian.org>
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

=encoding utf8

=head1 NAME

Dpkg::Archive::Ar - Unix ar archive support

=head1 DESCRIPTION

This module provides a class to handle Unix ar archives.
It support the common format, with no GNU or BSD extensions.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Archive::Ar 0.01;

use strict;
use warnings;

use Carp;
use Fcntl qw(:seek);
use IO::File;

use Dpkg::ErrorHandling;
use Dpkg::Gettext;

my $AR_MAGIC = "!<arch>\n";
my $AR_MAGIC_LEN = 8;
my $AR_FMAG = "\`\n";
my $AR_HDR_LEN = 60;

=head1 METHODS

=over 8

=item $ar = Dpkg::Archive::Ar->new(%opts)

Create a new object to handle Unix ar archives.

Options:

=over 8

=item B<filename>

The filename for the archive to open or create.

=item B<create>

A boolean denoting whether the archive should be created,
otherwise if it does not exist the constructor will not open, create or
scan it.

=back

=cut

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;
    my $self = {
        filename => undef,
        fh => undef,
        # XXX: If we promote this out from internal use, we should make this
        # default to the archive mtime, or be overridable like in libdpkg,
        # so that it can be initialized from SOURCE_DATE_EPOCH for example.
        time => 0,
        size => 0,
        members => [],
    };
    bless $self, $class;

    if ($opts{filename}) {
        if ($opts{create}) {
            $self->create_archive($opts{filename});
        } elsif (-e $opts{filename}) {
            $self->open_archive($opts{filename});
        }
        if (-e $opts{filename}) {
            $self->scan_archive();
        }
    }

    return $self;
}

sub init_archive {
    my $self = shift;

    $self->{fh}->binmode();
    $self->{fh}->stat
        or syserr(g_('cannot get archive %s size'), $self->{filename});
    $self->{size} = -s _;

    return;
}

=item $ar->create_archive($filename)

Create the archive.

=cut

sub create_archive {
    my ($self, $filename) = @_;

    if (defined $self->{fh}) {
        croak 'the object has already been initialized with another file';
    }

    $self->{filename} = $filename;
    $self->{fh} = IO::File->new($filename, '+>')
        or syserr(g_('cannot open or create archive %s'), $filename);
    $self->init_archive();
    $self->{fh}->write($AR_MAGIC, $AR_MAGIC_LEN)
        or syserr(g_('cannot write magic into archive %s'), $filename);

    return;
}

=item $ar->open_archive($filename)

Open the archive.

=cut

sub open_archive {
    my ($self, $filename) = @_;

    if (defined $self->{fh}) {
        croak 'the object has already been initialized with another file';
    }

    $self->{filename} = $filename;
    $self->{fh} = IO::File->new($filename, '+<')
        or syserr(g_('cannot open or create archive %s'), $filename);
    $self->init_archive();

    return;
}

sub _read_buf {
    my ($self, $subject, $size) = @_;

    my $buf;
    my $offs = $self->{fh}->tell();
    my $n = $self->{fh}->read($buf, $size);
    if (not defined $n) {
        # TRANSLATORS: The first %s string is either "the archive magic" or
        # "a file header".
        syserr(g_('cannot read %s; archive %s at offset %d'),
               $subject, $offs, $self->{filename});
    } elsif ($n == 0) {
        return;
    } elsif ($n != $size) {
        # TRANSLATORS: The first %s string is either "the archive magic" or
        # "a file header".
        error(g_('cannot read %s; archive %s is truncated at offset %d'),
              $subject, $offs, $self->{filename});
    }

    return $buf;
}

=item $ar->parse_magic()

Reads and parses the archive magic string, and validates it.

=cut

sub parse_magic {
    my $self = shift;

    my $magic = $self->_read_buf(g_('the archive magic'), $AR_MAGIC_LEN)
        or error(g_('archive %s contains no magic'), $self->{filename});

    if ($magic ne $AR_MAGIC) {
        error(g_('archive %s contains bad magic'), $self->{filename});
    }

    return;
}

=item $ar->parse_member()

Reads and parses the archive member and tracks it for later handling.

=cut

sub parse_member {
    my $self = shift;

    my $offs = $self->{fh}->tell();

    my $hdr = $self->_read_buf(g_('a file header'), $AR_HDR_LEN)
        or return;

    my $hdr_fmt = 'A16A12A6A6A8A10a2';
    my ($name, $time, $uid, $gid, $mode, $size, $fmag) = unpack $hdr_fmt, $hdr;

    if ($fmag ne $AR_FMAG) {
        error(g_('file header at offset %d in archive %s contains bad magic'),
              $offs, $self->{filename});
    }

    # Remove trailing spaces from the member name.
    $name =~ s{ *$}{};

    # Remove optional slash terminator (on GNU-style archives).
    $name =~ s{/$}{};

    my $member = {
        name => $name,
        time => int $time,
        uid => int $uid,
        gid => int $gid,
        mode => oct $mode,
        size => int $size,
        offs => $offs,
    };
    push @{$self->{members}}, $member;

    return $member;
}

=item $ar->skip_member($member)

Skip this member to the next one.
Get the value of a given substitution.

=cut

sub skip_member {
    my ($self, $member) = @_;

    my $size = $member->{size};
    my $offs = $member->{offs} + $AR_HDR_LEN + $size + ($size & 1);

    $self->{fh}->seek($offs, SEEK_SET)
        or syserr(g_('cannot seek into next file header at offset %d from archive %s'),
                  $offs, $self->{filename});

    return;
}

=item $ar->scan_archive()

Scan the archive for all its member files and metadata.

=cut

sub scan_archive {
    my $self = shift;

    $self->{fh}->seek(0, SEEK_SET)
        or syserr(g_('cannot seek into beginning of archive %s'),
                  $self->{filename});

    $self->parse_magic();

    while (my $member = $self->parse_member()) {
        $self->skip_member($member);
    }

    return;
}

=item $ar->get_members()

Get the list of members in the archive.

=cut

sub get_members {
    my $self = shift;

    return $self->{members};
}

sub _copy_fh_fh {
    my ($self, $if, $of, $size) = @_;

    while ($size > 0) {
        my $buf;
        my $buflen = $size > 4096 ? 4096 : $size;

        my $n = $if->{fh}->read($buf, $buflen)
            or syserr(g_('cannot read file %s'), $if->{name});

        $of->{fh}->write($buf, $n)
            or syserr(g_('cannot write file %s'), $of->{name});

        $size -= $n;
    }

    return;
}

=item $ar->extract_member($member)

Extract the specified member to the current directory.

=cut

sub extract_member {
    my ($self, $member) = @_;

    $self->{fh}->seek($member->{offs} + $AR_HDR_LEN, SEEK_SET);

    my $ofh = IO::File->new($member->{name}, '+>')
        or syserr(g_('cannot create file %s to extract from archive %s'),
                  $member->{name}, $self->{filename});

    $self->_copy_fh_fh({ fh => $self->{fh}, name => $self->{filename} },
                       { fh => $ofh, name => $member->{name} },
                      $member->{size});

    $ofh->close()
        or syserr(g_('cannot write file %s to the filesystem'),
                  $member->{name});

    return;
}

=item $ar->write_member($member)

Write the provided $member into the archive.

=cut

sub write_member {
    my ($self, $member) = @_;

    my $size = $member->{size};
    my $mode = sprintf '%o', $member->{mode};

    my $hdr_fmt = 'A16A12A6A6A8A10A2';
    my $data = pack $hdr_fmt, @{$member}{qw(name time uid gid)}, $mode, $size, $AR_FMAG;

    $self->{fh}->write($data, $AR_HDR_LEN, $member->{offs})
        or syserr(g_('cannot write file header into archive %s'),
                  $self->{filename});

    return;
}

=item $ar->add_file($filename)

Append the specified $filename into the archive.

=cut

sub add_file {
    my ($self, $filename) = @_;

    if (length $filename > 15) {
        error(g_('filename %s is too long'), $filename);
    }

    my $fh = IO::File->new($filename, '<')
        or syserr(g_('cannot open file %s to append to archive %s'),
                  $filename, $self->{filename});
    $fh->stat()
        or syserr(g_('cannot get file %s size'), $filename);
    my $size = -s _;

    my %member = (
        name => $filename,
        size => $size,
        time => $self->{time},
        mode => 0100644,
        uid => 0,
        gid => 0,
    );

    $self->write_member(\%member);
    $self->_copy_fh_fh({ fh => $fh, name => $filename },
                       { fh => $self->{fh}, name => $self->{filename} },
                       $size);
    if ($size & 1) {
        $self->{fh}->write("\n", 1)
            or syserr(g_('cannot write file %s padding to archive %s'),
                      $filename, $self->{filename});
    }

    return;
}

=item $ar->close_archive()

Close the archive and release any allocated resource.

=cut

sub close_archive {
    my $self = shift;

    $self->{fh}->close() if defined $self->{fh};
    $self->{fh} = undef;
    $self->{size} = 0;
    $self->{members} = [];

    return;
}

sub DESTROY {
    my $self = shift;

    $self->close_archive();

    return;
}

=back

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
