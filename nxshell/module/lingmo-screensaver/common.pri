isEmpty(PREFIX) {
    PREFIX = /usr
}

isEmpty(LIB_PATH) {
    # /usr/lib/lingmo-screensaver
    LIB_PATH = $$PREFIX/lib/lingmo-screensaver
}

isEmpty(MODULE_PATH) {
    # /usr/lib/lingmo-screensaver/modules
    MODULE_PATH = $$LIB_PATH/modules
}

isEmpty(RESOURCE_PATH) {
    # /usr/lib/lingmo-screensaver/modules
    RESOURCE_PATH = $$LIB_PATH/resources
}

isEmpty(COMPILE_ON_V23) {
    OS_VERSION_FILE = /etc/os-version
    COMPILE_ON_V23 = true
    !exists($$OS_VERSION_FILE): error(\"$$OS_VERSION_FILE\" is not exists. Install \"lingmo-desktop-base\" first)
    
    # 如果是V20，则设置COMPILE_ON_V23为false
    infile($$OS_VERSION_FILE, MajorVersion, 20) {
        COMPILE_ON_V23 = false
        message("COMPILE ON V20")
    } else {
        DEFINES += COMPILE_ON_V23
        message("COMPILE ON V23 OR LATER")
    }
}
