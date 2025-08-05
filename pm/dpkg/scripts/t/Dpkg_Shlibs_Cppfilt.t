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
use Test::Dpkg qw(:needs);

use Config;

test_needs_command('c++filt');

if (defined $Config{bin_ELF} && $Config{bin_ELF} eq 'define') {
    plan tests => 154;
} else {
    plan skip_all => 'only ELF is currently supported';
}

use_ok('Dpkg::Shlibs::Cppfilt');

# Simple C++ demangling tests
is(cppfilt_demangle_cpp('_ZNSt10istrstreamC1EPKcl'),
    'std::istrstream::istrstream(char const*, long)',
    'demangle symbol');
is(cppfilt_demangle_cpp('_ZNSt10istrstreamC1EPKcl@Base'),
    'std::istrstream::istrstream(char const*, long)@Base',
    'demangle symbol with extra postfix');
is(cppfilt_demangle_cpp('foobar _ZNSt10istrstreamC1EPKcl@Base'),
    'foobar std::istrstream::istrstream(char const*, long)@Base',
    'demangle symbol with garbage around it');
is(cppfilt_demangle_cpp('_ZN13AngleBracketsrSEi'),
    'AngleBrackets::operator>>=(int)',
    'demangle symbol exempt from normalized angle brackets');
is(cppfilt_demangle_cpp('_ZN13AngleBracketsrSIfEEvT_'),
    'void AngleBrackets::operator>>=<float>(float)',
    'demangle template symbol exempt from normalized angle brackets');
is(cppfilt_demangle_cpp('FoobarInvalidSymbol'), undef,
    'non-demanglable string');

# Mass C++ demangling. Checking if c++filt does not hang and cppfilt_demangle()
# immediately provides a correct answer to the caller (i.e. no buffering).
my @mangledtext = split(/\n+/s, <<'END');
0000000000000000      DF *UND*  0000000000000000  GCC_3.0     _Unwind_SetIP
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 __towlower_l
0000000000000000      DO *UND*  0000000000000000  GLIBC_2.2.5 stdout
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 wmemset
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 fflush
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 getc
0000000000000000  w   D  *UND*  0000000000000000              pthread_join
00000000000cfc22 g    DO .rodata        0000000000000001  GLIBCXX_3.4 _ZNSt14numeric_limitsIyE17has_signaling_NaNE
0000000000088d80  w   DF .text  0000000000000064  GLIBCXX_3.4 _ZNSt11__timepunctIcEC2Em
00000000002f40a0  w   DO .data.rel.ro   0000000000000020  GLIBCXX_3.4 _ZTTSt14basic_ifstreamIwSt11char_traitsIwEE
000000000005a5f0 g    DF .text  0000000000000063  GLIBCXX_3.4.11 _ZNVSt9__atomic011atomic_flag5clearESt12memory_order
00000000000bdc20  w   DF .text  0000000000000024  GLIBCXX_3.4 _ZNSbIwSt11char_traitsIwESaIwEEC1EPKwmRKS1_
0000000000063850 g    DF .text  0000000000000062  GLIBCXX_3.4 _ZNSt5ctypeIwED2Ev
00000000000898b0  w   DF .text  0000000000000255  GLIBCXX_3.4 _ZNKSt8time_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE3putES3_RSt8ios_basecPK2tmPKcSB_

00000000000bff30 g    DF .text  0000000000000019  _ZNSt18condition_variable10notify_oneEv@GLIBCXX_3.4.11
00000000000666a0 g    DF .text  000000000000003f  _ZNKSt3tr14hashIRKSbIwSt11char_traitsIwESaIwEEEclES6_@GLIBCXX_3.4.10
00000000002f6160  w   DO .data.rel.ro   0000000000000050  _ZTTSt18basic_stringstreamIcSt11char_traitsIcESaIcEE@GLIBCXX_3.4

0000000000001170 g    DF .text  0000000000000001  _ZN13AngleBracketsrSEi
0000000000001190 g    DF .text  0000000000000001  _ZN13AngleBracketsrSIfEEvT_
0000000000001180 g    DF .text  0000000000000001  _ZN13AngleBracketsrsIfEEvT_
0000000000001160 g    DF .text  0000000000000001  _ZN13AngleBracketsrsEi
END

my @demangledtext = split(/\n+/s, <<'END');
0000000000000000      DF *UND*  0000000000000000  GCC_3.0     _Unwind_SetIP
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 __towlower_l
0000000000000000      DO *UND*  0000000000000000  GLIBC_2.2.5 stdout
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 wmemset
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 fflush
0000000000000000      DF *UND*  0000000000000000  GLIBC_2.2.5 getc
0000000000000000  w   D  *UND*  0000000000000000              pthread_join
00000000000cfc22 g    DO .rodata        0000000000000001  GLIBCXX_3.4 std::numeric_limits<unsigned long long>::has_signaling_NaN
0000000000088d80  w   DF .text  0000000000000064  GLIBCXX_3.4 std::__timepunct<char>::__timepunct(unsigned long)
00000000002f40a0  w   DO .data.rel.ro   0000000000000020  GLIBCXX_3.4 VTT for std::basic_ifstream<wchar_t, std::char_traits<wchar_t> >
000000000005a5f0 g    DF .text  0000000000000063  GLIBCXX_3.4.11 std::__atomic0::atomic_flag::clear(std::memory_order) volatile
00000000000bdc20  w   DF .text  0000000000000024  GLIBCXX_3.4 std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >::basic_string(wchar_t const*, unsigned long, std::allocator<wchar_t> const&)
0000000000063850 g    DF .text  0000000000000062  GLIBCXX_3.4 std::ctype<wchar_t>::~ctype()
00000000000898b0  w   DF .text  0000000000000255  GLIBCXX_3.4 std::time_put<char, std::ostreambuf_iterator<char, std::char_traits<char> > >::put(std::ostreambuf_iterator<char, std::char_traits<char> >, std::ios_base&, char, tm const*, char const*, char const*) const

00000000000bff30 g    DF .text  0000000000000019  std::condition_variable::notify_one()@GLIBCXX_3.4.11
00000000000666a0 g    DF .text  000000000000003f  std::tr1::hash<std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > const&>::operator()(std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > const&) const@GLIBCXX_3.4.10
00000000002f6160  w   DO .data.rel.ro   0000000000000050  VTT for std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> >@GLIBCXX_3.4

0000000000001170 g    DF .text  0000000000000001  AngleBrackets::operator>>=(int)
0000000000001190 g    DF .text  0000000000000001  void AngleBrackets::operator>>=<float>(float)
0000000000001180 g    DF .text  0000000000000001  void AngleBrackets::operator>><float>(float)
0000000000001160 g    DF .text  0000000000000001  AngleBrackets::operator>>(int)
END

for my $try (1 .. 7) {
    for my $i (0 .. $#mangledtext) {
	my $demangled = cppfilt_demangle_cpp($mangledtext[$i]) || $mangledtext[$i];
	is($demangled, $demangledtext[$i], "mass c++ demangling (${try}x" . (${i} + 1) . ')');
    }
}
