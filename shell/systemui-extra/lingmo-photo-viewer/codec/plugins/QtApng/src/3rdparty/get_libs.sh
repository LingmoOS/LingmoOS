#!/bin/bash
# $1 zlib version
# $2 libpng version
set -e

ZLIB_VERSION=$1
LIBPNG_VERSION=$2

cd $(dirname $0)
rm -rf zlib/src libpng/src

# get zlib
curl -Lo zlib.tar.xz "https://www.zlib.net/zlib-${ZLIB_VERSION}.tar.xz"
tar -xf zlib.tar.xz
rm zlib.tar.xz
mv zlib-${ZLIB_VERSION} zlib/src

# get libpng
curl -Lo libpng.tar.xz "https://download.sourceforge.net/libpng/libpng-${LIBPNG_VERSION}.tar.xz"
tar xf libpng.tar.xz
rm libpng.tar.xz
mv libpng-${LIBPNG_VERSION} libpng/src

# get and apply apng patch
curl -Lo libpng-apng.patch.gz "https://downloads.sourceforge.net/sourceforge/libpng-apng/libpng-${LIBPNG_VERSION}-apng.patch.gz"
gunzip libpng-apng.patch.gz
pushd libpng/src
patch -Np1 -i "../../libpng-apng.patch"
cp scripts/pnglibconf.h.prebuilt pnglibconf.h
popd
rm libpng-apng.patch
