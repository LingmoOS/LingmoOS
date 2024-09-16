#!/bin/bash

# exit 0 ------ use open nouveau driver 
# exit 1 ------ use private nvidia driver

nouveau_mod=`lsmod | grep nouveau`
nvidia_mod=`lsmod | grep nvidia`

if [ -n "$nouveau_mod" ]; then
    echo "Had already used opensource nouveau driver"
    exit 0
fi


if [ -n "$nvidia_mod" ]; then
    echo "Had already used private nvidia driver "
    exit 1
fi

exit 0
