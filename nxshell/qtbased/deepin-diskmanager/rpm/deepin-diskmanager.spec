%global repo deepin-diskmanager

Name:           deepin-diskmanager
Version:        1.3.14
Release:        1%{?dist}
Summary:        Disk Utility Is A Disk Management Tool For Disk
License:        GPLv3+
URL:            https://github.com/linuxdeepin/deepin-diskmanager
Source0:        %{name}_%{version}.orig.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  qt5-qttools-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qttools-devel
BuildRequires:  dtkwidget-devel
BuildRequires:  qt5-qtbase-private-devel
BuildRequires:  parted-devel
BuildRequires:  dde-qt-dbus-factory-devel
BuildRequires:  polkit-qt5-1-devel
BuildRequires:  gtest-devel
BuildRequires:  gmock-devel
BuildRequires:  qt5-qtsvg-devel
Requires:       smartmontools
Requires:       mtools
Requires:       dosfstools
Requires:       ntfs-3g

%description
Disk Utility Is A Disk Management Tool For Disk.

%prep
%autosetup -p1 -n %{repo}-%{version}

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
%cmake -DCMAKE_BUILD_TYPE=Release
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%doc README.md
%license LICENSE
%{_bindir}/%{repo}
%{_datadir}/%{repo}/translations/*.qm
%{_datadir}/applications/%{repo}.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{repo}.svg
%{_datadir}/deepin-manual/manual-assets/application/deepin-diskmanager/*
/usr/lib/libbasestruct.a
/usr/lib/libddmlog.a
/usr/lib/deepin-daemon/deepin-diskmanager-service
/lib/udev/rules.d/99-diskmanager.rules
%{_libexecdir}/openconnect/USBadd.sh
%{_libexecdir}/openconnect/USBremove.sh
%{_datadir}/dbus-1/system-services/com.deepin.diskmanager.service
%{_datadir}/dbus-1/system.d/com.deepin.diskmanager.conf
%{_datadir}/polkit-1/actions/com.deepin.diskmanager.policy
%{_bindir}/deepin-diskmanager-authenticateProxy

%changelog
