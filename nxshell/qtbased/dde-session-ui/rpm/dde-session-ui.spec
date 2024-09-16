Name:           dde-session-ui
Version:        5.4.8
Release:        1
Summary:        Deepin desktop-environment - Session UI module
License:        GPLv3
URL:            https://github.com/linuxdeepin/%{name}
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  deepin-gettext-tools
BuildRequires:  pkgconfig(dtkwidget) >= 5.1
BuildRequires:  pkgconfig(dframeworkdbus)
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  pkgconfig(gtk+-2.0)
BuildRequires:  pkgconfig(libsystemd)
BuildRequires:  pkgconfig(xcb-ewmh)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xtst)
BuildRequires:  pkgconfig(xext)
BuildRequires:  gocode
BuildRequires:  qt5-devel
BuildRequires:  dtkcore-devel >= 5.1
BuildRequires:  dde-dock-devel
Requires:       dde-daemon
Requires:       startdde

Requires:       lightdm
Requires(post): sed
Provides:       lightdm-deepin-greeter = %{version}-%{release}
Provides:       lightdm-greeter = 1.2
Provides:       deepin-notifications = %{version}-%{release}
Obsoletes:      deepin-notifications < %{version}-%{release}

%description
This project include those sub-project:

- dde-shutdown: User interface of shutdown.
- dde-lock: User interface of lock screen.
- dde-lockservice: The back-end service of locking screen.
- lightdm-deepin-greeter: The user interface when you login in.
- dde-switchtogreeter: The tools to switch the user to login in.
- dde-lowpower: The user interface of reminding low power.
- dde-osd: User interface of on-screen display.
- dde-hotzone: User interface of setting hot zone.

%prep
%setup -q -n %{name}-%{version}
sed -i 's|default_background.jpg|default.png|' widgets/fullscreenbackground.cpp
sed -i 's|lib|libexec|' \
    misc/applications/deepin-toggle-desktop.desktop* \
    dde-osd/dde-osd_autostart.desktop \
    dde-osd/com.deepin.dde.osd.service \
    dde-osd/notification/files/com.deepin.dde.*.service* \
    dde-osd/dde-osd.pro \
    dde-welcome/com.deepin.dde.welcome.service \
    dde-welcome/dde-welcome.pro \
    dde-bluetooth-dialog/dde-bluetooth-dialog.pro \
    dde-touchscreen-dialog/dde-touchscreen-dialog.pro \
    dde-warning-dialog/com.deepin.dde.WarningDialog.service \
    dde-warning-dialog/dde-warning-dialog.pro \
    dde-offline-upgrader/dde-offline-upgrader.pro \
    dde-suspend-dialog/dde-suspend-dialog.pro \
    dnetwork-secret-dialog/dnetwork-secret-dialog.pro \
    dde-lowpower/dde-lowpower.pro
sed -i 's|/usr/lib/dde-dock|/usr/lib64/dde-dock|' dde-notification-plugin/notifications/notifications.pro

%build
export PATH=%{_qt5_bindir}:$PATH
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DARCHITECTURE=%{_arch} .
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%post
sed -i "s|#greeter-session.*|greeter-session=lightdm-deepin-greeter|g" /etc/lightdm/lightdm.conf

%files
%doc README.md
%license LICENSE
%{_bindir}/dde-*
%{_bindir}/dmemory-warning-dialog
%{_libexecdir}/deepin-daemon/*
%{_datadir}/%{name}/
%{_datadir}/icons/hicolor/*/apps/*
%{_datadir}/dbus-1/services/*.service
%{_libdir}/dde-dock/plugins/libnotifications.so
%{_prefix}/share/glib-2.0/schemas/com.deepin.dde.dock.module.notifications.gschema.xml

%changelog
* Wed Mar 31 2021 uoser <uoser@uniontech.com> - 5.4.8-1
- Update to 5.4.8
