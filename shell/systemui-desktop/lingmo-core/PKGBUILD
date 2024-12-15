pkgname="lingmo-core"
pkgver="2.0.0"
pkgrel="1"
pkgdesc="LingmoOS & LingmoDE Core"
arch=("x86_64")
depends=('lingmoui' 'liblingmo' 'qt5-quickcontrols2' 'libpulse' 'libxcursor' 'libxtst' 'polkit-qt5')
makedepends=('extra-cmake-modules' 'ninja' 'qt5-tools'  'xf86-input-libinput' 'xf86-input-synaptics' 'xorg-server-devel' 'git')
license=('GPL')
source=("./*")
sha512sums=("SKIP")
provides=("$pkgname")
conflicts=("$pkgname")
build() {
	cmake -DCMAKE_INSTALL_PREFIX=/usr .
	make
}

package() {
	make DESTDIR="$pkgdir" install
}
