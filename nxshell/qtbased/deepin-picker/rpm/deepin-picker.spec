%define pkgrelease  1
%if 0%{?openeuler}
%define specrelease %{pkgrelease}
%else
## allow specrelease to have configurable %%{?dist} tag in other distribution
%define specrelease %{pkgrelease}%{?dist}
%endif

Name:           deepin-picker
Version:        3.9.8
Release:        %{specrelease}
Summary:        A color picker tool for deepin
License:        GPLv3+
URL:            https://github.com/linuxdeepin/%{name}
Source0:        %{url}/archive/%{version}/%{name}_%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: qt5-devel
BuildRequires: libxcb-devel
BuildRequires: libXext-devel
BuildRequires: libX11-devel
BuildRequires: libXtst-devel

BuildRequires: pkgconfig(dtkwidget)
BuildRequires: pkgconfig(dtkgui)
BuildRequires: pkgconfig(libexif)
BuildRequires: pkgconfig(xcb-util)

%description
%{summary}.
 
%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
mkdir build && pushd build
%qmake_qt5 ../ 
%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"
 
%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/
%exclude %{_datadir}/applications/%{name}.desktop
%{_datadir}/dbus-1/services/com.deepin.Picker.service
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/%{name}/translations/*.qm

 
%changelog
* Thu Aug 13 2020 guoqinglan <guoqinglan@uniontech.com> - 5.8.0.19-1
- Update to 3.9.8
