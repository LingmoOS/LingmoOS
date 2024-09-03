#!/bin/sh
###
# Wrapper for running calamares on Debian live media
###

# Stale file left behind by live-build that messes with partitioning
sudo mv /etc/fstab /etc/fstab.orig.calamares

# Allow Calamares to scale the window for hidpi displays
# This is fixed in the Calamares 3.3.0 series, so we can remove this
# once we switch to that
# Upstream commit that will make this obsolete:
#     https://github.com/calamares/calamares/commit/e9f011b686a0982fb7828e8ac02a8e0784d3b11f
# Upstream bug:
#     https://github.com/calamares/calamares/issues/1945
# Debian bug:
#     https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=992162
export QT_AUTO_SCREEN_SCALE_FACTOR=1

# Access control to run calamares as root for xwayland
xhost +si:localuser:root
pkexec calamares
xhost -si:localuser:root

# Restore stale fstab, for what it's worth
sudo mv /etc/fstab.orig.calamares /etc/fstab
