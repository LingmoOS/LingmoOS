pkgname=lingmo-desktop
pkgver=2.0.0
pkgrel=1
pkgdesc='Lingmo OS Desktop'
arch=(x86_64)
url='https://github.com/LingmoOS/lingmo-desktop'
license=(LGPL-2.0-or-later)
depends=(attica
         baloo
         emoji-font # for clock and language KCMs
         gcc-libs
         glibc
         icu
         kauth
         kbookmarks
         kcmutils
         kcodecs
         kcompletion
         kconfig
         kconfigwidgets
         kcoreaddons
         kcrash
         kdbusaddons
         kdeclarative
         kglobalaccel
         kguiaddons
         ki18n
         kiconthemes
         kio
         lingmoui
         lingmoui-addons
         kitemmodels
         kitemviews
         kjobwidgets
         kmenuedit
         knewstuff
         knotifications
         knotifyconfig
         kpackage
         kpipewire
         krunner
         kservice
         ksvg
         kwidgetsaddons
         kwindowsystem
         kxmlgui
         libcanberra
         libksysguard
         libx11
         libxcb
         libxcursor
         libxi
         libxkbcommon
         libxkbfile
         liblingmo
         lingmo-activities
         lingmo-activities-stats
         lingmo-workspace
         lingmo5support
         polkit-kde-agent
         powerdevil
         qt6-5compat
         qt6-base
         qt6-declarative
         qt6-wayland
         sdl2
         solid
         sonnet
         systemsettings
         wayland
         xcb-util-keysyms
         xdg-user-dirs)
optdepends=('bluedevil: Bluetooth applet'
            'glib2: kimpanel IBUS support'
            'ibus: kimpanel IBUS support'
            'kaccounts-integration: OpenDesktop integration plugin'
            'kscreen: screen management'
            'libaccounts-qt: OpenDesktop integration plugin'
            'packagekit-qt6: to install new krunner plugins'
            'lingmo-nm: Network manager applet'
            'lingmo-pa: Audio volume applet'
            'scim: kimpanel SCIM support')
makedepends=(extra-cmake-modules
             intltool
             kaccounts-integration
             kdoctools
             libibus
             packagekit-qt6
             scim
             wayland-protocols
             xf86-input-libinput
             xorg-server-devel)
source=(git+https://github.com/LingmoOS/lingmo-desktop)
sha256sums=('SKIP'
            'SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build  -S $pkgname \
    -DCMAKE_INSTALL_LIBEXECDIR=lib \
    -DBUILD_TESTING=OFF
  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}
