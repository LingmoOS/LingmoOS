pkgname=lingmo-activities-stats
pkgver=2.9.0
pkgrel=1
pkgdesc='A library for accessing the usage data collected by the activities system'
arch=(x86_64)
url='https://github.com/LingmoOS/lingmo-activities-stats'
license=(LGPL-2.0-or-later)
depends=(gcc-libs
         glibc
         lingmo-activities
         kconfig
         qt6-base)
makedepends=(doxygen
             extra-cmake-modules
             qt6-doc
             qt6-tools)
conflicts=(kactivities-stats)
replaces=(kactivities-stats)
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

