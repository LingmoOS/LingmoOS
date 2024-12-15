#!/bin/bash
if [ ! -d "translations/" ];then
  mkdir translations
fi
cd ./translations
rm -f network-service-plugin.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete network-service-plugin.ts
cd ../
