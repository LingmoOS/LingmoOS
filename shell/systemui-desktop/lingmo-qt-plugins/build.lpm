# Maintainer: Lingmo OS Team <team@lingmo.org>

pkgname=lingmo-qt-plugins
pkgver=2.0.0
pkgrel=1
pkgdesc="Unify Qt application style of Lingmo OS"
arch=('x86_64')
url="https://github.com/LingmoOS/lingmo-qt-plugins"
license=('GPL')
groups=('lingmo')
depends=('kwindowsystem' 'libdbusmenu-qt5' 'libqtxdg' 'qt5-quickcontrols2')
makedepends=('extra-cmake-modules' 'ninja' 'qt5-tools' 'git')
provides=("$pkgname")
conflicts=("$pkgname")
source=("git+$url.git")
sha512sums=('SKIP')

build() {
  cd $pkgname

  cmake -GNinja -DCMAKE_INSTALL_PREFIX=/usr .
  ninja
}

package() {
  cd $pkgname
  DESTDIR="$pkgdir" ninja install
}
