# Copyright © 2008-2010 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2008-2022 Guillem Jover <guillem@debian.org>
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

Dpkg::Compression::Process - run compression/decompression processes

=head1 DESCRIPTION

This module provides an object oriented interface to run and manage
compression/decompression processes.

=cut

package Dpkg::Compression::Process 1.00;

use strict;
use warnings;

use Carp;

use Dpkg::Compression;
use Dpkg::ErrorHandling;
use Dpkg::Gettext;
use Dpkg::IPC;

=head1 METHODS

=over 4

=item $proc = Dpkg::Compression::Process->new(%opts)

Create a new instance of the object.

Options:

=over

=item B<compression>

See $proc->set_compression().

=item B<compression_level>

See $proc->set_compression_level().

=back

=cut

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;
    my $self = {};
    bless $self, $class;
    $self->set_compression($opts{compression} || compression_get_default());
    $self->set_compression_level($opts{compression_level} ||
        compression_get_default_level());
    return $self;
}

=item $proc->set_compression($comp)

Select the compression method to use. It errors out if the method is not
supported according to compression_is_supported() (of
L<Dpkg::Compression>).

=cut

sub set_compression {
    my ($self, $method) = @_;
    error(g_('%s is not a supported compression method'), $method)
        unless compression_is_supported($method);
    $self->{compression} = $method;
}

=item $proc->set_compression_level($level)

Select the compression level to use. It errors out if the level is not
valid according to compression_is_valid_level() (of
L<Dpkg::Compression>).

=cut

sub set_compression_level {
    my ($self, $level) = @_;

    compression_set_level($self->{compression}, $level);
}

=item @exec = $proc->get_compress_cmdline()

=item @exec = $proc->get_uncompress_cmdline()

Returns a list ready to be passed to exec(), its first element is the
program name (either for compression or decompression) and the following
elements are parameters for the program.

When executed the program acts as a filter between its standard input
and its standard output.

=cut

sub get_compress_cmdline {
    my $self = shift;

    return compression_get_cmdline_compress($self->{compression});
}

sub get_uncompress_cmdline {
    my $self = shift;

    return compression_get_cmdline_decompress($self->{compression});
}

sub _check_opts {
    my ($self, %opts) = @_;
    # Check for proper cleaning before new start
    error(g_('Dpkg::Compression::Process can only start one subprocess at a time'))
        if $self->{pid};
    # Check options
    my $to = my $from = 0;
    foreach my $thing (qw(file handle string pipe)) {
        $to++ if $opts{"to_$thing"};
        $from++ if $opts{"from_$thing"};
    }
    croak 'exactly one to_* parameter is needed' if $to != 1;
    croak 'exactly one from_* parameter is needed' if $from != 1;
    return %opts;
}

=item $proc->compress(%opts)

Starts a compressor program. You must indicate where it will read its
uncompressed data from and where it will write its compressed data to.
This is accomplished by passing one parameter C<to_*> and one parameter
C<from_*> as accepted by Dpkg::IPC::spawn().

You must call wait_end_process() after having called this method to
properly close the sub-process (and verify that it exited without error).

Options:

See Dpkg::IPC::spawn().

=cut

sub compress {
    my ($self, %opts) = @_;

    $self->_check_opts(%opts);
    my @prog = $self->get_compress_cmdline();
    $opts{exec} = \@prog;
    $self->{cmdline} = "@prog";
    $self->{pid} = spawn(%opts);
    delete $self->{pid} if $opts{to_string}; # wait_child already done
}

=item $proc->uncompress(%opts)

Starts a decompressor program. You must indicate where it will read its
compressed data from and where it will write its uncompressed data to.
This is accomplished by passing one parameter C<to_*> and one parameter
C<from_*> as accepted by Dpkg::IPC::spawn().

You must call wait_end_process() after having called this method to
properly close the sub-process (and verify that it exited without error).

Options:

See Dpkg::IPC::spawn().

=cut

sub uncompress {
    my ($self, %opts) = @_;

    $self->_check_opts(%opts);
    my @prog = $self->get_uncompress_cmdline();
    $opts{exec} = \@prog;
    $self->{cmdline} = "@prog";
    $self->{pid} = spawn(%opts);
    delete $self->{pid} if $opts{to_string}; # wait_child already done
}

=item $proc->wait_end_process(%opts)

Call Dpkg::IPC::wait_child() to wait until the sub-process has exited
and verify its return code. Any given option will be forwarded to
the wait_child() function. Most notably you can use the "nocheck" option
to verify the return code yourself instead of letting wait_child() do
it for you.

Options:

See Dpkg::IPC::wait_child().

=cut

sub wait_end_process {
    my ($self, %opts) = @_;
    $opts{cmdline} //= $self->{cmdline};
    wait_child($self->{pid}, %opts) if $self->{pid};
    delete $self->{pid};
    delete $self->{cmdline};
}

=back

=head1 CHANGES

=head2 Version 1.00 (dpkg 1.15.6)

Mark the module as public.

=cut

1;
