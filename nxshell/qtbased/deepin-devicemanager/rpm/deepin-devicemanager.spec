%define specrelease 1%{?dist}
%if 0%{?openeuler}
%define specrelease 1
%endif

Name:           deepin-devicemanager
Version:        5.8.5.40
Release:        %{specrelease}
Summary:        Device Manager is a handy tool for viewing hardware information and managing the devices
License:        GPLv3+
URL:            https://github.com/linuxdeepin/deepin-devicemanager
Source0:        %{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: cmake3
BuildRequires: dtkwidget-devel
BuildRequires: dtkgui-devel
BuildRequires: systemd-devel
BuildRequires: polkit-qt5-1-devel
BuildRequires: libicu-devel
BuildRequires: qt5-rpm-macros
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qttools-devel
BuildRequires: cups-devel
BuildRequires: pkgconfig(dframeworkdbus)
BuildRequires: zeromq3-devel
BuildRequires: gtest-devel

Requires: smartmontools
Requires: dmidecode
Requires: xorg-x11-server-utils
Requires: hwinfo
Requires: cups
Requires: upower
Requires: deepin-shortcut-viewer
Requires: lshw
Requires: util-linux

%description
%{summary}.

%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")

mkdir build && pushd build
%cmake ../ -DCMAKE_BUILD_TYPE=Release -DAPP_VERSION=%{version} -DDISABLE_DRIVER=true -DVERSION=%{version}
%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%post
systemctl enable deepin-devicemanager-server.service


%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
/usr/share/dbus-1/services/com.deepin.Devicemanager.service
/usr/share/polkit-1/actions/com.deepin.deepin-devicemanager.policy
/lib/systemd/system/deepin-devicemanager-server.service
%{_bindir}/deepin-devicemanager-server
%{_datadir}/deepin-manual/manual-assets/application/deepin-devicemanager/device-manager/*
/etc/dbus-1/system.d/com.deepin.devicemanager.conf

%changelog
* Thu Apr 15 2021 zhangdingwen <zhangdingwen@uniontech.com> - 5.6.0.40-1
- Initial release for OpenEuler

* Mon Oct 12 2020 guoqinglan <guoqinglan@uniontech.com> - 5.5.9.36-1
- bugfix#48018

* Sat Oct 10 2020 guoqinglan <guoqinglan@uniontech.com> - 5.5.9.35-1
- bugfix#48626

* Tue Sep 29 2020 guoqinglan <guoqinglan@uniontech.com> - 5.5.9.34-1
- update to 5.5.9.34

* Thu Sep 24 2020 guoqinglan <guoqinglan@uniontech.com> - 5.5.9.33-1
- update to 5.5.9.33

* Wed Sep 23 2020 guoqinglan <guoqinglan@uniontech.com> - 5.5.9.31-1
- update to 5.5.9.31
