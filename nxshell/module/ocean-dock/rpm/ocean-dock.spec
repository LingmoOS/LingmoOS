%global sname lingmo-dock

Name:           ocean-dock
Version:        5.4.9
Release:        1
Summary:        Lingmo desktop-environment - Dock module
License:        GPLv3
URL:            http://shuttle.corp.lingmo.com/cache/repos/eagle/release-candidate/RERFNS4wLjAuNjU3NQ/pool/main/d/ocean-dock/
Source0:        %{name}-%{version}.orig.tar.xz	

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(dbusmenu-qt5)
BuildRequires:  pkgconfig(ocean-network-utils)
BuildRequires:  dtkwidget-devel >= 5.1
BuildRequires:  dtkcore-devel >= 5.1
BuildRequires:  pkgconfig(dframeworkdbus) >= 2.0
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  pkgconfig(gtk+-2.0)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xtst)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xcb-composite)
BuildRequires:  pkgconfig(xcb-ewmh)
BuildRequires:  pkgconfig(xcb-icccm)
BuildRequires:  pkgconfig(xcb-image)
BuildRequires:  qt5-linguist
BuildRequires:  gtest-devel
BuildRequires:  gmock-devel
Requires:       dbusmenu-qt5
Requires:       ocean-network-utils
Requires:       ocean-qt-dbus-factory
Requires:       xcb-util-wm
Requires:       xcb-util-image
Requires:       libxcb

%description
Lingmo desktop-environment - Dock module.

%package devel
Summary:        Development package for %{sname}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files and libraries for %{sname}.

%package onboard-plugin
Summary:        lingmo desktop-environment - dock plugin
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description onboard-plugin
lingmo desktop-environment - dock plugin.

%prep
%setup -q -n %{name}-%{version}
sed -i '/TARGETS/s|lib|%{_lib}|' plugins/*/CMakeLists.txt \
                                 plugins/plugin-guide/plugins-developer-guide.md

sed -i -E '30,39d' CMakeLists.txt

sed -i 's|/lib|/%{_lib}|' frame/controller/dockpluginscontroller.cpp \
                          frame/panel/mainpanelcontrol.cpp \
                          plugins/tray/system-trays/systemtrayscontroller.cpp


sed -i 's|/lib|/libexec|g' plugins/show-desktop/showdesktopplugin.cpp

sed -i 's|/usr/lib/ocean-dock/plugins|%{_libdir}/ocean-dock/plugins|' plugins/plugin-guide/plugins-developer-guide.md
sed -i 's|local/lib/ocean-dock/plugins|local/%{_lib}/ocean-dock/plugins|' plugins/plugin-guide/plugins-developer-guide.md

%build
export PATH=%{_qt5_bindir}:$PATH
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DARCHITECTURE=%{_arch} .
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%ldconfig_scriptlets

%files
%license LICENSE
%{_bindir}/%{name}
%{_libdir}/%{name}/
%{_datarootdir}/glib-2.0/schemas/com.lingmo.ocean.dock.module.gschema.xml
%{_datarootdir}/polkit-1/actions/org.lingmo.ocean.dock.overlay.policy

%files devel
%{_includedir}/%{name}/
%{_libdir}/pkgconfig/%{name}.pc
%{_libdir}/cmake/DdeDock/DdeDockConfig.cmake

%files onboard-plugin
%{_libdir}/ocean-dock/plugins/libonboard.so


%changelog
* Thu Mar 23 2021 uoser <uoser@uniontech.com> - 5.4.9-1
- Update to 5.4.9

