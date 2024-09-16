%global repo dde-polkit-agent

%define pkgrelease  1
%if 0%{?openeuler}
%define specrelease %{pkgrelease}
%else
## allow specrelease to have configurable %%{?dist} tag in other distribution
%define specrelease %{pkgrelease}%{?dist}
%endif

Name:           dde-polkit-agent
Version:        5.4.5
Release:        %{specrelease}
Summary:        Deepin Polkit Agent
License:        GPLv3
URL:            https://github.com/linuxdeepin/dde-polkit-agent
Source0:        %{name}-%{version}.orig.tar.xz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  dtkwidget-devel >= 5.1.1
BuildRequires:  dde-qt-dbus-factory-devel
BuildRequires:  dde-qt-dbus-factory
BuildRequires:  pkgconfig(polkit-qt5-1)
BuildRequires:  qt5-devel
BuildRequires:  qt5-linguist
BuildRequires:  gsettings-qt-devel
BuildRequires:  qt5-qtmultimedia-devel
BuildRequires:  qt5-qtx11extras-devel

%description
DDE Polkit Agent is the polkit agent used in Deepin Desktop Environment.

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files and libraries for %{name}.

%prep
%setup -q -n %{repo}-%{version}
sed -i 's|lrelease|lrelease-qt5|' translate_generation.sh

%build
export PATH=%{_qt5_bindir}:$PATH
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DARCHITECTURE=%{_arch} .
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%doc README.md
%license LICENSE
%{_prefix}/lib/polkit-1-dde/dde-polkit-agent
%{_datadir}/%{repo}/

%files devel
%{_includedir}/dpa/agent-extension-proxy.h
%{_includedir}/dpa/agent-extension.h

%changelog
* Thu Apr 26 2021 uniontech <uoser@uniontech.com> - 5.4.5-1
- Update to 5.4.5
