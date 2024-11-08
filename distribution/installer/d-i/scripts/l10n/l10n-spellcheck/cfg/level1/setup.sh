#!/bin/bash

# *** l10n-spellcheck.sh ***
export LOCAL_REPOSITORY="${HOME}/tmp/spellcheck/level1"
export REFRESH_CMD=""
export OUT_DIR="/srv/d-i.debian.org/www/l10n-spellcheck/level1/"

# remove ${ALL_THESE_VARIABLES} which do not need to be spell checked
export REMOVE_VARS="yes"
export ASPELL_EXTRA_PARAM=

export PLOT_TITLE="Statistics for the level1"

export HANDLE_SUSPECT_VARS="yes"
