#!/bin/bash

make clean

rm -rf lingmo-music-plugins-simple/build/ \
    lingmo-music.pro.user \
    debian/files \
    .qmake.stash \
    Makefile \
    debian/.debhelper/ \
    debian/debhelper-build-stamp \
    debian/lingmo-music.debhelper.log \
    debian/lingmo-music.substvars \
    debian/lingmo-music/ \
    lingmo-music
