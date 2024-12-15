Name:           ocean-session-ui
Version:        5.4.8
Release:        1
Summary:        Lingmo desktop-environment - Session UI module
License:        GPLv3
URL:            https://github.com/lingmoos/%{name}
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  lingmo-gettext-tools
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
BuildRequires:  ocean-dock-devel
Requires:       ocean-daemon
Requires:       startocean

Requires:       lightdm
Requires(post): sed
Provides:       lightdm-lingmo-greeter = %{version}-%{release}
Provides:       lightdm-greeter = 1.2
Provides:       lingmo-notifications = %{version}-%{release}
Obsoletes:      lingmo-notifications < %{version}-%{release}

%description
This project include those sub-project:

- ocean-shutdown: User interface of shutdown.
- ocean-lock: User interface of lock screen.
- ocean-lockservice: The back-end service of locking screen.
- lightdm-lingmo-greeter: The user interface when you login in.
- ocean-switchtogreeter: The tools to switch the user to login in.
- ocean-lowpower: The user interface of reminding low power.
- ocean-osd: User interface of on-screen display.
- ocean-hotzone: User interface of setting hot zone.

%prep
%setup -q -n %{name}-%{version}
sed -i 's|default_background.jpg|default.png|' widgets/fullscreenbackground.cpp
sed -i 's|lib|libexec|' \
    misc/applications/lingmo-toggle-desktop.desktop* \
    ocean-osd/ocean-osd_autostart.desktop \
    ocean-osd/com.lingmo.ocean.osd.service \
    ocean-osd/notification/files/com.lingmo.ocean.*.service* \
    ocean-osd/ocean-osd.pro \
    ocean-welcome/com.lingmo.ocean.welcome.service \
    ocean-welcome/ocean-welcome.pro \
    ocean-bluetooth-dialog/ocean-bluetooth-dialog.pro \
    ocean-touchscreen-dialog/ocean-touchscreen-dialog.pro \
    ocean-warning-dialog/com.lingmo.ocean.WarningDialog.service \
    ocean-warning-dialog/ocean-warning-dialog.pro \
    ocean-offline-upgrader/ocean-offline-upgrader.pro \
    ocean-suspend-dialog/ocean-suspend-dialog.pro \
    dnetwork-secret-dialog/dnetwork-secret-dialog.pro \
    ocean-lowpower/ocean-lowpower.pro
sed -i 's|/usr/lib/ocean-dock|/usr/lib64/ocean-dock|' ocean-notification-plugin/notifications/notifications.pro

%build
export PATH=%{_qt5_bindir}:$PATH
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DARCHITECTURE=%{_arch} .
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%post
sed -i "s|#greeter-session.*|greeter-session=lightdm-lingmo-greeter|g" /etc/lightdm/lightdm.conf

%files
%doc README.md
%license LICENSE
%{_bindir}/ocean-*
%{_bindir}/dmemory-warning-dialog
%{_libexecdir}/lingmo-daemon/*
%{_datadir}/%{name}/
%{_datadir}/icons/hicolor/*/apps/*
%{_datadir}/dbus-1/services/*.service
%{_libdir}/ocean-dock/plugins/libnotifications.so
%{_prefix}/share/glib-2.0/schemas/com.lingmo.ocean.dock.module.notifications.gschema.xml

%changelog
* Wed Mar 31 2021 uoser <uoser@uniontech.com> - 5.4.8-1
- Update to 5.4.8
