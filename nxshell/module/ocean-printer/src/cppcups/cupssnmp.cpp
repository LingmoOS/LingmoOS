// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cupssnmp.h"
#include "cupsppd.h"
#include "mibpath.h"
#include "snmp.h"
#include "cupsmodule.h"

#include <stdlib.h>
#include <strings.h>
#include <stdexcept>

#define CUPS_MAX_SUPPLIES   32  /* Maximum number of supplies for a printer */
#define CUPS_SUPPLY_TIMEOUT 2.0 /* Timeout for SNMP lookups */

#define CUPS_DEVELOPER_LOW  0x0001
#define CUPS_DEVELOPER_EMPTY    0x0002
#define CUPS_MARKER_SUPPLY_LOW  0x0004
#define CUPS_MARKER_SUPPLY_EMPTY 0x0008
#define CUPS_OPC_NEAR_EOL   0x0010
#define CUPS_OPC_LIFE_OVER  0x0020
#define CUPS_TONER_LOW      0x0040
#define CUPS_TONER_EMPTY    0x0080
#define CUPS_WASTE_ALMOST_FULL  0x0100
#define CUPS_WASTE_FULL     0x0200
#define CUPS_CLEANER_NEAR_EOL   0x0400  /* Proposed JPS3 */
#define CUPS_CLEANER_LIFE_OVER  0x0800  /* Proposed JPS3 */

#define CUPS_SNMP_NONE      0x0000
#define CUPS_SNMP_CAPACITY  0x0001  /* Supply levels reported as percentages */

static const int    hrDeviceDescr[] =
{ CUPS_OID_hrDeviceDescr, 1, -1 };
//Device description OID
static const int    hrPrinterStatus[] =
{ CUPS_OID_hrPrinterStatus, 1, -1 };
//Current state OID
static const int    hrPrinterDetectedErrorState[] =
{ CUPS_OID_hrPrinterDetectedErrorState, 1, -1 };
// Current printer state bits OID
static const int    prtGeneralCurrentLocalization[] =
{ CUPS_OID_prtGeneralCurrentLocalization, 1, -1 };
static const int    prtLocalizationCharacterSet[] =
{ CUPS_OID_prtLocalizationCharacterSet, 1, 1, -1 },
prtLocalizationCharacterSetOffset =
    (sizeof(prtLocalizationCharacterSet) /
     sizeof(prtLocalizationCharacterSet[0]));
static const int    prtMarkerColorantValue[] =
{ CUPS_OID_prtMarkerColorantValue, -1 },
// Colorant OID
prtMarkerColorantValueOffset =
    (sizeof(prtMarkerColorantValue) /
     sizeof(prtMarkerColorantValue[0]));
// Offset to colorant index
static const int    prtMarkerLifeCount[] =
{ CUPS_OID_prtMarkerLifeCount, 1, 1, -1 };
// Page counter OID
static const int    prtMarkerSuppliesEntry[] =
{ CUPS_OID_prtMarkerSuppliesEntry, -1 };
// Supplies OID
static const int    prtMarkerSuppliesColorantIndex[] =
{ CUPS_OID_prtMarkerSuppliesColorantIndex, -1 },
// Colorant index OID
prtMarkerSuppliesColorantIndexOffset =
    (sizeof(prtMarkerSuppliesColorantIndex) /
     sizeof(prtMarkerSuppliesColorantIndex[0]));
// Offset to supply index
static const int    prtMarkerSuppliesDescription[] =
{ CUPS_OID_prtMarkerSuppliesDescription, -1 },
// Description OID
prtMarkerSuppliesDescriptionOffset =
    (sizeof(prtMarkerSuppliesDescription) /
     sizeof(prtMarkerSuppliesDescription[0]));
// Offset to supply index
static const int    prtMarkerSuppliesLevel[] =
{ CUPS_OID_prtMarkerSuppliesLevel, -1 },
// Level OID
prtMarkerSuppliesLevelOffset =
    (sizeof(prtMarkerSuppliesLevel) /
     sizeof(prtMarkerSuppliesLevel[0]));
// Offset to supply index
static const int    prtMarkerSuppliesMaxCapacity[] =
{ CUPS_OID_prtMarkerSuppliesMaxCapacity, -1 },
// Max capacity OID
prtMarkerSuppliesMaxCapacityOffset =
    (sizeof(prtMarkerSuppliesMaxCapacity) /
     sizeof(prtMarkerSuppliesMaxCapacity[0]));
// Offset to supply index
static const int    prtMarkerSuppliesClass[] =
{ CUPS_OID_prtMarkerSuppliesClass, -1 },
// Class OID
prtMarkerSuppliesClassOffset =
    (sizeof(prtMarkerSuppliesClass) /
     sizeof(prtMarkerSuppliesClass[0]));
// Offset to supply index
static const int    prtMarkerSuppliesType[] =
{ CUPS_OID_prtMarkerSuppliesType, -1 },
// Type OID
prtMarkerSuppliesTypeOffset =
    (sizeof(prtMarkerSuppliesType) /
     sizeof(prtMarkerSuppliesType[0]));
// Offset to supply index
static const int    prtMarkerSuppliesSupplyUnit[] =
{ CUPS_OID_prtMarkerSuppliesSupplyUnit, -1 },
// Units OID
prtMarkerSuppliesSupplyUnitOffset =
    (sizeof(prtMarkerSuppliesSupplyUnit) /
     sizeof(prtMarkerSuppliesSupplyUnit[0]));
// Offset to supply index

static int g_iNumSupply = -1;
static unsigned int g_iQuirks = CUPS_SNMP_NONE;
static SUPPLYSDATA g_supplies[CUPS_MAX_SUPPLIES];
static void backend_walk_cb(cups_snmp_t *packet, void *pData);

cupssnmp::cupssnmp()
{
    m_iFd = -1;
    m_bSNMPSupport = false;
    m_pHost = nullptr;
}

cupssnmp::~cupssnmp()
{
    /*http_addrlist_t是个链表结构*/
    if (m_pHost) {
        http_addrlist_t *pTempNode = nullptr;
        while (m_pHost->next != nullptr) {
            pTempNode = m_pHost->next;
            free(m_pHost);
            m_pHost = pTempNode;
        }
        free(m_pHost);
        m_pHost = nullptr;
    }
}

void cupssnmp::setIP(const string &strip)
{
    m_strip = strip;
}

void cupssnmp::setPPDName(const string &strppd)
{
    m_strPPD = strppd;
}

bool cupssnmp::SNMPOpen()
{
#ifdef AF_INET6_ENV
    m_iFd = _cupsSNMPOpen(AF_INET6);
#else
    m_iFd = _cupsSNMPOpen(AF_INET);
#endif

    return (m_iFd >= 0);
}

bool cupssnmp::SNMPReadSupplies()
{
    bool bRet = false;

    if (!SNMPOpen()) {
        return false;
    }

    SNMPInit();

    if (m_bSNMPSupport) {
        bRet = true;
    } else {
        bRet = false;
    }

    if (bRet) {
        SNMPWalk();
    }

    SNMPClose();
    return bRet;
}

vector<SUPPLYSDATA> cupssnmp::getMarkInfo()
{
    vector<SUPPLYSDATA> vecInfo;

    for (int i = 0; i < g_iNumSupply; i++) {
        if (g_supplies[i].type == 3 ||
                g_supplies[i].type == 4) {
            vecInfo.push_back(g_supplies[i]);
        }
    }

    return  vecInfo;
}

bool cupssnmp::SNMPSupport()
{
    bool bRet = false;

    try {
        PPD ppd;
        ppd.load(m_strPPD.c_str());

        Attribute attr = ppd.findAttr("cupsSNMPSupplies", nullptr);
        string strAttrValue = attr.getValue();

        if (strAttrValue.empty() || !strncmp(strAttrValue.c_str(), "true", strlen("true"))) {
            bRet = true;
        } else {
            bRet = false;
        }
    } catch (const std::runtime_error &e) {
        debugprintf("runtime_error: %s\n", e.what());
        bRet = false;
    }

    return bRet;
}

void cupssnmp::SNMPClose()
{
    _cupsSNMPClose(m_iFd);
}

void cupssnmp::SNMPWalk()
{
    memset(g_supplies, 0, sizeof(SUPPLYSDATA) * CUPS_MAX_SUPPLIES);
    g_iNumSupply = -1;

    char *snmpCommunity = _cupsSNMPDefaultCommunity();
    int iRet = _cupsSNMPWalk(m_iFd, &m_pHost->addr, CUPS_SNMP_VERSION_1,
                             snmpCommunity, prtMarkerSuppliesEntry,
                             CUPS_SUPPLY_TIMEOUT, backend_walk_cb, nullptr);

    if (iRet < 0) {
        g_iNumSupply = -1;
        free(snmpCommunity);
        return;
    }

    _cupsSNMPWalk(m_iFd, &m_pHost->addr, CUPS_SNMP_VERSION_1,
                  snmpCommunity, prtMarkerColorantValue,
                  CUPS_SUPPLY_TIMEOUT, backend_walk_cb, nullptr);
    free(snmpCommunity);

}

void cupssnmp::SNMPInit()
{
    cups_snmp_t packet;         /* SNMP response packet */
    g_iNumSupply  = -1;

    /*
    * See if we should be getting supply levels via SNMP...
    */

    if (!SNMPSupport()) {
        m_bSNMPSupport = false;
        return ;
    } else {
        m_bSNMPSupport = true;
    }

    QuirksInit();

    /*
    * Get the device description...
    */

    if (!m_pHost) {
        m_pHost = (http_addrlist_s *)calloc(sizeof(http_addrlist_t), 1);
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(m_strip.c_str());
    memcpy(&(m_pHost->addr), (void *)&addr,
           sizeof(struct sockaddr_in));

    char *snmpCommunity = _cupsSNMPDefaultCommunity();
    if (!_cupsSNMPWrite(m_iFd, &m_pHost->addr, CUPS_SNMP_VERSION_1,
                        snmpCommunity, CUPS_ASN1_GET_REQUEST, 1,
                        hrDeviceDescr)) {
        m_bSNMPSupport = false;
        free(snmpCommunity);
        return;
    }

    if (!_cupsSNMPRead(m_iFd, &packet, CUPS_SUPPLY_TIMEOUT) ||
            packet.object_type != CUPS_ASN1_OCTET_STRING) {
        g_iNumSupply = 0;
        m_bSNMPSupport = false;
        free(snmpCommunity);
        return;
    }

    free(snmpCommunity);
}

void cupssnmp::QuirksInit()
{
    PPD ppd;
    ppd.load(m_strPPD.c_str());
    Attribute attr = ppd.findAttr("cupsSNMPQuirks", nullptr);
    string strAttrValue = attr.getValue();
    g_iQuirks = CUPS_SNMP_NONE;

    if (!strAttrValue.empty() && (!strncmp(strAttrValue.c_str(), "capacity", strlen("capacity")))) {
        g_iQuirks |= CUPS_SNMP_CAPACITY;
    }
}

void backend_walk_cb(cups_snmp_t *packet, void *pData)
{
    static const char *const colors[][2] = {
        /* Standard color names */
        { "Black",         "#000000" },
        { "Blue",          "#0000FF" },
        { "Brown",         "#A52A2A" },
        { "Cyan",          "#00FFFF" },
        { "Dark-gray",     "#404040" },
        { "Dark gray",     "#404040" },
        { "Dark-yellow",   "#FFCC00" },
        { "Dark yellow",   "#FFCC00" },
        { "Gold",          "#FFD700" },
        { "Gray",          "#808080" },
        { "Green",         "#00FF00" },
        { "Light-black",   "#606060" },
        { "Light black",   "#606060" },
        { "Light-cyan",    "#E0FFFF" },
        { "Light cyan",    "#E0FFFF" },
        { "Light-gray",    "#D3D3D3" },
        { "Light gray",    "#D3D3D3" },
        { "Light-magenta", "#FF77FF" },
        { "Light magenta", "#FF77FF" },
        { "Magenta",       "#FF00FF" },
        { "Orange",        "#FFA500" },
        { "Red",           "#FF0000" },
        { "Silver",        "#C0C0C0" },
        { "White",         "#FFFFFF" },
        { "Yellow",        "#FFFF00" }
    };

    (void)pData;
    int i = -1;

    if (_cupsSNMPIsOIDPrefixed(packet, prtMarkerColorantValue) &&
            packet->object_type == CUPS_ASN1_OCTET_STRING) {
        /*
        * Get colorant...
        */

        i = packet->object_name[prtMarkerColorantValueOffset];

        for (int j = 0; j < g_iNumSupply; j ++)
            if (g_supplies[j].colorant == i) {
                for (int k = 0; k < (int)(sizeof(colors) / sizeof(colors[0])); k ++)

                    //此處要改爲不分大小寫
                    if (!strcasecmp((const char *)colors[k][0], (const char *)packet->object_value.string.bytes)) {
                        strncpy(g_supplies[j].colorName, colors[k][0], sizeof(g_supplies[j].colorName));
                        strncpy(g_supplies[j].color, colors[k][1], sizeof(g_supplies[j].color));
                        break;
                    }
            }
    } else if (_cupsSNMPIsOIDPrefixed(packet, prtMarkerSuppliesColorantIndex)) {
        /*
        * Get colorant index...
        */

        i = packet->object_name[prtMarkerSuppliesColorantIndexOffset];

        if (i < 1 || i > CUPS_MAX_SUPPLIES ||
                packet->object_type != CUPS_ASN1_INTEGER)
            return;

        if (i > g_iNumSupply)
            g_iNumSupply = i;

        g_supplies[i - 1].colorant = packet->object_value.integer;
    } else if (_cupsSNMPIsOIDPrefixed(packet, prtMarkerSuppliesDescription)) {
        /*
        * Get supply name/description...
        */

        i = packet->object_name[prtMarkerSuppliesDescriptionOffset];

        if (i < 1 || i > CUPS_MAX_SUPPLIES ||
                packet->object_type != CUPS_ASN1_OCTET_STRING)
            return;

        if (i > g_iNumSupply)
            g_iNumSupply = i;

        char    *src, *dst; /* Pointers into strings */

        for (src = (char *)packet->object_value.string.bytes,
                dst = g_supplies[i - 1].name;
                *src;
                src ++) {
            if ((*src & 0x80) || *src < ' ' || *src == 0x7f)
                *dst++ = '?';
            else
                *dst++ = *src;
        }

        *dst = '\0';
    } else if (_cupsSNMPIsOIDPrefixed(packet, prtMarkerSuppliesLevel)) {
        /*
        * Get level...
        */

        i = packet->object_name[prtMarkerSuppliesLevelOffset];
        if (i < 1 || i > CUPS_MAX_SUPPLIES ||
                packet->object_type != CUPS_ASN1_INTEGER)
            return;

        if (i > g_iNumSupply)
            g_iNumSupply = i;

        g_supplies[i - 1].level = packet->object_value.integer;
    } else if (_cupsSNMPIsOIDPrefixed(packet, prtMarkerSuppliesMaxCapacity) &&
               !(g_iQuirks & CUPS_SNMP_CAPACITY)) {
        /*
        * Get max capacity...
        */

        i = packet->object_name[prtMarkerSuppliesMaxCapacityOffset];

        if (i < 1 || i > CUPS_MAX_SUPPLIES ||
                packet->object_type != CUPS_ASN1_INTEGER)
            return;

        /*
        fprintf(stderr, "DEBUG2: prtMarkerSuppliesMaxCapacity.1.%d = %d\n", i,
                packet->object_value.integer);
        */


        if (i > g_iNumSupply)
            g_iNumSupply = i;

        if (g_supplies[i - 1].max_capacity == 0 && packet->object_value.integer > 0)
            g_supplies[i - 1].max_capacity = packet->object_value.integer;
    } else if (_cupsSNMPIsOIDPrefixed(packet, prtMarkerSuppliesType)) {
        /*
        * Get marker type...
        */

        i = packet->object_name[prtMarkerSuppliesTypeOffset];

        if (i < 1 || i > CUPS_MAX_SUPPLIES ||
                packet->object_type != CUPS_ASN1_INTEGER)
            return;

        /*
        fprintf(stderr, "DEBUG2: prtMarkerSuppliesType.1.%d = %d\n", i,
                packet->object_value.integer);
        */

        if (i > g_iNumSupply)
            g_iNumSupply = i;

        g_supplies[i - 1].type = packet->object_value.integer;
    } else if (_cupsSNMPIsOIDPrefixed(packet, prtMarkerSuppliesSupplyUnit)) {
        /*
        * Get units for capacity...
        */

        i = packet->object_name[prtMarkerSuppliesSupplyUnitOffset];

        if (i < 1 || i > CUPS_MAX_SUPPLIES || packet->object_type != CUPS_ASN1_INTEGER)
            return;

        /*
        fprintf(stderr, "DEBUG2: prtMarkerSuppliesSupplyUnit.1.%d = %d\n", i,
                packet->object_value.integer);
        */

        if (i > g_iNumSupply)
            g_iNumSupply = i;

        if (packet->object_value.integer == CUPS_TC_percent)
            g_supplies[i - 1].max_capacity = 100;
    }
}
