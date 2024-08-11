pkgname=lingmo-keyring
pkgver=3.0
pkgrel=1
pkgdesc='Lingmo OS PGP keyring'
arch=('any')
url='https://github.com/LingmoOS/lingmo-keyring/'
license=('GPL-3.0-or-later')
install=$pkgname.install
depends=('lpm')
makedepends=('git' 'python' 'sequoia-sq' 'pkgconf' 'systemd')
checkdepends=('python-coverage' 'python-pytest')
source=("git+https://github.com/LingmoOS/lingmo-keyring")
sha256sums=('SKIP')
validpgpkeys=('41EF7182553A87AC18196A77F27F2FDA54F067D8') # Lingmo OS Team <team@lingmo.org>

build() {
  cd lingmo-keyring/

  make build
}

check() {
  cd lingmo-keyring/

  make check
}

package() {
  cd lingmo-keyring/

  make PREFIX='/usr' DESTDIR="${pkgdir}" install
}
