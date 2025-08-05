Q1: Why does the Debian libc6-dev package create /usr/include/linux and 
/usr/include/asm directories containing header files from a specific 
kernel, instead of using the "established" convention of making those 
directories into symlinks pointing to the currently installed kernel?

A1: Occasionally, changes in the kernel headers cause problems with
the compilation of libc and of programs that use libc.  To ensure that
users are not affected by these problems, we configure libc to use the
headers from a kernel that is known to work with libc and the programs
that depend on stable kernel headers.

[Note: /usr/include/linux is now in the linux-libc-dev package.]

Q2: What if I want to compile a program that needs a feature from a
later kernel than is used by libc?

A2: In practice, most programs will not have this problem.  However,
depending on the scope of the problem you have several options available:

If the situation is one like "kernel driver development", where all use
of the machine for development is expected to be against a different set
of kernel headers than are supplied with the "stock" libc6-dev, rebuilding
the glibc packages against that particular kernel will create a full set of
libc6 packages that are "compliant" with the new kernel. All development 
done on machines with these packages installed will be done against the 
new kernel. To build libc6-dev against your particular kernel, export the
environment variable LINUX_SOURCE, set to the path to that particular kernel
source directory and then build the package.

If you want this new glibc package to stick against further upgrades, simply 
use dselect and place the packages on HOLD. This will keep them from being 
upgraded against your wishes.

If there is just one particular program/package that needs different headers,
and your kernel of choice is installed in the usual place, you can use the
-I/usr/src/linux/include option on the gcc command line, when compiling that
specific program.

Q3: My program is trying to use a kernel header that is in libc-dev, and
it is causing problems. (or) Why are the kernel headers not the same for
all architectures in Debian, for a given version of libc-dev?

A3: For starters, you should never use kernel headers directly from user
space programs. You cannot be guaranteed a consistent interface across
different versions of the kernel, much less across architectures (even for
the same version of kernel source).

Kernel headers are included with libc-dev _only_ to support those headers
in libc-dev. They are not there for userspace programs to include. If you
need kernel headers, then use one of the provided kernel-headers-x.x.x
packages provided for each architectures, or include the kernel headers
(or just the parts of the headers) you need directly in your source for
compilation. Only this way can you ensure a consistent state.

Q4: Why does Debian's glibc seem to follow the stable CVS branch?

A4: During our development cycle we follow the stable CVS branch so that
we can cause as little disruption as possible, and catch problems easier.
Differences between minor releases of glibc are known to be very large,
contain many changes, and generally affect all architectures differently.
We sync frequently with CVS so that we can take this large amount of
changes in smaller chunks, simplifying the process, and allowing us to
communicate problems to upstream much easier.

Q5: How to setup my own language/regional (locale) setting?

A5: Glibc provides "locale" defined in POSIX.  Locale is a framework
to switch between multiple languages for users who can select to use
their language, country, characters, collation order, etc.  For
example, some program display messages in your own language, if you
set the appropriate locale.  Default locale is C (or POSIX) which
behaves traditional Unix with ASCII message.  For more information,
look locale (5).

If you want to use your own locale, install "locales" package.  With
debconf interface, you can setup which locale is generated, and which
locale is used in default on your machine.

Q6: I get this message when I run a program:
	ld.so: Incorrectly built binary which accesses errno or h_errno directly.
A6:

The program is broken.  It probably says "extern int errno" somewhere,
instead of including <errno.h>.  Errno in recent glibc versions is a macro,
which calls the function __errno_location; when glibc is built with
thread-local storage enabled, there is no extern int variable named errno. 
In addition, "extern int errno" is not thread-safe.

NOTE: Currently this error message is suppressed, for the benefit of some
truly buggy programs which redirect stderr into something important, like a
network socket.

Q7: I get this error when I compile an static linked application that's
compiled with glibc 2.2 or before:
	gcc -o foo.o libbar.a
	libbar.a(obj.o): In function `func(char *)':
	: undefined reference to `__ctype_toupper'

A7: 
During glibc 2.3.x development, some symbols (__ctype_b, __ctype_toupper,
__ctype_tolower) are changed to hidden attributes.  This means old static
linked applications/libraries built with glibc 2.2.x cannot be linked on glibc
2.3.x systems any more.

But it made a lot of user applications unusable, we applied a workaround patch
for glibc in Sarge.  Therefore your applications worked OK, and you didn't
need to consider about this problem.  However, most other distros already
dropped such local modification.  For that reason, we decided to drop
supporting such old static linked applications/libraries from Etch, you need
to recompile them.

If you want to keep this workaround that was applied in Sarge for the present,
please recompile Debian glibc package with adding "glibc23-ctype-compat" line
at the end of debian/patches/00list, and install it on your local machine.
Note that we don't support this patch nowadays, please use it at your own
risk.
