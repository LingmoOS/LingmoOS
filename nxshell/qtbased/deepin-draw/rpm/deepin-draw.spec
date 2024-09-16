%global debug_package   %{nil}
%define pkgrelease  1
%if 0%{?openeuler}
%define specrelease %{pkgrelease}
%else
## allow specrelease to have configurable %%{?dist} tag in other distribution
%define specrelease %{pkgrelease}%{?dist}
%endif

Name:           deepin-draw
Version:        5.9.2.0
Release:        %{specrelease}
Summary:        A lightweight drawing tool for Linux Deepin
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-draw
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(dtkcore)
BuildRequires:  pkgconfig(dtkwidget)
BuildRequires:  pkgconfig(xcb-aux)
BuildRequires:  pkgconfig(Qt5)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  qt5-linguist
BuildRequires:  desktop-file-utils
#Requires:       deepin-notifications
#Requires:       deepin-qt5integration

%description
A lightweight drawing tool for Linux Deepin.

%prep
%autosetup -p1

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
mkdir build && pushd build
%cmake -DCMAKE_BUILD_TYPE=Release -DAPP_VERSION=%{version} -DVERSION=%{version}  ../
%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/
%{_datadir}/deepin-manual/manual-assets/application/%{name}/
%{_datadir}/applications/%{name}.desktop
%{_datadir}/dbus-1/services/com.deepin.Draw.service
%{_datadir}/icons/deepin/apps/scalable/%{name}.svg
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/application/x-ddf.xml
%{_datadir}/mime/packages/deepin-draw.xml

%changelog
* Wed Mar 17 2021 uoser <uoser@uniontech.com> - 5.9.2.0-1
- Update to 5.9.2.0
