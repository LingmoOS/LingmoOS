Name:           deepin-screensaver
Version:        5.0.6
Release:        1
Summary:        Screensaver Tool
License:        GPLv3+
Url:            https://github.com/linuxdeepin/%{name}
Source0:        %{name}_%{version}.orig.tar.xz

BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  pkgconfig(xscrnsaver)

Requires:       %{name}-data = %{version}-%{release}

%description
Deepin screensaver viewer and tools.

%package data
Summary:        Screensaver data
BuildArch:      noarch
Requires:       xscreensaver-extras
Requires:       xscreensaver-gl-extras

%description data
Extra data for Deepin Screensaver.

%prep
%setup -q
sed -i 's|/lib|/libexec|' xscreensaver/xscreensaver.pro common.pri
sed -i 's|/usr/lib|%{_libexecdir}|' tools/preview/main.cpp

%build
%qmake_qt5 PREFIX=%{_prefix}
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%doc CHANGELOG.md
%{_bindir}/%{name}
%{_datadir}/dbus-1/services/*
%{_datadir}/dbus-1/interfaces/*

%files data
%{_libexecdir}/%{name}


%changelog
* Thu Feb 28 2019 Robin Lee <cheeselee@fedoraproject.org> - 0.0.7-1
- Initial packaging
