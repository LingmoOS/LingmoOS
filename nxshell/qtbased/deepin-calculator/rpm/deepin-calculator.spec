%define pkgrelease  1
%if 0%{?openeuler}
%define specrelease %{pkgrelease}
%else
## allow specrelease to have configurable %%{?dist} tag in other distribution
%define specrelease %{pkgrelease}%{?dist}
%endif

Name:           deepin-calculator
Version:        5.7.1.1
Release:        %{specrelease}
Summary:        An easy to use calculator for ordinary users
License:        GPLv3+
URL:            https://github.com/linuxdeepin/%{name}
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: cmake3
BuildRequires: qt5-linguist
BuildRequires: qt5-devel

BuildRequires: pkgconfig(dtkcore)
BuildRequires: pkgconfig(dtkwidget)
BuildRequires: pkgconfig(dtkgui)


# BuildRequires: pkgconfig(Qt5Widgets)
# BuildRequires: pkgconfig(Qt5Core)
# BuildRequires: pkgconfig(Qt5Gui)
# BuildRequires: pkgconfig(Qt5DBus)
# BuildRequires: pkgconfig(Qt5Xml)
# BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: gtest-devel
BuildRequires: gmock-devel

Requires: 	qt5-qtbase
Requires: 	qt5-qtbase-gui
Requires: 	qt5-qtsvg

Requires: 	dde-qt-dbus-factory
Requires: 	dtkcore
Requires: 	dtkgui
Requires: 	dtkwidget

%description
%{summary}.

%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
# cmake_minimum_required version is too high
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")
mkdir build
pushd build
%cmake ../ -DCMAKE_BUILD_TYPE=Release \-DAPP_VERSION=%{version} -DVERSION=%{version}

%make_build
popd

%install
# pushd build
# %make_install
# popd
%make_install -C build INSTALL_ROOT="%buildroot"

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/applications/%{name}.desktop
%{_datadir}/deepin-manual/manual-assets/application/deepin-calculator/calculator/*

%changelog
* Tue Jun 14 2022 zhangzhong <zhangzhong@uniontech.com> - 5.7.21-1
- init spec for euler

* Mon Apr 19 2021 zhangdingwen <zhangdingwen@uniontech.com> - 5.7.1.1-1
- init spec for euler

* Thu Aug 13 2020 guoqinglan <guoqinglan@uniontech.com> - 5.5.27-1
- Update to 5.5.27
