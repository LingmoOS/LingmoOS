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

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QMetaEnum>


#define A2DP_SOURCE_UUID	"0000110a-0000-1000-8000-00805f9b34fb"
#define A2DP_SINK_UUID		"0000110b-0000-1000-8000-00805f9b34fb"

#define HSP_HS_UUID		    "00001108-0000-1000-8000-00805f9b34fb"
#define HSP_AG_UUID		    "00001112-0000-1000-8000-00805f9b34fb"

#define HFP_HS_UUID		    "0000111e-0000-1000-8000-00805f9b34fb"
#define HFP_AG_UUID		    "0000111f-0000-1000-8000-00805f9b34fb"

//#define OBEX_SYNC_UUID		"00001104-0000-1000-8000-00805f9b34fb"
#define OBEX_OPP_UUID		"00001105-0000-1000-8000-00805f9b34fb"
#define OBEX_FTP_UUID		"00001106-0000-1000-8000-00805f9b34fb"
//#define OBEX_PCE_UUID		"0000112e-0000-1000-8000-00805f9b34fb"
//#define OBEX_PSE_UUID		"0000112f-0000-1000-8000-00805f9b34fb"
//#define OBEX_PBAP_UUID		"00001130-0000-1000-8000-00805f9b34fb"
//#define OBEX_MAS_UUID		"00001132-0000-1000-8000-00805f9b34fb"
//#define OBEX_MNS_UUID		"00001133-0000-1000-8000-00805f9b34fb"
//#define OBEX_MAP_UUID		"00001134-0000-1000-8000-00805f9b34fb"


#define GREATERINDEX    "greater index"

enum enum_connect_errmsg
{
    ERR_BREDR_CONN_SUC,
    ERR_BREDR_CONN_ALREADY_CONNECTED = 1,
    ERR_BREDR_CONN_PAGE_TIMEOUT,
    ERR_BREDR_CONN_PROFILE_UNAVAILABLE,
    ERR_BREDR_CONN_SDP_SEARCH,
    ERR_BREDR_CONN_CREATE_SOCKET,
    ERR_BREDR_CONN_INVALID_ARGUMENTS,
    ERR_BREDR_CONN_ADAPTER_NOT_POWERED,
    ERR_BREDR_CONN_NOT_SUPPORTED,
    ERR_BREDR_CONN_BAD_SOCKET,
    ERR_BREDR_CONN_MEMORY_ALLOC,
    ERR_BREDR_CONN_BUSY,
    ERR_BREDR_CONN_CNCR_CONNECT_LIMIT,
    ERR_BREDR_CONN_TIMEOUT,
    ERR_BREDR_CONN_REFUSED,
    ERR_BREDR_CONN_ABORT_BY_REMOTE,
    ERR_BREDR_CONN_ABORT_BY_LOCAL,
    ERR_BREDR_CONN_LMP_PROTO_ERROR,
    ERR_BREDR_CONN_CANCELED,
    ERR_BREDR_CONN_UNKNOWN,

 ////////////////////////////////////////////

    ERR_LE_CONN_INVALID_ARGUMENTS = 20,
    ERR_LE_CONN_ADAPTER_NOT_POWERED,
    ERR_LE_CONN_NOT_SUPPORTED,
    ERR_LE_CONN_ALREADY_CONNECTED,
    ERR_LE_CONN_BAD_SOCKET,
    ERR_LE_CONN_MEMORY_ALLOC,
    ERR_LE_CONN_BUSY,
    ERR_LE_CONN_REFUSED,
    ERR_LE_CONN_CREATE_SOCKET,
    ERR_LE_CONN_TIMEOUT,
    ERR_LE_CONN_SYNC_CONNECT_LIMIT,
    ERR_LE_CONN_ABORT_BY_REMOTE,
    ERR_LE_CONN_ABORT_BY_LOCAL,
    ERR_LE_CONN_LL_PROTO_ERROR,
    ERR_LE_CONN_GATT_BROWSE,
    ERR_LE_CONN_UNKNOWN,

////////////////////////////////////////////////////

    ERR_BREDR_Invalid_Arguments = 36,
    ERR_BREDR_Operation_Progress,
    ERR_BREDR_Already_Exists,
    ERR_BREDR_Operation_Not_Supported,
    ERR_BREDR_Already_Connected,
    ERR_BREDR_Operation_Not_Available,
    ERR_BREDR_Does_Not_Exist,
    ERR_BREDR_Does_Not_Connected,
    ERR_BREDR_Does_In_Progress,
    ERR_BREDR_Operation_Not_Authorized,
    ERR_BREDR_No_Such_Adapter,
    ERR_BREDR_Agent_Not_Available,
    ERR_BREDR_Resource_Not_Ready,


 /************上面错误码为bluez返回错误码****************/
    ERR_BREDR_Bluezqt_DidNot_ReceiveReply,


 /////////////////////////////////////////////////
    ERR_BREDR_INTERNAL_NO_Default_Adapter,
    ERR_BREDR_INTERNAL_Operation_Progress,
    ERR_BREDR_INTERNAL_Already_Connected,
    ERR_BREDR_INTERNAL_Dev_Not_Exist,
    ERR_BREDR_UNKNOWN_Other,
};

enum enum_Adapter_attr
{
    enum_Adapter_attr_Pairing = 0,
    enum_Adapter_attr_Connecting,
    enum_Adapter_attr_Powered,
    enum_Adapter_attr_Discoverable,
    enum_Adapter_attr_Pairable,
    enum_Adapter_attr_Discovering,
    enum_Adapter_attr_Name,
    enum_Adapter_attr_Block,
    enum_Adapter_attr_Addr,
    enum_Adapter_attr_ActiveConnection,
    enum_Adapter_attr_DefaultAdapter,
    enum_Adapter_attr_TrayShow,
    enum_Adapter_attr_FileTransportSupport,
    enum_Adapter_attr_ClearPinCode,
};


enum enum_device_attr
{
    enum_device_attr_Paired = 0,
    enum_device_attr_Trusted,
    enum_device_attr_Blocked,
    enum_device_attr_Connected,
    enum_device_attr_Name,
    enum_device_attr_Type,
    enum_device_attr_TypeName,
    enum_device_attr_Pairing,
    enum_device_attr_Connecting,
    enum_device_attr_Battery,
    enum_device_attr_ConnectFailedId,
    enum_device_attr_ConnectFailedDisc,
    enum_device_attr_Rssi,
    enum_device_attr_Addr,
    enum_device_attr_FileTransportSupport,
    enum_device_attr_ShowName,
    enum_device_attr_Adapter,
};


enum enum_app_attr
{
    enum_app_attr_dbusid = 0,
    enum_app_attr_username,
    enum_app_attr_type,
    enum_app_attr_pid,
};

enum enum_app_type_bluetooth
{
    enum_app_type_bluetooth_controlPanel = 0,  //蓝牙控制面板
    enum_app_type_bluetooth_tray,              //蓝牙托盘
    enum_app_type_bluetooth_other = 0xff,
};

enum enum_start_pair
{
    enum_start_pair_dev = 0,
    enum_start_pair_name,
    enum_start_pair_pincode,
    enum_start_pair_type,
};

enum enum_pair_pincode_type
{
    enum_pair_pincode_type_request = 0,
    enum_pair_pincode_type_display,
};


/**
 * @brief
 *  蓝牙适配器属性发送类型
 */
enum enum_send_type
{
    enum_send_type_delay = 0,
    enum_send_type_immediately,
};


enum enum_filestatus
{
    enum_filestatus_status = 0,
    enum_filestatus_progress,
    enum_filestatus_filename,
    enum_filestatus_allfilenum,
    enum_filestatus_fileseq,
    enum_filestatus_dev,
    enum_filestatus_fileFailedDisc,
    enum_filestatus_filetype,
    enum_filestatus_transportType,
    enum_filestatus_savepath,
    enum_filestatus_filesize,
};

enum enum_filetransport_Type
{
    enum_filetransport_Type_send = 0,
    enum_filetransport_Type_receive,
    enum_filetransport_Type_other,
};


enum enum_receivefile
{
    enum_receivefile_dev = 0,
    enum_receivefile_name,
    enum_receivefile_filetype,
    enum_receivefile_filename,
    enum_receivefile_filesize,
};


extern QStringList Adapter_attr;
#define AdapterAttr(i)   (((Adapter_attr.size()>i))?Adapter_attr[i]:GREATERINDEX)
#define AdapterAttrIndex(str) (Adapter_attr.indexOf(str))

extern QStringList Device_attr;
#define DeviceAttr(i)         (((Device_attr.size()>i))?Device_attr[i]:GREATERINDEX)
#define DeviceAttrIndex(str)  (Device_attr.indexOf(str))

extern QStringList list_errmsg;
#define get_enum_errmsg(str) (list_errmsg.indexOf(str))
#define get_enum_errmsg_str(i)   (((list_errmsg.size()>i))?list_errmsg[i]:GREATERINDEX)


extern QStringList app_attr;
#define AppAttr(i)   (((app_attr.size()>i))?app_attr[i]:GREATERINDEX)
#define AppAttrIndex(str) (app_attr.indexOf(str))


extern QStringList startpair_attr;
#define PairAttr(i)   (((startpair_attr.size()>i))?startpair_attr[i]:GREATERINDEX)
#define PairAttrIndex(str) (startpair_attr.indexOf(str))

extern QStringList filestatus_attr;
#define FileStatusAttr(i)   (((filestatus_attr.size()>i))?filestatus_attr[i]:GREATERINDEX)
#define FileStatusAttrIndex(str) (filestatus_attr.indexOf(str))

extern QStringList receivefile_attr;
#define ReceiveFileAttr(i)   (((receivefile_attr.size()>i))?receivefile_attr[i]:GREATERINDEX)
#define ReceiveFileAttrIndex(str) (receivefile_attr.indexOf(str))



/**
 * @brief
 *  蓝牙协议支持
 * @param
 * @return true or false
 */
bool support_a2dp_sink(const QStringList &);
bool support_a2dp_source(const QStringList &);
bool support_hfphsp_ag(const QStringList &);
bool support_hfphsp_hf(const QStringList &);
bool support_filetransport(const QStringList &);

#endif // COMMON_H
