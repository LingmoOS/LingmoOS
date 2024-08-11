pkgname=kded
pkgver=6.4.0
pkgrel=1
pkgdesc='Extensible deamon for providing system level services'
arch=(x86_64)
url='https://github.com/LingmoOS/$pkgname'
license=(LGPL-2.0-only LGPL-3.0-only)
depends=(gcc-libs
         glibc
         kconfig
         kcoreaddons
         kcrash
         kdbusaddons
         kservice
         qt6-base)
makedepends=(extra-cmake-modules
             kdoctools)
groups=(kf6)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DBUILD_TESTING=OFF
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

