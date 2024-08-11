pkgname=kcmutils
pkgver=6.4.0
pkgrel=1
pkgdesc='Utilities for interacting with KCModules'
arch=(x86_64)
url='https://github.com/LingmoOS/kcmutils'
license=(LGPL-2.0-only LGPL-3.0-only)
depends=(gcc-libs
         glibc
         kconfig
         kconfigwidgets
         kcoreaddons
         kguiaddons
         ki18n
         kio
         lingmoui
         kitemviews
         kservice
         kwidgetsaddons
         kxmlgui
         qt6-base
         qt6-declarative)
makedepends=(doxygen
             extra-cmake-modules
             qt6-doc
             qt6-tools)
groups=(kf6)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_LIBEXECDIR=lib \
    -DBUILD_QCH=ON
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

