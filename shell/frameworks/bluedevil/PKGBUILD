pkgname=bluedevil
pkgver=2.9.0
_dirver=$(echo $pkgver | cut -d. -f1-3)
pkgrel=1
epoch=1
pkgdesc='Integrate the Bluetooth technology within Lingmo OS workspace and applications'
arch=(x86_64)
url='https://github.com/LingmoOS/bluedevil'
license=(GPL-2.0-or-later)
depends=(bluez-qt
         gcc-libs
         glibc
         kcmutils
         kconfig
         kcoreaddons
         kdbusaddons
         kdeclarative
         ki18n
         kio
         lingmoui
         kjobwidgets
         knotifications
         kservice
         ksvg
         kwidgetsaddons
         kwindowsystem
         liblingmo
         qt6-base
         qt6-declarative)
makedepends=(extra-cmake-modules
             kdoctools)
optdepends=('bluez-obex: file transfer'
            'pulseaudio-bluetooth: to connect to A2DP profile')
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

