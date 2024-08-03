pkgname=kactivitymanagerd
pkgver=2.9.0
_dirver=$(echo $pkgver | cut -d. -f1-3)
pkgrel=1
pkgdesc='System service to manage user activities and track the usage patterns'
arch=(x86_64)
url='https://github.com/LingmoOS/kactivitymanagerd'
license=(LGPL-2.0-or-later)
depends=(gcc-libs
         glibc
         kconfig
         kcoreaddons
         kcrash
         kglobalaccel
         ki18n
         kio
         kservice
         kwindowsystem
         kxmlgui
         qt6-base)
makedepends=(boost
             extra-cmake-modules)
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

