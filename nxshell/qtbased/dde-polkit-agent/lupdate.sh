#!/bin/bash

lupdate ./ -ts -no-obsolete translations/dde-polkit-agent.ts

tx push -s -b m20
