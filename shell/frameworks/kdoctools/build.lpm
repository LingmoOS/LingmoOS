pkgname=kdoctools
pkgver=6.4.0
pkgrel=1
pkgdesc='Documentation generation from docbook'
arch=(x86_64)
url='https://github.com/LingmoOS/kdoctools'
license=(LGPL-2.0-only LGPL-3.0-only)
depends=(docbook-xsl
         gcc-libs
         glibc
         karchive
         libxml2
         libxslt
         qt6-base)
makedepends=(doxygen
             extra-cmake-modules
             ki18n
             perl-uri
             qt6-doc
             qt6-tools)
groups=(kf6)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DBUILD_TESTING=OFF \
    -DBUILD_QCH=ON \
    -DDocBookXSL_DIR=/usr/share/xml/docbook/xsl-stylesheets-1.79.2-nons
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

