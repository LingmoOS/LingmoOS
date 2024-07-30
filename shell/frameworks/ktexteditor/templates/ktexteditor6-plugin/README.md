# Readme file

## Introduction

This is a simple Kate plugin right now. Make it do something :)


## Installation instructions

Define some testing install dir:

    export MYKATEPLUGINPATH=$HOME/mykateplugins

Build and install:

    mkdir build
    cd build
    cmake .. -DKDE_INSTALL_PLUGINDIR=$MYKATEPLUGINPATH
    make
    make install


## Check the plugin

Start Kate on the commandline, with adapted QT_PLUGIN_PATH:

    export QT_PLUGIN_PATH=$MYKATEPLUGINPATH:$QT_PLUGIN_PATH
    kate --startanon

Go to Settings / Configurate Kate / Application / Plugins
Search and enable your plugin


## Help

https://api.kde.org/frameworks/ktexteditor/html/classKTextEditor_1_1Plugin.html#plugin_intro

For help or more information, contact the Kate team
