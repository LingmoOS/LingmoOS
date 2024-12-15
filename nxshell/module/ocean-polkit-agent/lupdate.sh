#!/bin/bash

lupdate ./ -ts -no-obsolete translations/ocean-polkit-agent.ts

tx push -s -b m20
