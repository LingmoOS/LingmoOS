#!/bin/bash
dir="src/dcc-module/translations/"
if [ ! -d $dir ];then
  mkdir $dir
fi

pushd $dir
rm -f deepin-fcitx5configtool-plugin.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete deepin-fcitx5configtool-plugin.ts
popd
