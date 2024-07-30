#!/usr/bin/env bash

set -ue

if ! [ -d "kapidox" ]; then
  echo "FATAL: this must be executed in the root directory of the kapidox project"
  exit 1
fi

VENV=.kapidox_venv
rm -rf $VENV
python3 -m venv $VENV
$VENV/bin/pip install --upgrade pip wheel
$VENV/bin/pip install -r requirements.frozen.txt
$VENV/bin/pip install --no-deps --editable .

echo "******************************************************************"
echo "activate the development venv by running"
echo "    source $VENV/bin/activate"
echo "******************************************************************"
