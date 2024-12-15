Name:           ocean-session-shell
Version:        5.4.20.4
Release:        1
Summary:        lingmo-session-shell - Lingmo desktop-environment - session-shell module
License:        GPLv3+
URL:            http://shuttle.corp.lingmo.com/cache/repos/eagle/release-candidate/RERFNS4wLjAuNzYxMA/pool/main/d/ocean-session-shell/
Source0:        %{name}-%{version}.orig.tar.xz

BuildRequires:  cmake
BuildRequires:  dtkcore >= 5.1
BuildRequires:  gsettings-qt
BuildRequires:  qt5-linguist
BuildRequires:  dtkwidget-devel >= 5.1
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  qt5-qtmultimedia-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  libXcursor-devel
BuildRequires:  libXrandr-devel
BuildRequires:  libXtst-devel
BuildRequires:  libXi-devel
BuildRequires:  xcb-util-wm xcb-util-wm-devel
BuildRequires:  ocean-qt-dbus-factory-devel
BuildRequires:  gsettings-qt-devel
BuildRequires:  lightdm-qt5-devel
BuildRequires:  pam-devel
BuildRequires:  gtest-devel
Requires:       lightdm
Requires(post): sed
Provides:       lightdm-lingmo-greeter = %{version}-%{release}
Provides:       lightdm-greeter = 1.2

%description
lingmo-session-shell - Lingmo desktop-environment - session-shell module.

%prep
%setup -q -n %{name}-%{version}

%build
export PATH=$PATH:%{_qt5_bindir}
cmake_version=$(cmake --version | head -1 | awk '{print $3}')
sed -i "s|VERSION 3.13.4|VERSION $cmake_version|g" CMakeLists.txt
%cmake
%make_build

%install
%make_install

%files
%{_bindir}/ocean-lock
%{_bindir}/lightdm-lingmo-greeter
%attr(755,root,root) %{_bindir}/lingmo-greeter
%{_sysconfdir}/lingmo/greeters.d/00-xrandr
%{_sysconfdir}/lingmo/greeters.d/lightdm-lingmo-greeter
%{_datadir}/ocean-session-shell/

%{_sysconfdir}/lingmo/greeters.d/10-cursor-theme
%{_datadir}/xgreeters/lightdm-lingmo-greeter.desktop
%{_datadir}/dbus-1/services/com.lingmo.ocean.lockFront.service
%{_datadir}/dbus-1/services/com.lingmo.ocean.shutdownFront.service
%{_datadir}/applications/ocean-lock.desktop
%{_datadir}/glib-2.0/schemas/com.lingmo.ocean.session-shell.gschema.xml

%changelog
* Thu Jun 11 2020 uoser <uoser@uniontech.com> - 5.4.20.4-1
- Update to 5.4.20.4
