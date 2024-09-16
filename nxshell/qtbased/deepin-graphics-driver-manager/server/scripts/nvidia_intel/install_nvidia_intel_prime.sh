#!/bin/bash

/usr/lib/deepin-graphics-driver-manager/nvidia/install_nvidia_closesource.sh || exit 1

modinfo intelgpu >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
    echo "blacklist intelgpu" >> /etc/modprobe.d/nvidia-blacklists-nouveau.conf
    update-initramfs -u
fi

cat  > /etc/X11/xorg.conf.d/70-nvidia.conf <<EOF
Section "ServerLayout"
  Identifier "layout"
  #Screen 0 "iGPU"
  Option "AllowNVIDIAGPUScreens"
EndSection

Section "Device"
  Identifier "iGPU"
  Driver "modesetting"
EndSection

Section "Screen"
  Identifier "iGPU"
  Device "iGPU"
EndSection

Section "Device"
  Identifier "dGPU"
  Driver "nvidia"
EndSection
EOF

cat  > /etc/lightdm/display_setup.sh <<EOF
#!/bin/sh
xrandr --setprovideroffloadsink 1 0
xrandr --auto
EOF

chmod +x /etc/lightdm/display_setup.sh

sed -i '/^\[Seat\:\*\]/a\display-setup-script=\/etc\/lightdm\/display_setup.sh' /etc/lightdm/lightdm.conf

cat  > ${PRIME_GLTEST_G} <<EOF
#!/bin/sh
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia
/usr/lib/deepin-graphics-driver-manager/gltest
EOF

chmod a+x ${PRIME_GLTEST_G}