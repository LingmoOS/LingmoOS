# $Id: cdk.spec,v 1.71 2023/02/02 01:15:18 tom Exp $
Summary:  Curses Development Kit
%define AppProgram cdk
%define AppVersion 5.0
%define AppRelease 20230201
Name:  %{AppProgram}
Version:  %{AppVersion}
Release:  %{AppRelease}
License:  MIT-X11
Group:  Development/Libraries
URL:  http://invisible-island.net/%{name}/
Source0:  %{name}-%{version}-%{release}.tgz
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  ncurses6

%description
Cdk stands for "Curses Development Kit".  It contains a large number of ready
to use widgets which facilitate the speedy development of full screen curses
programs.

%package devel
Summary:        Curses Development Kit
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description devel
Development headers for cdk (Curses Development Kit)

%prep
%global SCREEN ncursesw6
%define debug_package %{nil}
%setup -q -n %{name}-%{version}-%{release}

%build
%configure
make all
find . -name '*.o' -exec rm -f {} \;

%configure \
 --enable-warnings \
 --enable-stdnoreturn \
 --enable-const \
 --with-shared \
 --with-screen=%{SCREEN} \
 --with-versioned-syms
#make %{?_smp_mflags} cdkshlib
make all

%install
rm -rf $RPM_BUILD_ROOT
make install.libs install \
  DESTDIR=$RPM_BUILD_ROOT \
  DOCUMENT_DIR=$RPM_BUILD_ROOT%{_defaultdocdir}/%{name}
ls -l $RPM_BUILD_ROOT%{_libdir}
chmod +x $RPM_BUILD_ROOT%{_libdir}/*.so # fixes rpmlint unstripped-binary-or-object
install -m 644 libcdk.a $RPM_BUILD_ROOT%{_libdir}

# The "doc" tag installs the contents of the demos and examples directories,
# but the binaries are unwanted in the rpm.
echo "** cleanup demos-directory"
( cd demos && make clean )

echo "** cleanup examples-directory"
( cd examples && make clean )

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc CHANGES COPYING VERSION
%{_libdir}/*.so.*
%exclude %{_libdir}/*.a
%exclude %{_mandir}/man1/*
%exclude %{_mandir}/man3/*
%{_defaultdocdir}/%{name}/*

%files devel
%defattr(-,root,root,-)
%doc examples demos
%{_libdir}/*.a
%{_libdir}/*.so
%{_bindir}/cdk5-config
%{_mandir}/man1/*
%{_mandir}/man3/*
%{_includedir}/%{name}.h
%{_includedir}/%{name}
%exclude %{_defaultdocdir}/%{name}/*

%changelog

* Tue Oct 25 2022 Thomas E. Dickey
- use ncursesw6 test-package as in Debian, ensuring wide-characters.

* Thu Oct 20 2022 Thomas E. Dickey
- update license

* Tue Oct 18 2022 Thomas E. Dickey
- drop obsolete doc-files

* Wed Feb 20 2019 Thomas E. Dickey
- build-fixes for Fedora 28

* Wed Nov 05 2014 Thomas E. Dickey
- move manpages to -devel package

* Sun Nov 02 2014 Thomas E. Dickey
- change shared library configuration to use --with-shared option

* Tue Mar 20 2012 Thomas E. Dickey
- install cdk.h in normal location

* Fri May 13 2011 Thomas E. Dickey
- parameterize/adapt to building from unpatched sources as part of test builds

* Sun Jan 4 2009 Marek Mahut <mmahut@fedoraproject.org> - 5.0.20081105-1
- Initial build
