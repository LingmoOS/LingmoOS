#!/bin/bash
mkdir -p image-blur

arch=$(uname -m)
if [ x$arch = x"mips64" ] || [ x$arch = x"sw_64" ];then
    for inname in $(ls ./lingmo)
    do
        picname=${inname//jpg/bmp}
        convert -resize 1920x1080 ./lingmo/$inname ./lingmo/$picname 
        rm -rf ./lingmo/$inname
    done
fi

for inname in ./lingmo/*
do
	inname=${inname##*/}
	md5name=`echo -n /usr/share/wallpapers/lingmo/$inname | md5sum`
	md5name=${md5name%  *}
    if [ x$arch = x"mips64" ] || [ x$arch = x"sw_64" ];then
	    outname=$md5name.bmp
    else
	    outname=$md5name.jpg
    fi
	/usr/lib/lingmo-api/image-blur --sigma 30 ./lingmo/$inname ./image-blur/$outname
done
