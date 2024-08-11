pkgname=extra-cmake-modules
pkgver=6.4.0
pkgrel=1
pkgdesc='Extra modules and scripts for CMake'
arch=(any)
url='https://github.com/LingmoOS/extra-cmake-modules'
license=(LGPL)
depends=(cmake)
makedepends=(python-requests
             python-sphinx)
optdepends=('python-pyxdg: to generate fastlane metadata for Android apps'
            'python-requests: to generate fastlane metadata for Android apps'
            'python-yaml: to generate fastlane metadata for Android apps')
groups=(kf6)
source=(git+$url)
sha256sums=('SKIP')
            
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DBUILD_HTML_DOCS=ON \
    -DBUILD_QTHELP_DOCS=ON
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

