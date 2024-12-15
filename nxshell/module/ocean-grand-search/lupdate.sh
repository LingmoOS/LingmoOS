#!/bin/bash
cp ".transifexrc" ${HOME}/

#rm ./translations/*.ts

lupdate ./src/ -ts -no-obsolete translations/ocean-grand-search.ts

#tx push -s -b m20
