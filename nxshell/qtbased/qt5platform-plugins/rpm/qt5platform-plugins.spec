%define specrelease 12%{?dist}
%if 0%{?openeuler}
%define specrelease 1
%endif

Name:           dde-qt5platform-plugins
Version:        5.0.25.1
Release:        %{specrelease}
Summary:        Qt platform plugins for DDE
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-desktop-schemas
Source0:        %{name}-%{version}.tar.gz


%description
%{summary}.

%package -n     dde-qt5xcb-plugin
Summary:        %{summary}
BuildRequires:  qt5-devel
BuildRequires:  git
BuildRequires:  qt5-qtwayland-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  wayland-devel
#BuildRequires:  dde-waylandserver-devel
#BuildRequires:  dde-waylandclient-devel
BuildRequires:  xcb-util-image-devel
BuildRequires:  xcb-util-renderutil-devel
BuildRequires:  libxcb-devel
BuildRequires:  xcb-util-wm-devel
BuildRequires:  mtdev-devel
BuildRequires:  libxkbcommon-x11-devel
BuildRequires:  dbus-devel
BuildRequires:  systemd-devel
BuildRequires:  libXrender-devel
BuildRequires:  libXi-devel
BuildRequires:  libSM-devel
BuildRequires:  libxcb-devel
BuildRequires:  fontconfig-devel
BuildRequires:  freetype-devel
BuildRequires:  libxcb-devel
BuildRequires:  cairo-devel
BuildRequires:  kf5-kwayland-devel
BuildRequires:  libqtxdg-devel
BuildRequires:  dtkwidget-devel
BuildRequires:  pkg-config
BuildRequires:  mtdev-devel
BuildRequires:  xcb-util-keysyms-devel



Provides:       qt5dxcb-plugin
Obsoletes:      qt5dxcb-plugin
%description -n dde-qt5xcb-plugin
%{summary}.

%package -n     dde-qt5wayland-plugin
Summary:        %{summary}
BuildRequires:  qt5-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtbase-static
BuildRequires:  libqtxdg-devel
BuildRequires:  dtkwidget-devel
BuildRequires:  pkg-config
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  mtdev-devel
BuildRequires:  qt5-qtmultimedia-devel
%description -n dde-qt5wayland-plugin
%{summary}.


%prep
%autosetup
sed -i 's|wayland/wayland.pro|#wayland/wayland.pro|' qt5platform-plugins.pro

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
mkdir build && pushd build
%qmake_qt5 ../
%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%files -n      dde-qt5xcb-plugin
%{_libdir}/qt5/plugins/platforms/libdxcb.so


%files -n      dde-qt5wayland-plugin
#%{_libdir}/qt5/plugins/platforms/libdwayland.so
#%{_libdir}/qt5/plugins/wayland-shell-integration/libkwayland-shell.so

%changelog
* Tue Apr 21 2021 uoser <uoser@uniontech.com> - 5.0.25.1-1
- update to 5.0.25.1-1