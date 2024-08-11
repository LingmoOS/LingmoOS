pkgname=ocean-icons
pkgver=2.9.0
pkgrel=1
pkgdesc='Ocean icon theme'
arch=(x86_64)
url='https://github.com/LingmoOS/ocean-icons'
license=(GPL-3.0-or-later
         LGPL-2.1-only)
depends=(glibc
         qt6-base)
makedepends=(extra-cmake-modules
             python-lxml)
groups=(kf6)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DBINARY_ICONS_RESOURCE=ON \
    -DBUILD_TESTING=OFF
  cmake --build build -j20
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}
