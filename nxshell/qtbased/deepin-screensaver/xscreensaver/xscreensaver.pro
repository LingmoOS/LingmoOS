QT -= gui
CONFIG -= qt
TEMPLATE = aux

include(../common.pri)

cover.files += $$PWD/cover/*
cover.path = $$MODULE_PATH/cover

screensaver_list = blaster bouboule cage discoball dymaxionmap endgame epicycle \
    flow flurry flyingtoasters fuzzyflakes galaxy glplanet hexadrop \
    klein lament lockward polytopes vermiculate

isEmpty(XSCREENSAVER_DATA_PATH) {
    if($$COMPILE_ON_V23) {
        XSCREENSAVER_DATA_PATH = $$PREFIX/libexec/xscreensaver
    } else {
        XSCREENSAVER_DATA_PATH = $$PREFIX/lib/xscreensaver
    }
}

message("XSCREENSAVER_DATA_PATH:$$XSCREENSAVER_DATA_PATH")

screensaver.path = $$MODULE_PATH

for(ss, screensaver_list) {
    screensaver.extra += ln -s $$XSCREENSAVER_DATA_PATH/$$ss $(INSTALL_ROOT)$$screensaver.path/$$ss;
}

INSTALLS += cover screensaver
