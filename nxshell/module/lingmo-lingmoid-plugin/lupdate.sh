#!/bin/bash
if [ ! -d "translations/" ]; then
	mkdir translations
fi
cd ./translations
rm -f oceanui-lingmoid-plugin.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete oceanui-lingmoid-plugin.ts
sed -e 's/DCC_NAMESPACE/oceanuiV20/g' oceanui-lingmoid-plugin.ts >tmp.ts
mv tmp.ts oceanui-lingmoid-plugin.ts
cd ../
tx push -s --branch m23
