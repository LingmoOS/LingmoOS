# Run tests in check section
# disable for bootstrapping

%bcond_with check

%global with_debug 1
%if 0%{?with_debug}
%global debug_package   %{nil}
%endif

Name:           ocean-clipboard
Version:        5.3.15
Release:        1
Summary:        Qt platform theme integration plugins for OCEAN
License:        GPLv3+
URL:            http://shuttle.corp.lingmo.com/cache/repos/eagle/release-candidate/RERFNS4wLjAuNjU3NQ/pool/main/d/ocean-clipboard/
Source0:        %{name}_%{version}.orig.tar.xz


BuildRequires:  cmake
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-linguist
BuildRequires:  dtkwidget-devel
BuildRequires:  dtkwidget
BuildRequires:  libgio-qt
BuildRequires:  libgio-qt-devel
BuildRequires:  pkgconfig(dframeworkdbus) >= 2.0
BuildRequires:  gtest-devel

%description
Qt platform theme integration plugins for OCEAN
 Multiple Qt plugins to provide better Qt5 integration for OCEAN is included.


%prep
%autosetup

%build
mkdir build && cd build
export PATH=%{_qt5_bindir}:$PATH
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DARCHITECTURE=%{_arch}
%{__make}

%install
pushd %{_builddir}/%{name}-%{version}/build
%make_install INSTALL_ROOT=%{buildroot}
popd

%files
/etc/xdg/autostart/ocean-clipboard.desktop
%{_bindir}/%{name}
%{_bindir}/%{name}loader
%{_datadir}/dbus-1/services/org.lingmo.ocean.Clipboard1.service
%{_datadir}/%{name}/translations/
%{_datadir}/applications/ocean-clipboard.desktop


%changelog
* Wed Mar 10 2021 uoser <uoser@uniontech.com> - 5.3.15
- Update to 5.3.15

