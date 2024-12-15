#!/bin/bash
cp ".transifexrc" ${HOME}/

lupdate ./ -ts -no-obsolete translations/ocean-dock.ts
#lupdate ./ -ts -no-obsolete plugins/oceanui-dock-plugin/translations/oceanui-dock-plugin.ts

tx push -s --branch m23
