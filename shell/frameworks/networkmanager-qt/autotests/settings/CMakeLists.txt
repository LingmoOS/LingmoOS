macro(NETWORKMANAGERQT_AUTOTESTS)
    foreach(_testname ${ARGN})
        ecm_add_test(${_testname}.cpp LINK_LIBRARIES Qt6::Test KF6NetworkManagerQt_static)
    endforeach(_testname)
endmacro(NETWORKMANAGERQT_AUTOTESTS)

NETWORKMANAGERQT_AUTOTESTS(
    8021xsettingtest
    adslsettingtest
    bluetoothsettingtest
    bondsettingtest
    bridgesettingtest
    bridgeportsettingtest
    cdmasettingtest
    connectionsettingtest
    dcbsettingtest
    gsmsettingtest
    infinibandsettingtest
    iptunnelsettingtest
    ipv4settingtest
    ipv6settingtest
    matchsettingtest
    macsecsettingtest
    olpcmeshsettingtest
    ovsbridgesettingtest
    ovsinterfacesettingtest
    ovspatchsettingtest
    ovsportsettingtest
    pppsettingtest
    pppoesettingtest
    proxysettingtest
    serialsettingtest
    teamportsettingtest
    tunsettingtest
    tcsettingtest
    usersettingtest
    vlansettingtest
    vxlansettingtest
    vpnsettingtest
    wimaxsettingtest
    wiredsettingtest
    wireguardsettingtest
    wirelesssettingtest
    wirelesssecuritysettingtest
)

