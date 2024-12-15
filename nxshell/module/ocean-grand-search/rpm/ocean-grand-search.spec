%global repo ocean-grand-search

Name:           ocean-grand-search
Version:        5.1.1
Release:        1%{?fedora:%dist}
Summary:        Lingmo desktop-environment - Search module
License:        GPLv3
URL:            https://aptly.uniontech.com/pkg/eagle-1050/release-candidate/5LiT5Lia54mIMTA1MHdheWxhbmTpgILphY3kuLvpopgyMDIxLTA5LTE3IDEzOjQzOjMz/pool/main/d/ocean-grand-search/
Source0:        %{name}_%{version}.orig.tar.xz

BuildRequires:  dtkcore-devel >= 5.1
BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(dframeworkdbus) >= 2.0
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  qt5-qtbase-devel
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  dtkwidget-devel >= 5.1
BuildRequires:  dtkgui-devel >= 5.2.2.16
BuildRequires:  pkgconfig(taglib)
BuildRequires:  ocean-dock-devel
BuildRequires:  pkgconfig(poppler-qt5)
BuildRequires:  ffmpeg-devel
BuildRequires:  ffmpegthumbnailer-devel
BuildRequires:  libicu-devel
BuildRequires:  lingmo-anything-devel

Requires:  startocean
Requires:  dtkwidget
Requires:  taglib
Requires:  poppler-qt5
Requires:  ffmpeg-libs
Requires:  ffmpegthumbnailer

%description
Lingmo desktop-environment - Search module.

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files for %{name}.

%prep
%autosetup 
sed -i 's|lib/${CMAKE_LIBRARY_ARCHITECTURE}|%{_lib}|' CMakeLists.txt
sed -i 's|lib/|%{_lib}/|' src/grand-search-dock-plugin/CMakeLists.txt

%build
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DARCHITECTURE=%{_arch} .
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%license LICENSE
%{_bindir}/%{name}
%{_bindir}/%{name}-daemon
%{_libdir}/ocean-grand-search-daemon/lib%{name}-daemon.so
%{_libdir}/ocean-dock/plugins/liboceangrandsearch_dockplugin.so
%{_libdir}/ocean-grand-search-daemon/plugins/searcher/.readme
%{_libdir}/ocean-grand-search/plugins/preview
%{_datadir}/dbus-1/interfaces/com.lingmo.ocean.GrandSearch.xml
%{_datadir}/dbus-1/services/com.lingmo.ocean.GrandSearch.service
%{_datadir}/dbus-1/services/com.lingmo.ocean.daemon.GrandSearch.service
%{_datadir}/ocean-grand-search/translations
%{_datadir}/glib-2.0/schemas/com.lingmo.ocean.dock.module.grand-search.gschema.xml
%{_datadir}/applications/ocean-grand-search-daemon.desktop

%changelog
