%global repo qt5integration

Name:           deepin-qt5integration
Version:        5.1.5
Release:        1%{?dist}
Summary:        Qt platform theme integration plugins for DDE
# The entire source code is GPLv3+ except styles/ which is BSD,
# styleplugins/dstyleplugin/dstyleanimation* which is LGPL
License:        GPLv3+ and BSD and LGPLv2+
URL:            https://github.com/linuxdeepin/qt5integration
Source0:        %{url}/archive/%{version}/%{repo}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(atk)
BuildRequires:  pkgconfig(dtkcore)
BuildRequires:  pkgconfig(dtkwidget) >= 2.0.6
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(gtk+-2.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gdk-pixbuf-2.0)
BuildRequires:  pkgconfig(ice)
BuildRequires:  pkgconfig(libinput)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(pango)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5OpenGL)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Xdg) >= 3.0.0
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xrender)
BuildRequires:  pkgconfig(xcb)
BuildRequires:  pkgconfig(mtdev)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5MultimediaWidgets)
BuildRequires:  qt5-qtbase-common
# for libQt5ThemeSupport.a
BuildRequires:  qt5-qtbase-static
BuildRequires:  qt5-qtbase-private-devel
BuildRequires:  make
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
Requires:       deepin-qt5dxcb-plugin%{?_isa}

%description
Multiple Qt plugins to provide better Qt5 integration for DDE is included.

%prep
%autosetup -p1 -n %{repo}-%{version}

%build
%qmake_qt5 PREFIX=%{_prefix} BASED_DTK_DIR=based-dtk
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%doc README.md
%license LICENSE
# chooser
%{_qt5_plugindir}/platformthemes/libqdeepin.so
%{_qt5_plugindir}/iconengines/libdsvgicon.so
%{_qt5_plugindir}/imageformats/libdsvg.so
%{_qt5_plugindir}/iconengines/libdtkbuiltin.so
%{_qt5_plugindir}/styles/libchameleon.so
# target
%{_qt5_plugindir}/platformthemes/based-dtk/libqdeepin.so
%{_qt5_plugindir}/iconengines/based-dtk/libdsvgicon.so
%{_qt5_plugindir}/imageformats/based-dtk/libdsvg.so
%{_qt5_plugindir}/iconengines/based-dtk/libdtkbuiltin.so
%{_qt5_plugindir}/styles/based-dtk/libchameleon.so

%changelog
