#!/bin/bash

# *** l10n-spellcheck.sh ***
export LOCAL_REPOSITORY="${HOME}/trunk/installation-guide/"
export REFRESH_CMD="git pull ${LOCAL_REPOSITORY}"
export OUT_DIR="/srv/d-i.debian.org/www/l10n-spellcheck/manual_d-i"

# *** check_dit.sh ***
export PO_FINDER="./pof_di-manual.sh"

# remove ${ALL_THESE_VARIABLES} which do not need to be spell checked
export REMOVE_VARS="no"
export ASPELL_EXTRA_PARAM="filter -H"

export PLOT_TITLE="Statistics for the d-i installation-guide"

export HANDLE_SUSPECT_VARS="no"
