%global sname ocean-launcher

Name:           ocean-launcher
Version:        5.4.8
Release:        1
Summary:        ocean desktop-environment - Launcher module
License:        GPLv3
URL:            http://shuttle.corp.lingmo.com/cache/repos/eagle/release-candidate/RERFNS4wLjAuNjU3NQ/pool/main/d/ocean-launcher/
Source0:        %{name}-%{version}.orig.tar.xz

BuildRequires:  cmake
BuildRequires:  cmake(Qt5LinguistTools)
BuildRequires:  dtkwidget-devel
BuildRequires:  pkgconfig(dtkcore)
BuildRequires:  pkgconfig(dframeworkdbus)
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  pkgconfig(xcb-ewmh)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  qt5-qtbase-private-devel
BuildRequires:  gtest-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
Requires:       lingmo-menu
Requires:       ocean-daemon
Requires:       startocean
Requires:       hicolor-icon-theme

%description
%{summary}.

%package devel
Summary:        Development package for %{sname}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files and libraries for %{sname}.

%prep
%setup -q -n %{name}-%{version}
sed -i 's|lrelease|lrelease-qt5|' translate_generation.sh

%build
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DWITHOUT_UNINSTALL_APP=1 .
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/
%{_bindir}/ocean-launcher-wapper
%{_datadir}/applications/ocean-launcher.desktop
%{_datadir}/dbus-1/services/*.service
%{_datadir}/icons/hicolor/scalable/apps/lingmo-launcher.svg
%{_datadir}/glib-2.0/schemas/com.lingmo.ocean.launcher.gschema.xml

%files devel
%{_includedir}/%{name}/

%changelog
* Tue Apr 6 2021 uoser <uoser@uniontech.com> - 5.4.8-1
- Update to 5.4.8
