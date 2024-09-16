%global debug_package   %{nil}
Name:           dpa-ext-gnomekeyring
Version:        5.0.7
Release:        1
Summary:        GNOME keyring extension for dde-polkit-agent.
License:        GPLv3+
URL:            https://www.deepin.org
Source0:        %{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: qt5-devel
BuildRequires: libsecret-devel
BuildRequires: dde-polkit-agent-devel

%description
DDE Polkit Agent extension to automatically do some thing with GNOME keyring.

%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
# cmake_minimum_required version is too high
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")
mkdir build && pushd build
%cmake ../ -DCMAKE_BUILD_TYPE=Release -DAPP_VERSION=%{version} -DVERSION=%{version}

%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%files
%{_prefix}/lib/polkit-1-dde/plugins/libdpa-ext-gnomekeyring.so
%{_datadir}/%{name}/translations/*

%changelog
* Tue Jun 15 2021 panchenbo <panchenbo@uniontech.com> - 5.0.7-1
- Update to 5.0.7
