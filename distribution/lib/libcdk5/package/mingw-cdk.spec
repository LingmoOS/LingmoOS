%?mingw_package_header

# $Id: mingw-cdk.spec,v 1.40 2023/02/02 01:15:18 tom Exp $
Summary:	Curses Development Kit
%define AppProgram mingw32-cdk
%define AppVersion 5.0
%define AppRelease 20230201
Name:  %{AppProgram}
Version:  %{AppVersion}
Release:  %{AppRelease}
License:  MIT-X11
Group: Development/Libraries
Source: cdk-%{version}-%{release}.tgz
URL: http://invisible-island.net/cdk/

BuildRequires:  mingw32-filesystem >= 95
BuildRequires:  mingw32-gcc
BuildRequires:  mingw32-binutils
BuildRequires:  mingw32-ncurses6

BuildRequires:  mingw64-filesystem >= 95
BuildRequires:  mingw64-gcc
BuildRequires:  mingw64-binutils
BuildRequires:  mingw64-ncurses6

%define CC_NORMAL -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wconversion
%define CC_STRICT %{CC_NORMAL} -W -Wbad-function-cast -Wcast-align -Wcast-qual -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wwrite-strings -ansi -pedantic

%description -n mingw32-cdk
Cdk stands for "Curses Development Kit".  It contains a large number of ready
to use widgets which facilitate the speedy development of full screen curses
programs.

%package -n mingw64-cdk
Summary:        Curses library for MinGW64

%description -n mingw64-cdk
Cdk stands for "Curses Development Kit".  It contains a large number of ready
to use widgets which facilitate the speedy development of full screen curses
programs.

%prep
%define debug_package %{nil}

%define CFG_OPTS \\\
	--disable-echo \\\
	--verbose \\\
	--with-shared

%define debug_package %{nil}
%setup -q -n cdk-%{version}-%{release}

%build
mkdir BUILD-W32
pushd BUILD-W32
CFLAGS="%{CC_NORMAL}" \
CC=%{mingw32_cc} \
%mingw32_configure %{CFG_OPTS}
make
popd

mkdir BUILD-W64
pushd BUILD-W64
CFLAGS="%{CC_NORMAL}" \
CC=%{mingw64_cc} \
%mingw64_configure %{CFG_OPTS}
make
popd

%install
rm -rf $RPM_BUILD_ROOT

pushd BUILD-W32
%{mingw32_make} installCDKLibrary installImportLibrary installCDKHeaderFiles DESTDIR=$RPM_BUILD_ROOT
popd

pushd BUILD-W64
%{mingw64_make} installCDKLibrary installImportLibrary installCDKHeaderFiles DESTDIR=$RPM_BUILD_ROOT
popd

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)

%files -n mingw32-cdk
%{mingw32_bindir}/*
%{mingw32_includedir}/*
%{mingw32_libdir}/*

%files -n mingw64-cdk
%{mingw64_bindir}/*
%{mingw64_includedir}/*
%{mingw64_libdir}/*

%changelog

* Sat Nov 02 2013 Thomas E. Dickey
- initial version
