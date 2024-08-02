pkgbase=lingmo-integration
pkgname=(lingmo-integration
         lingmo5-integration)
pkgver=2.9.0
_dirver=$(echo $pkgver | cut -d. -f1-3)
pkgrel=2
pkgdesc='Qt Platform Theme integration plugins for the Lingmo OS workspaces'
arch=(x86_64)
url='https://github.com/LingmoOS/lingmo-integration'
license=(LGPL-2.0-or-later)
depends=(gcc-libs
         glibc
         kcolorscheme
         kcompletion
         kconfig
         kcoreaddons
         kguiaddons
         kiconthemes
         ki18n
         kio
         kjobwidgets
         kservice
         kstatusnotifieritem
         kwidgetsaddons
         kwindowsystem
         kxmlgui
         libxcb
         libxcursor
         qqc2-breeze-style
         qqc2-desktop-style
         qt6-base
         qt6-declarative
         qt6-wayland
         wayland)
makedepends=(extra-cmake-modules
             lingmo-wayland-protocols
             kconfig5
             ki18n5
             kiconthemes5
             kio5
             knotifications5
             kwayland5
             kwindowsystem5
             kxmlgui5
             qt5-base
             qt5-declarative
             qt5-quickcontrols2
             qt5-wayland
             qt5-x11extras)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DBUILD_TESTING=OFF \
    -DBUILD_QT5=OFF
  cmake --build build -j12

  cmake -B build5 -S $pkgname \
    -DBUILD_TESTING=OFF \
    -DBUILD_QT6=OFF
  cmake --build build5 -j12
}

package_lingmo-integration() {
  depends+=(noto-fonts
            ttf-hack
            xdg-desktop-portal-kde)
  DESTDIR="$pkgdir" cmake --install build
}

package_lingmo5-integration() {
  pkgdesc='Qt5 Platform Theme integration plugins for the Plasma workspaces'
  depends=(gcc-libs
           glibc
           kcompletion5
           kconfig5
           kconfigwidgets5
           kcoreaddons5
           ki18n5
           kiconthemes5
           kio5
           kjobwidgets5
           knotifications5
           kservice5
           kwayland5
           kwidgetsaddons5
           kwindowsystem5
           kxmlgui5
           libxcb
           libxcursor
           qt5-base
           qt5-declarative
           qt5-quickcontrols2
           qt5-wayland
           qt5-x11extras
           wayland)
  conflicts=('lingmo-integration<5.27.80')
  replaces=('lingmo-integration<5.27.80')
  groups=()

  DESTDIR="$pkgdir" cmake --install build5
}

