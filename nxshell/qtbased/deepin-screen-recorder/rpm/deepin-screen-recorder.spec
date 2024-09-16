%global soname screen_shot_recorder
Name:           deepin-screen-recorder
Version:        5.8.0
Release:        1
Summary:        Deepin Screen Recorder
License:        GPLv3+
URL:            https://github.com/linuxdeepin/deepin-screen-recorder
Source0:        %{name}_%{version}.orig.tar.xz

BuildRequires:  qt5-linguist
BuildRequires:  dtkwidget-devel
#BuildRequires:  pkgconfig(dtkwm)
#BuildRequires:  pkgconfig(libprocps)
BuildRequires:  procps-ng-devel
BuildRequires:  qt5-qtmultimedia-devel
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(dframeworkdbus)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xtst)
BuildRequires:  pkgconfig(xcb)
BuildRequires:  pkgconfig(xcb-util)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  gcc-c++
BuildRequires:   gcc
Requires:       byzanz
Requires:       ffmpeg
Requires:       hicolor-icon-theme
Requires:       dbus
Requires:       deepin-turbo

%description
%{summary}.

%prep
%setup -q

%build
export PATH=%{_qt5_bindir}:$PATH
%qmake_qt5 PREFIX=%{_prefix}
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}


%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/
%{_datadir}/dman/%{name}/

%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.svg
%{_datadir}/dbus-1/services/com.deepin.ScreenRecorder.service

%{_sysconfdir}/modprobe.d/deepin-screen-recorder.conf
%{_sysconfdir}/modules-load.d/deepin-screen-recorder.conf
#%{_datarootdir}/appdata/deepin-screen-recorder.appdata.xml
%{_datarootdir}/dbus-1/services/com.deepin.Screenshot.service

%changelog
* Mon Aug 26 2019 Zamir SUN <sztsian@gmail.com> - 5.0.0-1
- Update to 5.0.0

* Fri Aug 09 2019 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 2.7.6-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Mon Mar 04 2019 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 2.7.6-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Thu Nov 15 2018 Zamir SUN <sztsian@gmail.com> - 2.7.6-1
- Update to 2.7.6

* Fri Jul 27 2018 mosquito <sensor.wen@gmail.com> - 2.7.5-1
- Update to 2.7.5

* Fri Feb 16 2018 mosquito <sensor.wen@gmail.com> - 2.7.3-1
- Update to 2.7.3

* Mon Nov 27 2017 mosquito <sensor.wen@gmail.com> - 2.6.5.1-1
- Update to 2.6.5.1

* Mon Oct 23 2017 mosquito <sensor.wen@gmail.com> - 2.6.3-1
- Update to 2.6.3

* Tue Oct 17 2017 mosquito <sensor.wen@gmail.com> - 2.6.1-1
- Update to 2.6.1

* Mon Aug 21 2017 mosquito <sensor.wen@gmail.com> - 2.6-1
- Update to 2.6

* Thu Jul 20 2017 mosquito <sensor.wen@gmail.com> - 2.4-1.gitbacac81
- Update to 2.4

* Fri Jul 14 2017 mosquito <sensor.wen@gmail.com> - 2.3-1.git6184619
- Update to 2.3

* Fri May 19 2017 mosquito <sensor.wen@gmail.com> - 1.8-1.gitc4040d0 
- Update to 1.8

* Tue Mar  7 2017 mosquito <sensor.wen@gmail.com> - 1.3-1.git8e0a4b3
- Update to 1.3

* Sun Feb 26 2017 mosquito <sensor.wen@gmail.com> - 0.8-1.git9eda269
- Initial build
