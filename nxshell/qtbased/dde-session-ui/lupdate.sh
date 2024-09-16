#!/bin/bash
lupdate ./ -ts -no-obsolete ./translations/dde-session-ui.ts
tx push -s -b m23
