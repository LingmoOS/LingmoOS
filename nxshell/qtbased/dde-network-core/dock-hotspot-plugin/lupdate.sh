#!/bin/bash
if [ ! -d "translations/" ];then
  mkdir translations
fi
cd ./translations
rm -f dock-hotspot-plugin_en_US.ts
lupdate ../ ../../src ../../common-plugin -ts -no-ui-lines -locations none -no-obsolete dock-hotspot-plugin_en_US.ts
cd ../

tx push -s --branch m23
