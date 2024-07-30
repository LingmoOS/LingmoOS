pkgname=namcap
pkgver=3.5.2
pkgrel=2
pkgdesc='A Lpm package analyzer'
arch=(any)
url="https://github.com/LingmoOS/namcap"
license=(GPL-2.0-or-later)
depends=(binutils
         elfutils
         licenses
         pkgconf
         pyalpm
         python
         python-license-expression
         python-pyelftools)
checkdepends=(python-pytest
              python-six
              systemd)
makedepends=(python-{build,installer,wheel}
            python-setuptools)
source=("git+https://github.com/LingmoOS/namcap")
sha256sums=('SKIP')
validpgpkeys=(41EF7182553A87AC18196A77F27F2FDA54F067D8)  # Lingmo OS Team
build() {
  cd "$pkgname"
  python -m build -wn
}

check() {
  cd "$pkgname"
  env PARSE_PKGBUILD_PATH="$PWD" PATH="$PWD/scripts:$PATH" pytest
}

package() {
  cd "$pkgname"
  python -m installer -d "$pkgdir" dist/*.whl
  local site_packages="$(python -c "import site; print(site.getsitepackages()[0])")"
  mv "$pkgdir/"{"$site_packages/usr/share",usr}
  rmdir "$pkgdir/$site_packages/usr"
}
