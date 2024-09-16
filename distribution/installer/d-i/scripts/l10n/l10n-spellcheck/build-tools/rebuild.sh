#!/bin/bash
# all_static.sh
#
# rebuild aspell, aspell-dictionaries using static libs and install into TOOLS_PATH

export ALIOTH_HOME=~
export TOOLS_PATH=$ALIOTH_HOME/tools_0.50.5

export ASPELL=$TOOLS_PATH/bin/aspell 
export WORD_LIST_COMPRESS=$TOOLS_PATH/bin/word-list-compress

sh aspell.sh

# ***************************
# *** aspell dictionaries ***
# ***************************

if [ ! -d ./dictionaries_pool ] ; then
    mkdir ./dictionaries_pool
fi

sh aspell-dicts.sh dictionaries.txt

sh gnuplot.sh