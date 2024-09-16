#!/bin/bash
if [ ! -d "translations/" ];then
  mkdir translations
fi
cd ./translations
rm -f dss-network-plugin_en_US.ts
lupdate ../ ../../src ../../common-plugin -ts -no-ui-lines -locations none -no-obsolete dss-network-plugin_en_US.ts
cd ../

tx push -s --branch m23
