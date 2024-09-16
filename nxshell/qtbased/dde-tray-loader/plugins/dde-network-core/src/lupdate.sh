#!/bin/bash
if [ ! -d "translations/" ];then
  mkdir translations
fi
cd ./translations
rm -f dde-network-core.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete dde-network-core.ts
cd ../
