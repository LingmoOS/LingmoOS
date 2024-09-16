%define pkgrelease  1
%if 0%{?openeuler}
%define specrelease %{pkgrelease}
%else
## allow specrelease to have configurable %%{?dist} tag in other distribution
%define specrelease %{pkgrelease}%{?dist}
%endif

Name:           dde-calendar
Version:        5.8.1.1

Release:        %{specrelease}
Summary:        Calendar is a smart daily planner to schedule all things in life
License:        GPLv3
URL:            https://github.com/linuxdeepin/dde-calendar
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: qt5-devel
BuildRequires: dtkgui-devel
BuildRequires: dtkwidget-devel
BuildRequires: deepin-gettext-tools
BuildRequires: gtest-devel

%description
%{summary}.

%prep
%autosetup

%build
export PATH=%{_qt5_bindir}:$PATH
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")
mkdir build && pushd build 
%cmake -DCMAKE_BUILD_TYPE=Release ../  -DAPP_VERSION=%{version} -DVERSION=%{version} 
%make_build  
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/deepin-manual/manual-assets/application/dde-calendar/calendar/*
%{_datadir}/dbus-1/services/*.service
/etc/xdg/autostart/dde-calendar-service.desktop
%{_datadir}/dde-calendar/data/*
/usr/lib/deepin-daemon/dde-calendar-service

%changelog
* Tue Apr 20 2021 zhangdingwen <zhangdingwen@uniontech.com> - 5.8.1.1-1
- init spec for euler
