isEmpty(PREFIX) {
    PREFIX = /usr
}

isEmpty(LIB_PATH) {
    # /usr/lib/deepin-screensaver
    LIB_PATH = $$PREFIX/lib/deepin-screensaver
}

isEmpty(MODULE_PATH) {
    # /usr/lib/deepin-screensaver/modules
    MODULE_PATH = $$LIB_PATH/modules
}

isEmpty(RESOURCE_PATH) {
    # /usr/lib/deepin-screensaver/modules
    RESOURCE_PATH = $$LIB_PATH/resources
}

isEmpty(COMPILE_ON_V23) {
    OS_VERSION_FILE = /etc/os-version
    COMPILE_ON_V23 = false
    !exists($$OS_VERSION_FILE): error(\"$$OS_VERSION_FILE\" is not exists. Install \"deepin-desktop-base\" first)
    infile($$OS_VERSION_FILE, MajorVersion, 23) {
        COMPILE_ON_V23 = true
        DEFINES += COMPILE_ON_V23
        message("COMPILE ON V23")
    }
}
