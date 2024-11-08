pkgname=bash
_basever=5.2
_patchlevel=026
pkgver=${_basever}.${_patchlevel}
pkgrel=5
pkgdesc='The GNU Bourne Again shell'
arch=(x86_64)
license=('GPL-3.0-or-later')
url='https://www.gnu.org/software/bash/bash.html'
backup=(etc/bash.bash{rc,_logout} etc/skel/.bash{rc,_profile,_logout})
depends=(readline libreadline.so glibc ncurses)
optdepends=('bash-completion: for tab completion')
provides=('sh')
install=bash.install
source=(https://ftp.gnu.org/gnu/bash/bash-$_basever.tar.gz{,.sig}
        bash-5.2_p15-configure-clang16.patch
        bash-5.2_p15-random-ub.patch
        bash-5.2_p21-configure-strtold.patch
        bash-5.2_p21-wpointer-to-int.patch
        bash-5.2_p26-memory-leaks.patch
        dot.bashrc
        dot.bash_profile
        dot.bash_logout
        system.bashrc
        system.bash_logout)
validpgpkeys=('7C0135FB088AAF6C66C650B9BB5869F064EA74AB') # Chet Ramey

if [[ $((10#${_patchlevel})) -gt 0 ]]; then
  for (( _p=1; _p<=$((10#${_patchlevel})); _p++ )); do
    source=(${source[@]} https://ftp.gnu.org/gnu/bash/bash-$_basever-patches/bash${_basever//.}-$(printf "%03d" $_p){,.sig})
  done
fi
b2sums=('SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP'
        'SKIP')

prepare() {
  cd $pkgname-$_basever
  for (( _p=1; _p<=$((10#${_patchlevel})); _p++ )); do
    local patch="bash${_basever//.}-$(printf "%03d" $_p)"
    patch -Np0 -i ../"${patch}"
  done
  # add patches from gentoo, fixing various upstream reported issues
  # https://gitweb.gentoo.org/repo/gentoo.git/tree/app-shells/bash/files
  patch -Np0 -i ../bash-5.2_p15-configure-clang16.patch
  patch -Np0 -i ../bash-5.2_p15-random-ub.patch
  patch -Np0 -i ../bash-5.2_p21-configure-strtold.patch
  patch -Np0 -i ../bash-5.2_p21-wpointer-to-int.patch
  patch -Np0 -i ../bash-5.2_p26-memory-leaks.patch
}

build() {
  cd $pkgname-$_basever

  _bashconfig=(-DDEFAULT_PATH_VALUE=\'\"/usr/local/sbin:/usr/local/bin:/usr/bin\"\'
               -DSTANDARD_UTILS_PATH=\'\"/usr/bin\"\'
               -DSYS_BASHRC=\'\"/etc/bash.bashrc\"\'
               -DSYS_BASH_LOGOUT=\'\"/etc/bash.bash_logout\"\'
               -DNON_INTERACTIVE_LOGIN_SHELLS)
  export CFLAGS="${CFLAGS} ${_bashconfig[@]}"

  ./configure \
    --prefix=/usr \
    --with-curses \
    --enable-readline \
    --without-bash-malloc \
    --with-installed-readline
  make
}

check() {
  make -C $pkgname-$_basever check
}

package() {
  make -C $pkgname-$_basever DESTDIR="$pkgdir" install
  ln -s bash "$pkgdir/usr/bin/sh"
  ln -s bash "$pkgdir/usr/bin/rbash"

  # system-wide configuration files
  install -Dm644 system.bashrc "$pkgdir/etc/bash.bashrc"
  install -Dm644 system.bash_logout "$pkgdir/etc/bash.bash_logout"

  # user configuration file skeletons
  install -dm755 "$pkgdir/etc/skel/"
  install -m644 dot.bashrc "$pkgdir/etc/skel/.bashrc"
  install -m644 dot.bash_profile "$pkgdir/etc/skel/.bash_profile"
  install -m644 dot.bash_logout "$pkgdir/etc/skel/.bash_logout"
}
