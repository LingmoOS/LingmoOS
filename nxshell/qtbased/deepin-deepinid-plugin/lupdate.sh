#!/bin/bash
if [ ! -d "translations/" ]; then
	mkdir translations
fi
cd ./translations
rm -f dcc-deepinid-plugin.ts
lupdate ../ -ts -no-ui-lines -locations none -no-obsolete dcc-deepinid-plugin.ts
sed -e 's/DCC_NAMESPACE/dccV20/g' dcc-deepinid-plugin.ts >tmp.ts
mv tmp.ts dcc-deepinid-plugin.ts
cd ../
tx push -s --branch m23
