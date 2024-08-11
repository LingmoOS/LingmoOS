pkgname=flatpak-kcm
pkgver=2.9.0
_dirver=$(echo $pkgver | cut -d. -f1-3)
pkgrel=1
pkgdesc='Flatpak Permissions Management KCM'
arch=(x86_64)
url='https://github.com/LingmoOS/flatpak-kcm'
license=(LGPL-2.0-or-later)
depends=(flatpak
         gcc-libs
         glib2
         glibc
         kcmutils
         kconfig
         kcoreaddons
         ki18n
         lingmoui
         kitemmodels
         qt6-base
         qt6-declarative)
makedepends=(extra-cmake-modules)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build  -S $pkgname \
    -DBUILD_TESTING=OFF
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

