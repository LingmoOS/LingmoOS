/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "common.h"
#include "lingmo-log4qt.h"



QStringList list_errmsg =
{
    "",
    "br-connection-already-connected", //1
    "br-connection-page-timeout",
    "br-connection-profile-unavailable",
    "br-connection-sdp-search",
    "br-connection-create-socket",
    "br-connection-invalid-argument",
    "br-connection-adapter-not-powered",
    "br-connection-not-suuported",
    "br-connection-bad-socket",
    "br-connection-memory-allocation",
    "br-connection-busy",
    "br-connection-concurrent-connection-limit",
    "br-connection-timeout",
    "br-connection-refused",
    "br-connection-aborted-by-remote",
    "br-connection-aborted-by-local",
    "br-connection-lmp-protocol-error",
    "br-connection-canceled",
    "br-connection-unknown",

    /////////////////////////////////
    "le-connection-invalid-arguments", //20
    "le-connection-adapter-not-powered",
    "le-connection-not-supported",
    "le-connection-already-connected",
    "le-connection-bad-socket",
    "le-connection-memory-allocation",
    "le-connection-busy",
    "le-connection-refused",
    "le-connection-create-socket",
    "le-connection-timeout",
    "le-connection-concurrent-connection-limit",
    "le-connection-abort-by-remote",
    "le-connection-abort-by-local",
    "le-connection-link-layer-protocol-error",
    "le-connection-gatt-browsing",
    "le-connection-unknown",

    ////////////////////////////////////////////
    "Invalid arguments in method call",  //36
    "Operation already in progress",
    "Already Exists",
    "Operation is not supported",
    "Already Connected",
    "Operation currently not available",
    "Does Not Exist",
    "Not Connected",
    "In Progress",
    "Operation Not Authorized",
    "No such adapter",
    "Agent Not Available",
    "Resource Not Ready",

    ////////////////////////////////////////////////
    "Did not receive a reply. Possible causes include: "\
    "the remote application did not send a reply, "\
    "the message bus security policy blocked the reply, "\
    "the reply timeout expired, or the network connection was broken.",


/************上面错误码为bluez返回错误码****************/
    "ERR_BREDR_INTERNAL_NO_Default_Adapter",
    "ERR_BREDR_INTERNAL_Operation_Progress",
    "ERR_BREDR_INTERNAL_Already_Connected",
    "ERR_BREDR_INTERNAL_Dev_Not_Exist",
    "ERR_BREDR_UNKNOWN_Other",
    /////////////////////////////////////////////////

};


QStringList Adapter_attr
{
    "Pairing",
    "Connecting",
    "Powered",
    "Discoverable",
    "Pairable",
    "Discovering",
    "Name",
    "Block",
    "Addr",
    "ActiveConnection",
    "DefaultAdapter",
    "trayShow",
    "FileTransportSupport",
    "ClearPinCode",
};

QStringList Device_attr
{
    "Paired",
    "Trusted",
    "Blocked",
    "Connected",
    "Name",
    "Type",
    "TypeName",
    "Pairing",
    "Connecting",
    "Battery",
    "ConnectFailedId",
    "ConnectFailedDisc",
    "Rssi",
    "Addr",
    "FileTransportSupport",
    "ShowName",
    "Adapter",
};

QStringList app_attr
{
    "dbusid",
    "username",
    "type",
    "pid",
};

QStringList startpair_attr
{
    "dev",
    "name",
    "pincode",
    "type",
};

QStringList filestatus_attr
{
    "status",
    "progress",
    "filename",
    "allfilenum",
    "fileseq",
    "dev",
    "fileFailedDisc",
    "filetype",
    "transportType",
    "savepath",
    "filesize",
};

QStringList receivefile_attr
{
    "dev",
    "name",
    "filetype",
    "filename",
    "filesize",
};


bool support_a2dp_sink(const QStringList & uuids)
{
    if(-1 != uuids.indexOf(A2DP_SINK_UUID) || -1 != uuids.indexOf(QString(A2DP_SINK_UUID).toUpper()))
    {
        return true;
    }
    return false;
}

bool support_a2dp_source(const QStringList & uuids)
{
    if(-1 != uuids.indexOf(A2DP_SOURCE_UUID) || -1 != uuids.indexOf(QString(A2DP_SOURCE_UUID).toUpper()))
    {
        return true;
    }
    return false;
}

bool support_hfphsp_ag(const QStringList & uuids)
{
    if(-1 != uuids.indexOf(HFP_AG_UUID) || -1 != uuids.indexOf(QString(HFP_AG_UUID).toUpper()) ||
       -1 != uuids.indexOf(HSP_AG_UUID) || -1 != uuids.indexOf(QString(HSP_AG_UUID).toUpper()))
    {
        return true;
    }
    return false;
}

bool support_hfphsp_hf(const QStringList & uuids)
{
    if(-1 != uuids.indexOf(HFP_HS_UUID) || -1 != uuids.indexOf(QString(HFP_HS_UUID).toUpper()) ||
       -1 != uuids.indexOf(HSP_HS_UUID) || -1 != uuids.indexOf(QString(HSP_HS_UUID).toUpper()))
    {
        return true;
    }
    return false;
}

static QStringList static_filetransport_uuids
{
    //OBEX_SYNC_UUID,
    OBEX_OPP_UUID,
    OBEX_FTP_UUID,
    /*
    OBEX_PCE_UUID,
    OBEX_PSE_UUID,
    OBEX_PBAP_UUID,
    OBEX_MAS_UUID,
    OBEX_MNS_UUID,
    OBEX_MAP_UUID,
    */
};

bool support_filetransport(const QStringList & uuids)
{
    for(auto key : static_filetransport_uuids)
    {
        if(-1 != uuids.indexOf(key) || -1 != uuids.indexOf(key.toUpper()))
        {
            return true;
        }
    }
    return false;
}
