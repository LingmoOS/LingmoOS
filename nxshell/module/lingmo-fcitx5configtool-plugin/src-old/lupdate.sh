#!/bin/bash
dir="src/oceanui-module/translations/"
if [ ! -d $dir ];then
  mkdir $dir
fi

pushd $dir
rm -f lingmo-fcitx5configtool-plugin.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete lingmo-fcitx5configtool-plugin.ts
popd
