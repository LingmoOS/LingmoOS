#!/bin/bash
mkdir -p image-blur

arch=$(uname -m)
if [ x$arch = x"mips64" ] || [ x$arch = x"sw_64" ];then
    for inname in $(ls ./deepin)
    do
        picname=${inname//jpg/bmp}
        convert -resize 1920x1080 ./deepin/$inname ./deepin/$picname 
        rm -rf ./deepin/$inname
    done
fi

for inname in ./deepin/*
do
	inname=${inname##*/}
	md5name=`echo -n /usr/share/wallpapers/deepin/$inname | md5sum`
	md5name=${md5name%  *}
    if [ x$arch = x"mips64" ] || [ x$arch = x"sw_64" ];then
	    outname=$md5name.bmp
    else
	    outname=$md5name.jpg
    fi
	/usr/lib/deepin-api/image-blur --sigma 30 ./deepin/$inname ./image-blur/$outname
done
