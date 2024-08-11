pkgname=kwayland
pkgver=6.1.3
pkgrel=1
pkgdesc='Qt-style Client and Server library wrapper for the Wayland libraries'
arch=(x86_64)
url='https://github.com/LingmoOS/kwayland'
license=(LGPL-2.0-or-later)
depends=(gcc-libs
         glibc
         qt6-base
         qt6-wayland
         wayland)
makedepends=(doxygen
             extra-cmake-modules
             lingmo-wayland-protocols
             qt6-doc
             qt6-tools
             wayland-protocols)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build  -S $pkgname \
    -DCMAKE_INSTALL_LIBEXECDIR=lib \
    -DBUILD_TESTING=OFF \
    -DBUILD_QCH=ON
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

