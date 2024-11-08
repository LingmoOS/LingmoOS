pkgname=liblingmo
pkgver=2.9.0
pkgrel=1
pkgdesc='Lingmo OS library and runtime components'
arch=(x86_64)
url='https://github.com/LingmoOS/liblingmo'
license=(LGPL-2.0-or-later)
depends=(gcc-libs
         glibc
         lingmo-activities
         kcmutils
         kcolorscheme
         kconfig
         kcoreaddons
         kglobalaccel
         ki18n
         kiconthemes
         kio
         lingmoui
         knotifications
         kpackage
         ksvg
         kwidgetsaddons
         kwindowsystem
         libglvnd
         libx11
         libxcb
         qt6-5compat
         qt6-base
         qt6-declarative
         qt6-wayland
         wayland)
makedepends=(doxygen
             extra-cmake-modules
             kdoctools
             lingmo-wayland-protocols
             qt6-doc
             qt6-tools)
conflicts=(lingmo-framework)
replaces=(lingmo-framework)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build  -S $pkgname \
    -DBUILD_TESTING=OFF \
    -DBUILD_QCH=ON
  cmake --build build -j8
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

