#!/bin/bash

for pkg in *.lpk; do
    echo "Signing: $pkg"

    gpg --detach-sign -o "$pkg".sig -a "$pkg"
    if [ $? -eq 0 ]; then
        echo "Sign: $pkg"
    else
        echo "Error: $pkg"
    fi
done
