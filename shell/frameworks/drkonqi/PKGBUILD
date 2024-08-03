pkgname=drkonqi
pkgver=2.9.0
_dirver=$(echo $pkgver | cut -d. -f1-3)
pkgrel=1
pkgdesc='The Lingmo OS crash handler'
arch=(x86_64)
url='https://github.com/LingmoOS/lingmo-drkonqi'
license=(GPL-2.0-or-later)
depends=(gcc-libs
         gdb
         glibc
         kcmutils
         kconfig
         kcoreaddons
         kcrash
         ki18n
         kidletime
         kio
         lingmoui
         kitemmodels
         knotifications
         kservice
         kstatusnotifieritem
         kwallet
         kwidgetsaddons
         kwindowsystem
         polkit-qt6
         python
         python-psutil
         python-pygdbmi
         python-sentry_sdk
         qt6-base
         qt6-declarative
         syntax-highlighting
         systemd-libs)
makedepends=(extra-cmake-modules)
groups=(lingmo)
source=(git+$url)
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cmake -B build  -S lingmo-$pkgname \
    -DCMAKE_INSTALL_LIBEXECDIR=lib \
    -DBUILD_TESTING=OFF \
    -DWITH_GDB12=ON \
    -DWITH_PYTHON_VENDORING=OFF
  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

