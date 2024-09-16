%define specrelease 2%{?dist}
%if 0%{?openeuler}
%define specrelease 2
%endif

Name:           dde-device-formatter
Version:        0.0.1.8
Release:        %{specrelease}
Summary:        A simple graphical interface for creating file system in a block device
License:        GPLv3+
URL:            https://github.com/linuxdeepin/dde-device-formatter
Source0:        %{name}-%{version}.tar.gz

BuildRequires: qt5-devel
BuildRequires: dtkwidget-devel
BuildRequires: dtkgui-devel
BuildRequires: udisks2-qt5-devel
BuildRequires: deepin-gettext-tools

%description
%{summary}.

%prep
%autosetup

%build
export PATH=%{_qt5_bindir}:$PATH
mkdir build && pushd build
%qmake_qt5 ../ VERSION=%{version} LIB_INSTALL_DIR=%{_libdir} DEFINES+="VERSION=%{version}"
%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"


%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/*

%changelog