# Copyright © 2021-2025 Guillem Jover <guillem@debian.org>
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

Dpkg::OpenPGP::Backend::Sequoia - OpenPGP backend for Sequoia

=head1 DESCRIPTION

This module provides a class that implements the OpenPGP backend
for Sequoia-PGP.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::OpenPGP::Backend::Sequoia 0.01;

use strict;
use warnings;

use POSIX qw(:sys_wait_h);
use File::Temp;

use Dpkg::ErrorHandling;
use Dpkg::Gettext;
use Dpkg::IPC;
use Dpkg::OpenPGP::ErrorCodes;

use parent qw(Dpkg::OpenPGP::Backend);

sub DEFAULT_CMDV {
    return [ qw(sqv) ];
}

sub DEFAULT_CMD {
    return [ qw(sq) ];
}

sub has_keystore {
    my $self = shift;

    return 1 if $ENV{HOME} && -e "$ENV{HOME}/.local/share/sequoia/keystore";
    return 1 if $ENV{HOME} && -e "$ENV{HOME}/.gnupg";
    return 0;
}

sub _sq_exec
{
    my ($self, $cmd, @exec) = @_;

    my ($stdout, $stderr);
    spawn(exec => [ $cmd, @exec ],
          wait_child => 1, nocheck => 1, timeout => 10,
          to_string => \$stdout, error_to_string => \$stderr);
    if (WIFEXITED($?)) {
        my $status = WEXITSTATUS($?);
        print { *STDERR } "$stdout$stderr" if $status;
        return $status;
    } else {
        subprocerr("$cmd @exec");
    }
}

sub _sq_exec_cmdv
{
    my ($self, @exec) = @_;

    if ($self->{cmdv}) {
        return $self->_sq_exec($self->{cmdv}, @exec);
    } else {
        return $self->_sq_exec($self->{cmd}, qw(verify), @exec);
    }
}

sub _sq_exec_cmd
{
    my ($self, @exec) = @_;

    return $self->_sq_exec($self->{cmd}, @exec);
}

sub armor
{
    my ($self, $type, $in, $out) = @_;

    return $self->SUPER::armor($type, $in, $out) unless $self->{cmd};

    # We ignore the $type, and let "sq" handle this automatically.
    my $rc = $self->_sq_exec_cmd(qw(packet armor --output), $out, $in);
    return OPENPGP_BAD_DATA if $rc;
    return OPENPGP_OK;
}

sub dearmor
{
    my ($self, $type, $in, $out) = @_;

    return $self->SUPER::dearmor($type, $in, $out) unless $self->{cmd};

    # We ignore the $type, and let "sq" handle this automatically.
    my $rc = $self->_sq_exec_cmd(qw(packet dearmor --output), $out, $in);
    return OPENPGP_BAD_DATA if $rc;
    return OPENPGP_OK;
}

sub inline_verify
{
    my ($self, $inlinesigned, $data, @certs) = @_;

    return OPENPGP_MISSING_CMD unless ($self->{cmdv} || $self->{cmd});
    return OPENPGP_NO_SIG if @certs == 0;

    # XXX: sqv does not support --signer-file. See:
    #   <https://gitlab.com/sequoia-pgp/sequoia-sqv/-/issues/11>.
    my $keyring_opt = $self->{cmdv} ? '--keyring' : '--signer-file';

    my @opts;
    # Select stateless mode for sq.
    push @opts, '--home=none' unless $self->{cmdv};
    push @opts, '--cleartext';
    push @opts, map { ($keyring_opt, $_) } @certs;
    my $tmpdir;
    if (not defined $data) {
        # XXX: For sqv the --output option is mandatory. See:
        #   <https://gitlab.com/sequoia-pgp/sequoia-sqv/-/issues/12>.
        # XXX: For sqv the --output option does not accept «-» as stdout,
        #   which we would discard at spawn() time, and would then not need
        #   to pass /dev/null or use a temporary file. See:
        #   <https://gitlab.com/sequoia-pgp/sequoia-sqv/-/issues/15>.
        # XXX: We need a temporary file because we cannot pass /dev/null. See:
        #   <https://gitlab.com/sequoia-pgp/sequoia-sq/-/issues/561> and
        #   <https://gitlab.com/sequoia-pgp/sequoia-sqv/-/issues/13>.
        if ($self->{cmdv}) {
            $tmpdir = File::Temp->newdir(
                TEMPLATE => 'dpkg-openpgp-backend-sq-verify-XXXXXX',
                TMPDIR => 1,
            );
            $data = "$tmpdir/output";
        } else {
            $data = '-';
        }
    }
    push @opts, '--output', $data;

    my $rc = $self->_sq_exec_cmdv(@opts, $inlinesigned);
    return OPENPGP_NO_SIG if $rc;
    return OPENPGP_OK;
}

sub verify
{
    my ($self, $data, $sig, @certs) = @_;

    return OPENPGP_MISSING_CMD unless ($self->{cmdv} || $self->{cmd});
    return OPENPGP_NO_SIG if @certs == 0;

    # XXX: sqv does not support --signer-file. See:
    #   <https://gitlab.com/sequoia-pgp/sequoia-sqv/-/issues/11>.
    my $keyring_opt = $self->{cmdv} ? '--keyring' : '--signer-file';

    my @opts;
    # Select stateless mode for sq.
    push @opts, '--home=none' unless $self->{cmdv};
    push @opts, map { ($keyring_opt, $_) } @certs;
    push @opts, '--signature-file', $sig;

    my $rc = $self->_sq_exec_cmdv(@opts, $data);
    return OPENPGP_NO_SIG if $rc;
    return OPENPGP_OK;
}

sub inline_sign
{
    my ($self, $data, $inlinesigned, $key) = @_;

    return OPENPGP_MISSING_CMD unless $self->{cmd};
    return OPENPGP_NEEDS_KEYSTORE if ! $self->can_use_key($key);

    my @opts;
    push @opts, '--cleartext';
    if ($key->type eq 'keyfile') {
        push @opts, '--signer-file', $key->handle;
    } elsif ($key->type eq 'userid') {
        push @opts, '--signer-userid', $key->handle;
    } else {
        push @opts, '--signer', $key->handle;
    }
    push @opts, '--output', $inlinesigned;

    my $rc = $self->_sq_exec_cmd('sign', @opts, $data);
    if ($rc) {
        # XXX: Ideally sq would emit this kind of hint itself when it knows it
        #      might apply, but for now this is probably better than nothing.
        hint(g_('imported own keys might be missing ownership information, try:'));
        hint(g_('  sq pki link authorize --cert FINGERPRINT --all --unconstrained'));
        return OPENPGP_KEY_CANNOT_SIGN;
    }
    return OPENPGP_OK;
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
