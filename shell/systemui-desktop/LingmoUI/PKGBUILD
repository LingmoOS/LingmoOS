pkgname="lingmoui"
pkgver="2.0.0"
pkgrel="1"
pkgdesc="Lingmo OS GUI library"
arch=("x86_64")
url=("https://github.com/LingmoOS/LingmoUI")
license=("GPL")
depends=('kwindowsystem' 'qt5-declarative' 'qt5-graphicaleffects' 'qt5-quickcontrols2')
makedepends=('git' 'extra-cmake-modules' 'ninja' 'qt5-tools')
provides=("$pkgname")
conflicts=("$pkgname")
source=("git+$url.git")
sha512sums=('SKIP')

build() {
	cd LingmoUI/
	cmake -GNinja -DCMAKE_INSTALL_PREFIX=/usr .
  	ninja
}

package() {
	cd LingmoUI
	DESTDIR="$pkgdir" ninja install
}
