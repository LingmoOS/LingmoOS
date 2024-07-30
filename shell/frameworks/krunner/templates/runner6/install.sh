#!/bin/bash

# Exit if something fails
set -e

mkdir -p build
cd build

cmake -DKDE_INSTALL_USE_QT_SYS_PATHS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo  ..
make -j$(nproc)
sudo make install

# KRunner needs to be restarted for the changes to be applied
# we can just kill it and it will be started when the shortcut is invoked
kquitapp6 krunner
