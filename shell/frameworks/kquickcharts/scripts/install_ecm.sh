#!/bin/bash

git clone https://invent.kde.org/frameworks/extra-cmake-modules
cd extra-cmake-modules
cmake . -DCMAKE_INSTALL_PREFIX=../ecm -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF
make install
cd ..
