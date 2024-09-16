#!/bin/bash
cp ".transifexrc" ${HOME}/

#rm ./translations/*.ts

lupdate ./src/ -ts -no-obsolete translations/dde-grand-search.ts

#tx push -s -b m20
