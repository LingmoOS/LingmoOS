#!/bin/bash
if [ ! -d "translations/" ];then
  mkdir translations
fi
cd ./translations
rm -f ocean-network-core.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete ocean-network-core.ts
cd ../
