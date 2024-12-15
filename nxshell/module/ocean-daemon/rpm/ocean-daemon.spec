%global _smp_mflags -j1

%global debug_package   %{nil}
%global _unpackaged_files_terminate_build 0
%global _missing_build_ids_terminate_build 0
%define __debug_install_post   \
   %{_rpmconfigdir}/find-debuginfo.sh %{?_find_debuginfo_opts} "%{_builddir}/%{?buildsubdir}"\
%{nil}

%global sname lingmo-daemon
%global release_name server-industry

Name:           ocean-daemon
Version:        5.13.13
Release:        1
Summary:        Daemon handling the OCEAN session settings
License:        GPLv3
URL:            http://shuttle.corp.lingmo.com/cache/tasks/18802/unstable-amd64/
Source0:        %{name}-%{version}.orig.tar.xz

BuildRequires:  python3
BuildRequires:  gocode
BuildRequires:  compiler(go-compiler)
BuildRequires:  lingmo-gettext-tools
BuildRequires:  fontpackages-devel
BuildRequires:  librsvg2-tools
BuildRequires:  pam-devel >= 1.3.1
BuildRequires:  pam >= 1.3.1
BuildRequires:  glib2-devel
BuildRequires:  gtk3-devel
BuildRequires:  systemd-devel
BuildRequires:  alsa-lib-devel
BuildRequires:  alsa-lib
BuildRequires:  pulseaudio-libs-devel
BuildRequires:  gdk-pixbuf2-xlib-devel
BuildRequires:  gdk-pixbuf2-xlib
BuildRequires:  libnl3-devel
BuildRequires:  libnl3
BuildRequires:  libgudev-devel
BuildRequires:  libgudev
BuildRequires:  libinput-devel
BuildRequires:  libinput
BuildRequires:  librsvg2-devel
BuildRequires:  librsvg2
BuildRequires:  libXcursor-devel
BuildRequires:  ddcutil-devel
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:	golang-github-lingmoos-go-x11-client-devel
BuildRequires:  golang-github-lingmoos-go-dbus-factory-devel
BuildRequires:  go-lib-devel
BuildRequires:  go-gir-generator
BuildRequires:  ocean-api-devel

Requires:       bluez-libs
Requires:       lingmo-desktop-base
Requires:       lingmo-desktop-schemas
Requires:       ocean-session-ui
Requires:       ocean-polkit-agent
Requires:       rfkill
Requires:       gvfs
Requires:       iw
Requires:       xdotool
Requires:       gnome-keyring-pam
Requires:       imwheel
Requires:       lingmo-installer-timezones

Recommends:     lshw
Recommends:     iso-codes
Recommends:     mobile-broadband-provider-info
Recommends:     google-noto-mono-fonts
Recommends:     google-noto-sans-fonts

%description
Daemon handling the OCEAN session settings

%prep
%autosetup
patch langselector/locale.go < rpm/locale.go.patch
patch accounts/users/passwd.go < rpm/passwd.go.patch

# Fix library exec path
sed -i '/lingmo/s|lib|libexec|' Makefile
sed -i '/${DESTDIR}\/usr\/lib\/lingmo-daemon\/service-trigger/s|${DESTDIR}/usr/lib/lingmo-daemon/service-trigger|${DESTDIR}/usr/libexec/lingmo-daemon/service-trigger|g' Makefile
sed -i '/${DESTDIR}${PREFIX}\/lib\/lingmo-daemon/s|${DESTDIR}${PREFIX}/lib/lingmo-daemon|${DESTDIR}${PREFIX}/usr/libexec/lingmo-daemon|g' Makefile
sed -i 's|lib/NetworkManager|libexec|' network/utils_test.go

for file in $(grep "/usr/lib/lingmo-daemon" * -nR |awk -F: '{print $1}')
do
    sed -i 's|/usr/lib/lingmo-daemon|/usr/libexec/lingmo-daemon|g' $file
done

# Fix grub.cfg path
sed -i 's|boot/grub|boot/grub2|' grub2/{grub2,grub_params,theme}.go

# Fix activate services failed (Permission denied)
# dbus service
pushd misc/system-services/
sed -i '$aSystemdService=lingmo-accounts-daemon.service' org.lingmo.ocean.Power1.service \
    org.lingmo.ocean.{Accounts,Apps,Daemon}1.service \
    org.lingmo.ocean.{Gesture,SwapSchedHelper,Timedated}1.service
sed -i '$aSystemdService=dbus-org.lingmo.ocean.Lockservice1.service' org.lingmo.ocean.LockService1.service
popd
# systemd service
cat > misc/systemd/services/dbus-org.lingmo.ocean.Lockservice1.service <<EOF
[Unit]
Description=Lingmo Lock Service
Wants=user.slice dbus.socket
After=user.slice dbus.socket

[Service]
Type=dbus
BusName=org.lingmo.ocean.LockService1
ExecStart=%{_libexecdir}/%{sname}/ocean-lockservice

[Install]
WantedBy=graphical.target
EOF

# Replace reference of google-chrome to chromium-browser
sed -i 's/google-chrome/chromium-browser/g' misc/ocean-daemon/mime/data.json

%build
BUILDID="0x$(head -c20 /dev/urandom|od -An -tx1|tr -d ' \n')"
export GOPATH=/usr/share/gocode
%make_build GO_BUILD_FLAGS=-trimpath GOBUILD="go build -compiler gc -ldflags \"-B $BUILDID\""
#make GOPATH=/usr/share/gocode

%install
BUILDID="0x$(head -c20 /dev/urandom|od -An -tx1|tr -d ' \n')"
export GOPATH=/usr/share/gocode
%make_install PAM_MODULE_DIR=%{_libdir}/security GOBUILD="go build -compiler gc -ldflags \"-B $BUILDID\""

# fix systemd/logind config
install -d %{buildroot}/usr/lib/systemd/logind.conf.d/
cat > %{buildroot}/usr/lib/systemd/logind.conf.d/10-%{sname}.conf <<EOF
[Login]
HandlePowerKey=ignore
HandleSuspendKey=ignore
EOF

%find_lang %{name}

%post
if [ $1 -ge 1 ]; then
  systemd-sysusers %{sname}.conf
  %{_sbindir}/alternatives --install %{_bindir}/x-terminal-emulator \
    x-terminal-emulator %{_libexecdir}/%{sname}/default-terminal 30
fi

%preun
if [ $1 -eq 0 ]; then
  %{_sbindir}/alternatives --remove x-terminal-emulator \
    %{_libexecdir}/%{sname}/default-terminal
fi

%postun
if [ $1 -eq 0 ]; then
  rm -f /var/cache/lingmo/mark-setup-network-services
  rm -f /var/log/lingmo.log 
fi

%files -f %{name}.lang
%doc README.md
%license LICENSE
%{_sysconfdir}/default/grub.d/10_lingmo.cfg
%{_sysconfdir}/grub.d/35_lingmo_gfxmode
%{_sysconfdir}/pam.d/lingmo-auth-keyboard
%{_libexecdir}/%{sname}/
%{_prefix}/lib/systemd/logind.conf.d/10-%{sname}.conf
%{_datadir}/dbus-1/services/*.service
%{_datadir}/dbus-1/system-services/*.service
%{_datadir}/dbus-1/system.d/*.conf
%{_datadir}/icons/hicolor/*/status/*
%{_datadir}/%{name}/
%{_datadir}/ocean/
%{_datadir}/polkit-1/actions/*.policy
%{_var}/lib/polkit-1/localauthority/10-vendor.d/org.lingmo.ocean.accounts.pkla
%{_var}/lib/polkit-1/localauthority/10-vendor.d/org.lingmo.ocean.grub2.pkla
%{_sysconfdir}/acpi/actions/lingmo_lid.sh
%{_sysconfdir}/acpi/events/lingmo_lid
%{_sysconfdir}/pulse/daemon.conf.d/10-lingmo.conf
/lib/udev/rules.d/80-lingmo-fprintd.rules
/var/lib/polkit-1/localauthority/10-vendor.d/org.lingmo.ocean.fprintd.pkla
/lib/systemd/system/dbus-org.lingmo.ocean.Lockservice1.service
/lib/systemd/system/lingmo-accounts-daemon.service

%changelog
* Tue Apr 8 2021 uoser <uoser@uniontech.com> - 5.13.13-1
- Update to 5.13.13

