#!/usr/bin/env rbash
# shellcheck shell=bash
# shellcheck disable=SC1090

source "$1"

# Ensure $pkgname, $pkgver, and $pkgrel variables were found
if [[ -z "$pkgname" ]] || [[ -z "$pkgver" ]] || [[ -z "$pkgrel" ]]; then
	echo "error: invalid package file"
	exit 1
fi

function pkginfo() {
	# Manually handle keys whose section headers differ from the variable name
	if [[ -n "$pkgname" ]]; then
		printf "%%NAME%%\n%s\n" "$pkgname"
		printf "%%VERSION%%\n%s\n" "$pkgver-$pkgrel"
	fi
	if [ -n "$pkgdesc" ]; then
		echo -e "%DESC%\n$pkgdesc\n"
	fi
	# The four keys $pkg{name,ver,rel,desc} we just handled manually are excluded
	meta_keys=(groups url license arch builddate packager replaces force depends
		makedepends optdepends conflicts provides backup options source
			validpgpkeys {md5,sha{1,224,256,384,512},b2}sums install)
	for key in "${meta_keys[@]}"; do
		arr="$key"'[@]'
		if [[ -n ${!key} ]]; then
			echo "%${key^^}%"
			for i in "${!arr}"; do echo "$i"; done
			echo ''
		fi
	done

	unset arr key meta_keys i
	echo "%SETVARS%"
	compgen -A variable
}

# is it a split pkgbuild ?
if [[ "${#pkgname[@]}" -gt 1 ]]; then
	pkgbase=${pkgbase:-${pkgname[0]}}
	_namcap_pkgnames=("${pkgname[@]}")
	unset pkgname
	echo -e "%SPLIT%\n1\n"
	echo -e "%BASE%\n${pkgbase}\n"
	echo "%NAMES%"
	for i in "${_namcap_pkgnames[@]}"; do echo "$i"; done
	echo ''
	pkginfo
	# Print per package information
	for _namcap_subpkg in "${_namcap_pkgnames[@]}"; do
		echo -e '\0'
		pkgname=$_namcap_subpkg
		package_"$_namcap_subpkg"
		pkginfo
		# Did the function actually exist?
		echo "%PKGFUNCTION%"
		type -t package_"$_namcap_subpkg" || echo undefined
		echo ''
	done
else
	pkginfo
fi
