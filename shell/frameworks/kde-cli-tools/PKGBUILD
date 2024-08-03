pkgname=kde-cli-tools
pkgver=2.9.0
_dirver=$(echo $pkgver | cut -d. -f1-3)
pkgrel=1
pkgdesc='Tools based on KDE Frameworks 5 to better interact with the system'
arch=(x86_64)
url='https://github.com/LingmoOS/$pkgname'
license=(LGPL-2.0-or-later)
depends=(gcc-libs
         glibc
         kcmutils
         kcompletion
         kconfig
         kcoreaddons
         kdesu
         ki18n
         kiconthemes
         kio
         kparts
         kservice
         kwidgetsaddons
         kwindowsystem
         qt6-base
         qt6-svg
         sh)
makedepends=(extra-cmake-modules
             kdoctools)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build  -S $pkgname \
    -DCMAKE_INSTALL_LIBEXECDIR=lib \
    -DBUILD_TESTING=OFF
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

