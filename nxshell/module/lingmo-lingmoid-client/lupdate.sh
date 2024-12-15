#!/bin/bash
if [ ! -d "translations/" ];then
  mkdir translations
fi
cd ./translations
rm -f lingmo-lingmoid-client.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete lingmo-lingmoid-client.ts
cd ../
