%define debug_package %{nil}
Name:           deepin-log-viewer
Version:        6.0.2
Release:        1%{?dist}.01.zs
Summary:        Log Viewer is a useful tool for viewing system logs
License:        GPLv3+
URL:            https://github.com/linuxdeepin/%{name}
Source0:        %{name}-%{version}.tar.gz

BuildRequires: qt5-devel
BuildRequires: gcc-c++
BuildRequires: cmake3
BuildRequires: dtkwidget-devel
BuildRequires: dtkgui-devel
BuildRequires: systemd-devel
BuildRequires: libicu-devel
BuildRequires: qt5-rpm-macros
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qttools-devel
BuildRequires: pkgconfig
BuildRequires: gtest-devel
BuildRequires: gmock-devel
BuildRequires: kf5-kcodecs-devel
BuildRequires: libzip-devel
BuildRequires: xerces-c-devel
BuildRequires: boost-devel
BuildRequires: minizip-devel
BuildRequires: rapidjson-devel
BuildRequires: libtool-ltdl-devel
BuildRequires: dtkcommon-devel
BuildRequires: qt5-qtbase-private-devel
BuildRequires: qt5-qtsvg-devel
BuildRequires: zlib-devel

%description
Log Viewer is a useful tool for viewing system logs.

%package logviewerplugin
Summary: Log Viewer Plugin library
%description logviewerplugin
Deepin Log Viewer Plugin library.

%package logviewerplugin-devel
Summary: Log Viewer Plugin library development headers
%description logviewerplugin-devel
Deepin Log Viewer Plugin library development headers.

%prep
%if 0%{?openeuler}
%setup -q
%else
%autosetup -p1
%endif

%build
export PATH=%{_qt5_bindir}:$PATH
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")
mkdir build && pushd build 
%cmake -DCMAKE_BUILD_TYPE=Release ../  -DAPP_VERSION=%{version} -DVERSION=%{version} 
%make_build  
popd

%install
%define _unpackaged_files_terminate_build 0
%make_install -C build INSTALL_ROOT="%buildroot"

%files
%doc README.md
%license LICENSE.txt
%{_bindir}/%{name}
%{_bindir}/logViewerAuth
%{_bindir}/logViewerTruncate
%{_datadir}/applications/%{name}.desktop
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/polkit-1/actions/*.policy
%{_datadir}/deepin-manual/manual-assets/application/deepin-log-viewer/log-viewer/*
%{_prefix}/lib/deepin-daemon/log-view-service
%{_datadir}/dbus-1/system-services/com.deepin.logviewer.service
%{_datadir}/dbus-1/system.d/com.deepin.logviewer.conf
%{_datadir}/%{name}/DocxTemplate/*.dfw
%{_datadir}/dsg/configs/org.deepin.deepin-log-viewer/*.json
%{_datadir}/glib-2.0/schemas/*.gschema.xml

%files logviewerplugin
%{_libdir}/liblogviewerplugin.so.*

%files logviewerplugin-devel
%{_includedir}/liblogviewerplugin
%{_libdir}/liblogviewerplugin.so
%{_libdir}/pkgconfig/liblogviewerplugin.pc

%changelog
* Mon Jun 13 2022 uoser <uoser@uniontech.com> - 6.0.2-1.01
- update: update to 6.0.2-1.01

