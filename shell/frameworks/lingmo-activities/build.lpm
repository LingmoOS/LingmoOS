pkgname=lingmo-activities
pkgver=2.9.0
pkgrel=1
pkgdesc='Core components for Lingmo OS Activities'
arch=(x86_64)
url='https://github.com/LingmoOS/lingmo-activities'
license=(LGPL-2.0-or-later)
depends=(gcc-libs
         glibc
         kconfig
         kcoreaddons
         qt6-base)
makedepends=(boost
             doxygen
             extra-cmake-modules
             qt6-declarative
             qt6-doc
             qt6-tools)
optdepends=('qt6-declarative: QML bindings')
conflicts=(kactivities)
replaces=(kactivities)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build  -S $pkgname \
    -DBUILD_TESTING=OFF \
    -DBUILD_QCH=ON
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

