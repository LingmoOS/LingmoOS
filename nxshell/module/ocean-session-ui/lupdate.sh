#!/bin/bash
lupdate ./ -ts -no-obsolete ./translations/ocean-session-ui.ts
tx push -s -b m23
