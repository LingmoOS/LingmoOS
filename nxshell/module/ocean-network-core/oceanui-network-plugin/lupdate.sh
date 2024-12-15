#!/bin/bash
if [ ! -d "translations/" ];then
  mkdir translations
fi
cd ./translations
rm -f oceanui-network-plugin_en_US.ts
lupdate ../ ../../src -ts -no-ui-lines -locations none -no-obsolete oceanui-network-plugin.ts
sed -e 's/DCC_NAMESPACE/oceanuiV23/g' oceanui-network-plugin.ts > tmp.ts
mv tmp.ts oceanui-network-plugin_en_US.ts
cd ../
tx push -s --branch m23
