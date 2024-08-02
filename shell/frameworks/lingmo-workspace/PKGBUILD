pkgname=lingmo-workspace
pkgver=2.9.0
pkgrel=1
pkgdesc='Lingmo OS Workspace'
arch=(x86_64)
url='https://github.com/LingmoOS/lingmo-workspace'
license=(LGPL-2.0-or-later)
depends=(accountsservice
         appstream-qt
         fontconfig
         freetype2
         gcc-libs
         glibc
         icu
         lingmo-activities
         lingmo-activities-stats
         kactivitymanagerd
         karchive
         kauth
         kbookmarks
         kcmutils
         kcolorscheme
         kcompletion
         kconfig
         kconfigwidgets
         kcoreaddons
         kcrash
         kde-cli-tools
         kdeclarative
         kded
         kdbusaddons
         kglobalaccel
         kguiaddons
         kholidays
         ki18n
         kiconthemes
         kidletime
         kio
         kio-extras
         kio-fuse
         lingmoui
         lingmoui-addons
         kitemmodels
         kjobwidgets
         knewstuff
         knotifications
         knotifyconfig
         kpackage
         kparts
         kpipewire
         krunner
         kquickcharts
         kscreenlocker
         kservice
         kstatusnotifieritem
         ksvg
         ksystemstats
         ktexteditor
         ktextwidgets
         kuserfeedback
         kwallet
         kwayland
         kwidgetsaddons
         kwin
         kwindowsystem
         kxmlgui
         layer-shell-qt
         libcanberra
         libice
         libkexiv2
         libksysguard
         liblingmo
         libqalculate
         libsm
         libx11
         libxau
         libxcb
         libxcrypt
         libxcursor
         libxfixes
         libxft
         libxtst
         milou
         ocean-sound-theme
         phonon-qt6
         lingmo5support
         prison
         qcoro-qt6
         qt6-5compat
         qt6-base
         qt6-declarative
         qt6-svg
         qt6-tools # for qdbus
         qt6-virtualkeyboard
         qt6-wayland
         sh
         solid
         systemd-libs
         wayland
         xcb-util
         xcb-util-image
         xorg-xmessage
         xorg-xrdb
         xorg-xsetroot
         xorg-xwayland
         zlib)
makedepends=(baloo
             extra-cmake-modules
             gpsd
             kdoctools
             kunitconversion
             networkmanager-qt
             lingmo-wayland-protocols)
optdepends=('appmenu-gtk-module: global menu support for GTK2 and some GTK3 applications'
            'baloo: Baloo search runner'
            'discover: manage applications installation from the launcher'
            'gpsd: GPS based geolocation'
            'kdepim-addons: displaying PIM events in the calendar'
            'kwayland-integration: Wayland integration for Qt5 applications'
            'networkmanager-qt: IP based geolocation'
            'lingmo-workspace-wallpapers: additional wallpapers'
            'lingmo5-integration: use Lingmo OS settings in Qt5 applications'
            'xdg-desktop-portal-gtk: sync font settings to Flatpak apps')
conflicts=(lingmo-wayland-session)
replaces=(lingmo-wayland-session)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DCMAKE_INSTALL_LIBEXECDIR=lib \
    -DGLIBC_LOCALE_GEN=OFF \
    -DBUILD_TESTING=OFF
  cmake --build build -j12
}

package() {
  depends+=(lingmo-integration) # Declare runtime dependency here to avoid dependency cycles at build time

  DESTDIR="$pkgdir" cmake --install build
}

