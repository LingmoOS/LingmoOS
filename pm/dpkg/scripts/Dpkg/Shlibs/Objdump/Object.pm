# Copyright © 2007-2010 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2007-2009,2012-2015,2017-2018 Guillem Jover <guillem@debian.org>
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

Dpkg::Shlibs::Objdump::Object - represent an object from objdump output

=head1 DESCRIPTION

This module provides a class to represent an object parsed from
L<objdump(1)> output.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Shlibs::Objdump::Object 0.01;

use strict;
use warnings;
use feature qw(state);

use Dpkg::Gettext;
use Dpkg::ErrorHandling;
use Dpkg::Path qw(find_command);
use Dpkg::Arch qw(debarch_to_gnutriplet get_build_arch get_host_arch);

sub new {
    my $this = shift;
    my $file = shift // '';
    my $class = ref($this) || $this;
    my $self = {};
    bless $self, $class;

    $self->reset;
    if ($file) {
	$self->analyze($file);
    }

    return $self;
}

sub reset {
    my $self = shift;

    $self->{file} = '';
    $self->{id} = '';
    $self->{HASH} = '';
    $self->{GNU_HASH} = '';
    $self->{INTERP} = 0;
    $self->{SONAME} = '';
    $self->{NEEDED} = [];
    $self->{RPATH} = [];
    $self->{dynsyms} = {};
    $self->{flags} = {};
    $self->{dynrelocs} = {};

    return $self;
}

sub _select_objdump {
    # Decide which objdump to call
    if (get_build_arch() ne get_host_arch()) {
        my $od = debarch_to_gnutriplet(get_host_arch()) . '-objdump';
        return $od if find_command($od);
    }
    return 'objdump';
}

sub analyze {
    my ($self, $file) = @_;

    $file ||= $self->{file};
    return unless $file;

    $self->reset;
    $self->{file} = $file;

    $self->{exec_abi} = Dpkg::Shlibs::Objdump::get_format($file);

    if (not defined $self->{exec_abi}) {
        warning(g_("unknown executable format in file '%s'"), $file);
        return;
    }

    state $OBJDUMP = _select_objdump();
    local $ENV{LC_ALL} = 'C';
    open(my $objdump, '-|', $OBJDUMP, '-w', '-f', '-p', '-T', '-R', $file)
        or syserr(g_('cannot fork for %s'), $OBJDUMP);
    my $ret = $self->parse_objdump_output($objdump);
    close($objdump);
    return $ret;
}

sub parse_objdump_output {
    my ($self, $fh) = @_;

    my $section = 'none';
    while (<$fh>) {
	s/\s*$//;
	next if length == 0;

	if (/^DYNAMIC SYMBOL TABLE:/) {
	    $section = 'dynsym';
	    next;
	} elsif (/^DYNAMIC RELOCATION RECORDS/) {
	    $section = 'dynreloc';
	    $_ = <$fh>; # Skip header
	    next;
	} elsif (/^Dynamic Section:/) {
	    $section = 'dyninfo';
	    next;
	} elsif (/^Program Header:/) {
	    $section = 'program';
	    next;
	} elsif (/^Version definitions:/) {
	    $section = 'verdef';
	    next;
	} elsif (/^Version References:/) {
	    $section = 'verref';
	    next;
	}

	if ($section eq 'dynsym') {
	    $self->parse_dynamic_symbol($_);
	} elsif ($section eq 'dynreloc') {
	    if (/^\S+\s+(\S+)\s+(.+)$/) {
		$self->{dynrelocs}{$2} = $1;
	    } else {
		warning(g_("couldn't parse dynamic relocation record: %s"), $_);
	    }
	} elsif ($section eq 'dyninfo') {
	    if (/^\s*NEEDED\s+(\S+)/) {
		push @{$self->{NEEDED}}, $1;
	    } elsif (/^\s*SONAME\s+(\S+)/) {
		$self->{SONAME} = $1;
	    } elsif (/^\s*HASH\s+(\S+)/) {
		$self->{HASH} = $1;
	    } elsif (/^\s*GNU_HASH\s+(\S+)/) {
		$self->{GNU_HASH} = $1;
	    } elsif (/^\s*RUNPATH\s+(\S+)/) {
                # RUNPATH takes precedence over RPATH but is
                # considered after LD_LIBRARY_PATH while RPATH
                # is considered before (if RUNPATH is not set).
                my $runpath = $1;
                $self->{RPATH} = [ split /:/, $runpath ];
	    } elsif (/^\s*RPATH\s+(\S+)/) {
                my $rpath = $1;
                unless (scalar(@{$self->{RPATH}})) {
                    $self->{RPATH} = [ split /:/, $rpath ];
                }
	    }
        } elsif ($section eq 'program') {
            if (/^\s*INTERP\s+/) {
                $self->{INTERP} = 1;
            }
	} elsif ($section eq 'none') {
	    if (/^\s*.+:\s*file\s+format\s+(\S+)$/) {
		$self->{format} = $1;
	    } elsif (/^architecture:\s*\S+,\s*flags\s*\S+:$/) {
		# Parse 2 lines of "-f"
		# architecture: i386, flags 0x00000112:
		# EXEC_P, HAS_SYMS, D_PAGED
		# start address 0x08049b50
		$_ = <$fh>;
		chomp;
		$self->{flags}{$_} = 1 foreach (split(/,\s*/));
	    }
	}
    }
    # Update status of dynamic symbols given the relocations that have
    # been parsed after the symbols...
    $self->apply_relocations();

    return $section ne 'none';
}

# Output format of objdump -w -T
#
# /lib/libc.so.6:     file format elf32-i386
#
# DYNAMIC SYMBOL TABLE:
# 00056ef0 g    DF .text  000000db  GLIBC_2.2   getwchar
# 00000000 g    DO *ABS*  00000000  GCC_3.0     GCC_3.0
# 00069960  w   DF .text  0000001e  GLIBC_2.0   bcmp
# 00000000  w   D  *UND*  00000000              _pthread_cleanup_pop_restore
# 0000b788 g    DF .text  0000008e  Base        .protected xine_close
# 0000b788 g    DF .text  0000008e              .hidden IA__g_free
# |        ||||||| |      |         |           |
# |        ||||||| |      |         Version str (.visibility) + Symbol name
# |        ||||||| |      Alignment
# |        ||||||| Section name (or *UND* for an undefined symbol)
# |        ||||||F=Function,f=file,O=object
# |        |||||d=debugging,D=dynamic
# |        ||||I=Indirect
# |        |||W=warning
# |        ||C=constructor
# |        |w=weak
# |        g=global,l=local,!=both global/local
# Size of the symbol
#
# GLIBC_2.2 is the version string associated to the symbol
# (GLIBC_2.2) is the same but the symbol is hidden, a newer version of the
# symbol exist

my $vis_re = qr/(\.protected|\.hidden|\.internal|0x\S+)/;
my $dynsym_re = qr<
    ^
    [0-9a-f]+                   # Symbol size
    \ (.{7})                    # Flags
    \s+(\S+)                    # Section name
    \s+[0-9a-f]+                # Alignment
    (?:\s+(\S+))?               # Version string
    (?:\s+$vis_re)?             # Visibility
    \s+(.+)                     # Symbol name
>x;

sub parse_dynamic_symbol {
    my ($self, $line) = @_;
    if ($line =~ $dynsym_re) {
	my ($flags, $sect, $ver, $vis, $name) = ($1, $2, $3, $4, $5);

	# Special case if version is missing but extra visibility
	# attribute replaces it in the match
	if (defined($ver) and $ver =~ /^$vis_re$/) {
	    $vis = $ver;
	    $ver = '';
	}

	# Cleanup visibility field
	$vis =~ s/^\.// if defined($vis);

	my $symbol = {
		name => $name,
		version => $ver // '',
		section => $sect,
		dynamic => substr($flags, 5, 1) eq 'D',
		debug => substr($flags, 5, 1) eq 'd',
		type => substr($flags, 6, 1),
		weak => substr($flags, 1, 1) eq 'w',
		local => substr($flags, 0, 1) eq 'l',
		global => substr($flags, 0, 1) eq 'g',
		visibility => $vis // '',
		hidden => '',
		defined => $sect ne '*UND*'
	    };

	# Handle hidden symbols
	if (defined($ver) and $ver =~ /^\((.*)\)$/) {
	    $ver = $1;
	    $symbol->{version} = $1;
	    $symbol->{hidden} = 1;
	}

	# Register symbol
	$self->add_dynamic_symbol($symbol);
    } elsif ($line =~ /^[0-9a-f]+ (.{7})\s+(\S+)\s+[0-9a-f]+/) {
	# Same start but no version and no symbol ... just ignore
    } elsif ($line =~ /^REG_G\d+\s+/) {
	# Ignore some s390-specific output like
	# REG_G6           g     R *UND*      0000000000000000              #scratch
    } else {
	warning(g_("couldn't parse dynamic symbol definition: %s"), $line);
    }
}

sub apply_relocations {
    my $self = shift;
    foreach my $sym (values %{$self->{dynsyms}}) {
	# We want to mark as undefined symbols those which are currently
	# defined but that depend on a copy relocation
	next if not $sym->{defined};

        my @relocs;

        # When objdump qualifies the symbol with a version it will use @ when
        # the symbol is in an undefined section (which we discarded above, or
        # @@ otherwise.
        push @relocs, $sym->{name} . '@@' . $sym->{version} if $sym->{version};

        # Symbols that are not versioned, or versioned but shown with objdump
        # from binutils < 2.26, do not have a version appended.
        push @relocs, $sym->{name};

        foreach my $reloc (@relocs) {
            next if not exists $self->{dynrelocs}{$reloc};
            next if not $self->{dynrelocs}{$reloc} =~ /^R_.*_COPY$/;

	    $sym->{defined} = 0;
            last;
	}
    }
}

sub add_dynamic_symbol {
    my ($self, $symbol) = @_;
    $symbol->{objid} = $symbol->{soname} = $self->get_id();
    $symbol->{soname} =~ s{^.*/}{} unless $self->{SONAME};
    if ($symbol->{version}) {
	$self->{dynsyms}{$symbol->{name} . '@' . $symbol->{version}} = $symbol;
    } else {
	$self->{dynsyms}{$symbol->{name} . '@Base'} = $symbol;
    }
}

sub get_id {
    my $self = shift;
    return $self->{SONAME} || $self->{file};
}

sub get_symbol {
    my ($self, $name) = @_;
    if (exists $self->{dynsyms}{$name}) {
	return $self->{dynsyms}{$name};
    }
    if ($name !~ /@/) {
        if (exists $self->{dynsyms}{$name . '@Base'}) {
            return $self->{dynsyms}{$name . '@Base'};
        }
    }
    return;
}

sub get_exported_dynamic_symbols {
    my $self = shift;
    return grep {
        $_->{defined} && $_->{dynamic} && !$_->{local}
    } values %{$self->{dynsyms}};
}

sub get_undefined_dynamic_symbols {
    my $self = shift;
    return grep {
        (!$_->{defined}) && $_->{dynamic}
    } values %{$self->{dynsyms}};
}

sub get_needed_libraries {
    my $self = shift;
    return @{$self->{NEEDED}};
}

sub is_executable {
    my $self = shift;
    return (exists $self->{flags}{EXEC_P} && $self->{flags}{EXEC_P}) ||
           (exists $self->{INTERP} && $self->{INTERP});
}

sub is_public_library {
    my $self = shift;
    return exists $self->{flags}{DYNAMIC} && $self->{flags}{DYNAMIC}
	&& exists $self->{SONAME} && $self->{SONAME};
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
