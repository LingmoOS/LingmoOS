%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
%{!?gtk2_binary_version: %global gtk2_binary_version %(pkg-config  --variable=gtk_binary_version gtk+-2.0)}
%{!?gtk3_binary_version: %global gtk3_binary_version %(pkg-config  --variable=gtk_binary_version gtk+-3.0)}

Name:		deepin-fcitxconfigtool-plugin
Summary:		An input method framework
Version:		0.1.17
Release:		1%{?dist}
License:		GPLv2+
URL:			https://fcitx-im.org/wiki/Fcitx
Source0:		%{name}-%{version}.tar.xz
BuildRequires:		gcc-c++
BuildRequires:		pango-devel, dbus-devel
%if 0%{?rhel} < 8
%else
%ifarch aarch64 s390x
%else
BuildRequires:		enchant-devel
%endif
%endif

BuildRequires:		cmake
BuildRequires:		fcitx, fcitx-devel
BuildRequires:		fcitx-qt5-devel
BuildRequires:		dtkwidget-devel
BuildRequires:		dtkgui-devel dtkcore-devel
BuildRequires:		gtest-devel, gtk3-devel, gtk2-devel
BuildRequires:		qt5-qttools-devel
BuildRequires:		pkgconfig
BuildRequires:		gtest-devel
BuildRequires:		dde-control-center-devel
BuildRequires:		iso-codes, dbus-glib-devel
BuildRequires:	 	gmock-devel, unique-devel
BuildRequires:		dtkwidget-devel
BuildRequires:		dtkgui-devel dtkcore-devel
BuildRequires:		qt5-qtbase-devel
BuildRequires:		qt5-qtbase-gui
BuildRequires:		qt5-qtbase
BuildRequires:		dde-qt-dbus-factory-devel

%description
Fcitx is an input method framework with extension support. Currently it
supports Linux and Unix systems like FreeBSD.

Fcitx tries to provide a native feeling under all desktop as well as a light
weight core. You can easily customize it to fit your requirements.

%package data
Summary:		fcitx-config-tools and multilanguages translations
BuildArch:		noarch
Requires:	 	fcitx

%description data
The %{name} package provides fcitx-config-tools and multilanguages translations

%prep

%setup -q

%build
mkdir -p build
pushd build
%cmake .. -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_OFF" 
popd
make %{?_smp_mflags} -C build

%install
%make_install INSTALL="install -p" -C build

%post 

%postun  

%files 
%doc README.md
%license
%{_exec_prefix}/*
/lib/*

%exclude %{_sysconfdir}/*

