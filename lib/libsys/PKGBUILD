pkgname="liblingmo"
pkgver="2.0.0"
pkgrel="1"
pkgdesc="LingmoOS & LingmoDE Library"
arch=("x86_64")
depends=('kscreen' 'modemmanager-qt5' 'networkmanager-qt5' 'qt5-quickcontrols2' 'accountsservice' 'bluez' 'bluez-qt5')
url="https://github.com/LingmoOS/liblingmo"
makedepends=('extra-cmake-modules' 'ninja' 'qt5-tools' 'git')
provides=("$pkgname")
conflicts=("$pkgname")
source=("git+$url.git")
license=("GPL2.0")
sha512sums=("SKIP")

build() {
	cd $pkgname
	cmake -GNinja -DCMAKE_INSTALL_PREFIX=/usr .
	ninja
}

package() {
	cd $pkgname
	DESTDIR="$pkgdir" ninja install
}
