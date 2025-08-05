# Copyright © 1998 Richard Braakman
# Copyright © 1999 Darren Benham
# Copyright © 2000 Sean 'Shaleh' Perry
# Copyright © 2004 Frank Lichtenheld
# Copyright © 2006 Russ Allbery
# Copyright © 2007-2009 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2008-2009,2012-2014 Guillem Jover <guillem@debian.org>
#
# This program is free software; you may redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

=encoding utf8

=head1 NAME

Dpkg::Deps - parse and manipulate dependencies of Debian packages

=head1 DESCRIPTION

The Dpkg::Deps module provides classes implementing various types of
dependencies.

The most important function is deps_parse(), it turns a dependency line in
a set of Dpkg::Deps::{Simple,AND,OR,Union} objects depending on the case.

=cut

package Dpkg::Deps 1.07;

use strict;
use warnings;
use feature qw(current_sub);

our @EXPORT = qw(
    deps_concat
    deps_parse
    deps_eval_implication
    deps_iterate
    deps_compare
);

use Carp;
use Exporter qw(import);

use Dpkg::Version;
use Dpkg::Arch qw(get_host_arch get_build_arch debarch_to_debtuple);
use Dpkg::BuildProfiles qw(get_build_profiles);
use Dpkg::ErrorHandling;
use Dpkg::Gettext;
use Dpkg::Deps::Simple;
use Dpkg::Deps::Union;
use Dpkg::Deps::AND;
use Dpkg::Deps::OR;
use Dpkg::Deps::KnownFacts;

=head1 FUNCTIONS

All the deps_* functions are exported by default.

=over 4

=item deps_eval_implication($rel_p, $v_p, $rel_q, $v_q)

($rel_p, $v_p) and ($rel_q, $v_q) express two dependencies as (relation,
version). The relation variable can have the following values that are
exported by L<Dpkg::Version>: REL_EQ, REL_LT, REL_LE, REL_GT, REL_GT.

This functions returns 1 if the "p" dependency implies the "q"
dependency. It returns 0 if the "p" dependency implies that "q" is
not satisfied. It returns undef when there's no implication.

The $v_p and $v_q parameter should be L<Dpkg::Version> objects.

=cut

sub deps_eval_implication {
    my ($rel_p, $v_p, $rel_q, $v_q) = @_;

    # If versions are not valid, we can't decide of any implication
    return unless defined($v_p) and $v_p->is_valid();
    return unless defined($v_q) and $v_q->is_valid();

    # q wants an exact version, so p must provide that exact version.  p
    # disproves q if q's version is outside the range enforced by p.
    if ($rel_q eq REL_EQ) {
        if ($rel_p eq REL_LT) {
            return ($v_p <= $v_q) ? 0 : undef;
        } elsif ($rel_p eq REL_LE) {
            return ($v_p < $v_q) ? 0 : undef;
        } elsif ($rel_p eq REL_GT) {
            return ($v_p >= $v_q) ? 0 : undef;
        } elsif ($rel_p eq REL_GE) {
            return ($v_p > $v_q) ? 0 : undef;
        } elsif ($rel_p eq REL_EQ) {
            return ($v_p == $v_q);
        }
    }

    # A greater than clause may disprove a less than clause. An equal
    # cause might as well.  Otherwise, if
    # p's clause is <<, <=, or =, the version must be <= q's to imply q.
    if ($rel_q eq REL_LE) {
        if ($rel_p eq REL_GT) {
            return ($v_p >= $v_q) ? 0 : undef;
        } elsif ($rel_p eq REL_GE) {
            return ($v_p > $v_q) ? 0 : undef;
	} elsif ($rel_p eq REL_EQ) {
            return ($v_p <= $v_q) ? 1 : 0;
        } else { # <<, <=
            return ($v_p <= $v_q) ? 1 : undef;
        }
    }

    # Similar, but << is stronger than <= so p's version must be << q's
    # version if the p relation is <= or =.
    if ($rel_q eq REL_LT) {
        if ($rel_p eq REL_GT or $rel_p eq REL_GE) {
            return ($v_p >= $v_p) ? 0 : undef;
        } elsif ($rel_p eq REL_LT) {
            return ($v_p <= $v_q) ? 1 : undef;
	} elsif ($rel_p eq REL_EQ) {
            return ($v_p < $v_q) ? 1 : 0;
        } else { # <<, <=
            return ($v_p < $v_q) ? 1 : undef;
        }
    }

    # Same logic as above, only inverted.
    if ($rel_q eq REL_GE) {
        if ($rel_p eq REL_LT) {
            return ($v_p <= $v_q) ? 0 : undef;
        } elsif ($rel_p eq REL_LE) {
            return ($v_p < $v_q) ? 0 : undef;
	} elsif ($rel_p eq REL_EQ) {
            return ($v_p >= $v_q) ? 1 : 0;
        } else { # >>, >=
            return ($v_p >= $v_q) ? 1 : undef;
        }
    }
    if ($rel_q eq REL_GT) {
        if ($rel_p eq REL_LT or $rel_p eq REL_LE) {
            return ($v_p <= $v_q) ? 0 : undef;
        } elsif ($rel_p eq REL_GT) {
            return ($v_p >= $v_q) ? 1 : undef;
	} elsif ($rel_p eq REL_EQ) {
            return ($v_p > $v_q) ? 1 : 0;
        } else {
            return ($v_p > $v_q) ? 1 : undef;
        }
    }

    return;
}

=item $dep = deps_concat(@dep_list)

This function concatenates multiple dependency lines into a single line,
joining them with ", " if appropriate, and always returning a valid string.

=cut

sub deps_concat {
    my (@dep_list) = @_;

    return join ', ', grep { defined } @dep_list;
}

=item $dep = deps_parse($line, %opts)

This function parses the dependency line and returns an object, either a
L<Dpkg::Deps::AND> or a L<Dpkg::Deps::Union>. Various options can alter the
behavior of that function.

=over 4

=item use_arch (defaults to 1)

Take into account the architecture restriction part of the dependencies.
Set to 0 to completely ignore that information.

=item host_arch (defaults to the current architecture)

Define the host architecture. By default it uses
Dpkg::Arch::get_host_arch() to identify the proper architecture.

=item build_arch (defaults to the current architecture)

Define the build architecture. By default it uses
Dpkg::Arch::get_build_arch() to identify the proper architecture.

=item reduce_arch (defaults to 0)

If set to 1, ignore dependencies that do not concern the current host
architecture. This implicitly strips off the architecture restriction
list so that the resulting dependencies are directly applicable to the
current architecture.

=item use_profiles (defaults to 1)

Take into account the profile restriction part of the dependencies. Set
to 0 to completely ignore that information.

=item build_profiles (defaults to no profile)

Define the active build profiles. By default no profile is defined.

=item reduce_profiles (defaults to 0)

If set to 1, ignore dependencies that do not concern the current build
profile. This implicitly strips off the profile restriction formula so
that the resulting dependencies are directly applicable to the current
profiles.

=item reduce_restrictions (defaults to 0)

If set to 1, ignore dependencies that do not concern the current set of
restrictions. This implicitly strips off any architecture restriction list
or restriction formula so that the resulting dependencies are directly
applicable to the current restriction.
This currently implies C<reduce_arch> and C<reduce_profiles>, and overrides
them if set.

=item union (defaults to 0)

If set to 1, returns a L<Dpkg::Deps::Union> instead of a L<Dpkg::Deps::AND>.
Use this when parsing non-dependency fields like Conflicts.

=item virtual (defaults to 0)

If set to 1, allow only virtual package version relations, that is none,
or "=".
This should be set whenever working with Provides fields.

=item build_dep (defaults to 0)

If set to 1, allow build-dep only arch qualifiers, that is ":native".
This should be set whenever working with build-deps.

=item tests_dep (defaults to 0)

If set to 1, allow tests-specific package names in dependencies, that is
"@" and "@builddeps@" (since dpkg 1.18.7). This should be set whenever
working with dependency fields from F<debian/tests/control>.

This option implicitly (and forcibly) enables C<build_dep> because test
dependencies are based on build dependencies (since dpkg 1.22.1).

=back

=cut

sub deps_parse {
    my ($dep_line, %opts) = @_;

    # Validate arguments.
    croak "invalid host_arch $opts{host_arch}"
        if defined $opts{host_arch} and not defined debarch_to_debtuple($opts{host_arch});
    croak "invalid build_arch $opts{build_arch}"
        if defined $opts{build_arch} and not defined debarch_to_debtuple($opts{build_arch});

    $opts{use_arch} //= 1;
    $opts{reduce_arch} //= 0;
    $opts{use_profiles} //= 1;
    $opts{reduce_profiles} //= 0;
    $opts{reduce_restrictions} //= 0;
    $opts{union} //= 0;
    $opts{virtual} //= 0;
    $opts{build_dep} //= 0;
    $opts{tests_dep} //= 0;

    if ($opts{reduce_restrictions}) {
        $opts{reduce_arch} = 1;
        $opts{reduce_profiles} = 1;
    }
    if ($opts{reduce_arch}) {
        $opts{host_arch} //= get_host_arch();
        $opts{build_arch} //= get_build_arch();
    }
    if ($opts{reduce_profiles}) {
        $opts{build_profiles} //= [ get_build_profiles() ];
    }
    if ($opts{tests_dep}) {
        $opts{build_dep} = 1;
    }

    # Options for Dpkg::Deps::Simple.
    my %deps_options = (
        host_arch => $opts{host_arch},
        build_arch => $opts{build_arch},
        build_dep => $opts{build_dep},
        tests_dep => $opts{tests_dep},
    );

    # Merge in a single-line
    $dep_line =~ s/\s*[\r\n]\s*/ /g;
    # Strip trailing/leading spaces
    $dep_line =~ s/^\s+//;
    $dep_line =~ s/\s+$//;

    my @dep_list;
    foreach my $dep_and (split(/\s*,\s*/m, $dep_line)) {
        my @or_list = ();
        foreach my $dep_or (split(/\s*\|\s*/m, $dep_and)) {
	    my $dep_simple = Dpkg::Deps::Simple->new($dep_or, %deps_options);
	    if (not defined $dep_simple->{package}) {
		warning(g_("can't parse dependency %s"), $dep_or);
		return;
	    }
            if ($opts{virtual} && defined $dep_simple->{relation} &&
                $dep_simple->{relation} ne '=') {
                warning(g_('virtual dependency contains invalid relation: %s'),
                        $dep_simple->output);
                return;
            }
            $dep_simple->{arches} = undef if not $opts{use_arch};
            if ($opts{reduce_arch}) {
                $dep_simple->reduce_arch($opts{host_arch});
                next if not $dep_simple->arch_is_concerned($opts{host_arch});
	    }
            $dep_simple->{restrictions} = undef if not $opts{use_profiles};
            if ($opts{reduce_profiles}) {
                $dep_simple->reduce_profiles($opts{build_profiles});
                next if not $dep_simple->profile_is_concerned($opts{build_profiles});
	    }
	    push @or_list, $dep_simple;
        }
	next if not @or_list;
	if (scalar @or_list == 1) {
	    push @dep_list, $or_list[0];
	} else {
	    my $dep_or = Dpkg::Deps::OR->new();
	    $dep_or->add($_) foreach (@or_list);
	    push @dep_list, $dep_or;
	}
    }
    my $dep_and;
    if ($opts{union}) {
	$dep_and = Dpkg::Deps::Union->new();
    } else {
	$dep_and = Dpkg::Deps::AND->new();
    }
    foreach my $dep (@dep_list) {
        if ($opts{union} and not $dep->isa('Dpkg::Deps::Simple')) {
            warning(g_('an union dependency can only contain simple dependencies'));
            return;
        }
        $dep_and->add($dep);
    }
    return $dep_and;
}

=item $bool = deps_iterate($deps, $callback_func)

This function visits all elements of the dependency object, calling the
callback function for each element.

The callback function is expected to return true when everything is fine,
or false if something went wrong, in which case the iteration will stop.

Return the same value as the callback function.

=cut

sub deps_iterate {
    my ($deps, $callback_func) = @_;

    my $visitor_func = sub {
        foreach my $dep (@_) {
            return unless defined $dep;

            if ($dep->isa('Dpkg::Deps::Simple')) {
                return unless $callback_func->($dep);
            } else {
                return unless __SUB__->($dep->get_deps());
            }
        }
        return 1;
    };

    return $visitor_func->($deps);
}

=item deps_compare($a, $b)

Implements a comparison operator between two dependency objects.
This function is mainly used to implement the sort() method.

=back

=cut

my %relation_ordering = (
	undef => 0,
	REL_GE() => 1,
	REL_GT() => 2,
	REL_EQ() => 3,
	REL_LT() => 4,
	REL_LE() => 5,
);

sub deps_compare {
    my ($aref, $bref) = @_;

    my (@as, @bs);
    deps_iterate($aref, sub { push @as, @_ });
    deps_iterate($bref, sub { push @bs, @_ });

    while (1) {
        my ($a, $b) = (shift @as, shift @bs);
        my $aundef = not defined $a or $a->is_empty();
        my $bundef = not defined $b or $b->is_empty();

        return  0 if $aundef and $bundef;
        return -1 if $aundef;
        return  1 if $bundef;

        my $ar = $a->{relation} // 'undef';
        my $br = $b->{relation} // 'undef';
        my $av = $a->{version} // '';
        my $bv = $b->{version} // '';

        my $res = (($a->{package} cmp $b->{package}) ||
                   ($relation_ordering{$ar} <=> $relation_ordering{$br}) ||
                   ($av cmp $bv));
        return $res if $res != 0;
    }
}

=head1 CLASSES - Dpkg::Deps::*

There are several kind of dependencies. A L<Dpkg::Deps::Simple> dependency
represents a single dependency statement (it relates to one package only).
L<Dpkg::Deps::Multiple> dependencies are built on top of this class
and combine several dependencies in different manners. L<Dpkg::Deps::AND>
represents the logical "AND" between dependencies while L<Dpkg::Deps::OR>
represents the logical "OR". L<Dpkg::Deps::Multiple> objects can contain
L<Dpkg::Deps::Simple> object as well as other L<Dpkg::Deps::Multiple> objects.

In practice, the code is only meant to handle the realistic cases which,
given Debian's dependencies structure, imply those restrictions: AND can
contain Simple or OR objects, OR can only contain Simple objects.

L<Dpkg::Deps::KnownFacts> is a special class that is used while evaluating
dependencies and while trying to simplify them. It represents a set of
installed packages along with the virtual packages that they might
provide.

=head1 CHANGES

=head2 Version 1.07 (dpkg 1.20.0)

New option: Add virtual option to deps_parse().

=head2 Version 1.06 (dpkg 1.18.7; module version bumped on dpkg 1.18.24)

New option: Add tests_dep option to deps_parse().

=head2 Version 1.05 (dpkg 1.17.14)

New function: deps_iterate().

=head2 Version 1.04 (dpkg 1.17.10)

New options: Add use_profiles, build_profiles, reduce_profiles and
reduce_restrictions to deps_parse().

=head2 Version 1.03 (dpkg 1.17.0)

New option: Add build_arch option to deps_parse().

=head2 Version 1.02 (dpkg 1.17.0)

New function: deps_concat()

=head2 Version 1.01 (dpkg 1.16.1)

<Used to document changes to Dpkg::Deps::* modules before they were split.>

=head2 Version 1.00 (dpkg 1.15.6)

Mark the module as public.

=cut

1;
