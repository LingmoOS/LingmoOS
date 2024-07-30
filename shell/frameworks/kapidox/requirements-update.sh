#!/usr/bin/env bash

# Update `requirements.frozen.txt`. It works by installing the kapidox package with all current dependencies in a fresh
# virtualenv and then removing some unwanted fluff. The new frozen requirements contain the newest versions of all
# dependencies. If for any reason, ranged version restrictions need to be introduced, this should be done in
# `install_requires` in `setup.py`.

set -ue

if ! [ -d "kapidox" ]; then
  echo "FATAL: this must be executed in the root directory of the kapidox project"
  exit 1
fi

VENV=.kapidox_upgrade_venv
FROZEN_REQS=requirements.frozen.txt

rm -rf $VENV

python3 -m venv $VENV
$VENV/bin/pip install --upgrade pip wheel
$VENV/bin/pip install --editable .
$VENV/bin/pip freeze > $FROZEN_REQS

sed -i '/kapidox/d' $FROZEN_REQS  # filter out own package as dependency
# long standing ubuntu bug - see https://bugs.launchpad.net/ubuntu/+source/python-pip/+bug/1635463
sed -i '/pkg_resources/d' $FROZEN_REQS

rm -rf $VENV

echo "******************************************************************"
echo "Dependencies updated to newest versions."
echo "Please test if everything still works."
echo "******************************************************************"
