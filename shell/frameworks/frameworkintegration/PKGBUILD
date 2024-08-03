pkgname=frameworkintegration
pkgver=6.4.0
pkgrel=1
pkgdesc='Framework providing components to allow applications to integrate with a Lingmo OS Workspace'
arch=(x86_64)
url='https://github.com/LingmoOS/frameworkintegration'
license=(LGPL-2.0-only LGPL-3.0-only)
depends=(gcc-libs
         glibc
         kcolorscheme
         kconfig
         ki18n
         kiconthemes
         knewstuff
         knotifications
         kwidgetsaddons
         qt6-base)
makedepends=(appstream-qt
             extra-cmake-modules
             packagekit-qt6)
optdepends=('appstream-qt: dependency resolving via AppStream'
            'packagekit-qt6: dependency resolving via AppStream')
groups=(kf6)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build -S $pkgname \
    -DCMAKE_INSTALL_LIBEXECDIR=lib \
    -DBUILD_TESTING=OFF
  cmake --build build -j12
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

