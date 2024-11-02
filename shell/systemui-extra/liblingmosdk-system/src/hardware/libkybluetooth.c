/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "libkybluetooth.h"
#include "hd.h"
#include <dbus-1.0/dbus/dbus.h>
#include <libkylog.h>
#include <stdbool.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>

int** kdk_bluetooth_get_device_id()
{
	static struct hci_dev_info di;
	int **id = (int **)calloc(16, sizeof(int*));
	if(!id)
	{
		return 0;
	}
	struct hci_dev_list_req *dl;
	struct hci_dev_req *dr;
	int i, ctl;
	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0) {
		free(id);
		return 0;
	}

	if (!(dl = malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t)))) {
		free(id);
		return 0;
	}
	dl->dev_num = HCI_MAX_DEV;
	dr = dl->dev_req;

	if (ioctl(ctl, HCIGETDEVLIST, (void *) dl) < 0) {
		free(dl);
		free(id);
		return 0;
	}

	for (i = 0; i< dl->dev_num; i++) {
		
		di.dev_id = (dr+i)->dev_id;
		id[i] = malloc(sizeof(int) * 16);
		if(!id[i])
		{
			free(dl);
			free(id);
			return 0;
		}
		id[i] = (int*)di.dev_id;
	}
	// i = i + 1;
	id[i] = 0;
	return id;
}

char *kdk_bluetooth_get_manufacturer(int id)
{
    char name[64] = "\0";

    hd_data_t *hd_data;
    hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
    if (!hd_data)
    {
        return NULL;
    }

    hd_data->progress = NULL;
    hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);
	int i = 0;
    hd_t *hd, *hd0;
    hd0 = hd_list(hd_data, hw_bluetooth, 1, NULL);
	if(!hd0)
    {
		goto out;
    }
    
    for (hd = hd0; hd; hd = hd->next)
    {
		if(i == id)
		{
			if(hd->vendor.name)
			{
				strcpy(name, hd->vendor.name);
			}
		}
		i++;
    }
	if(name[0] == '\0')
	{
		goto out;
	}
	char *manufacturer = (char *)malloc(64 * sizeof(char));
    if (!manufacturer)
    {
		goto out;
    }
	strcpy(manufacturer, name);
	return manufacturer;
out:
	hd_free_hd_data(hd_data);
	hd_free_hd_list(hd0);
	return NULL;
}

char *kdk_bluetooth_get_dev_version(int id)
{
    char name[64] = "\0";

    hd_data_t *hd_data;
    hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
    if (!hd_data)
    {
        return NULL;
    }

    hd_data->progress = NULL;
    hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);

	int i = 0;
    hd_t *hd, *hd0;
    hd0 = hd_list(hd_data, hw_bluetooth, 1, NULL);
	if(!hd0)
    {
        return NULL;
    }

    for (hd = hd0; hd; hd = hd->next)
    {
		if(i == id)
		{
			if(hd->revision.name)
			{
				strcpy(name, hd->revision.name);
			}
		}
        i++;
	}
	hd_free_hd_data(hd_data);
	hd_free_hd_list(hd0);

	if(name[0] == '\0')
	{
		int ctl;
		static struct hci_dev_info di;
		struct hci_version ver;
		int dd;
		char tmp[30] = "\0";

		if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
		{
			return NULL;
		}

		if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
		{
			return NULL;
		}
		
		// print_dev_info(ctl, &di);
		dd = hci_open_dev(di.dev_id);
		if (dd < 0)
		{
			return NULL;
		}

		if (hci_read_local_version(dd, &ver, 1000) < 0)
		{
			return NULL;
		}
		if(ver.hci_rev)
		{
			sprintf(tmp, "0x%x", ver.hci_rev);
			strcpy(name, tmp);
		}		
		hci_close_dev(dd);
	}
	char *version = (char *)malloc(12 * sizeof(char));
    if (!version)
    {
        return NULL;
    }
	strcpy(version, name);
    return version;
}

char* kdk_bluetooth_get_name(int id)
{
	int ctl;
	static struct hci_dev_info di;
	int dd;
	char name[249];
	int i;
	
	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}
	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		return NULL;
	}

	dd = hci_open_dev(di.dev_id);
	if (dd < 0) {
		return NULL;
	}

	if (hci_read_local_name(dd, sizeof(name), name, 1000) < 0) {
		return NULL;
	}

	for (i = 0; i < 248 && name[i]; i++) {
		if ((unsigned char) name[i] < 32 || name[i] == 127)
			name[i] = '.';
	}

	hci_close_dev(dd);

	name[248] = '\0';
	char *version = (char *)malloc(sizeof(char) * 64);
	if(!version)
	{
		return NULL;
	}
	strcpy(version, name);
	return version;
}

char* kdk_bluetooth_get_address(int id)
{
	int ctl;
	static struct hci_dev_info di;
	char addr[18] = "\0";

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		return NULL;
	}

	ba2str(&di.bdaddr, addr);
	char *address = (char *)malloc(sizeof(char) * 64);
	if (!address)
	{
		return NULL;
	}
	if(addr[0] != '\0')
	{
		strcpy(address, addr);
		return address;
	}
	else{
		free(address);
		return NULL;
	}
}

char* kdk_bluetooth_get_link_mode(int id)
{
	char *str = NULL;
	int ctl;
	static struct hci_dev_info di;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		return NULL;
	}
	str =  hci_lmtostr(di.link_mode);
	char *link_mode = (char *)malloc(sizeof(char) * 64);
	if(!link_mode)
	{
		return NULL;
	}
	if(str)
	{
		strcpy(link_mode, str);		
		bt_free(str);
		return link_mode;
	}
	else{
		free(link_mode);
		return NULL;
	}
}

char* kdk_bluetooth_get_link_policy(int id)
{
	int ctl;
	static struct hci_dev_info di;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		return NULL;
	}

	char *link_policy = (char *)malloc(sizeof(char) * 64);
	if(!link_policy)
	{
		return NULL;
	}
	strcpy(link_policy, hci_lptostr(di.link_policy));
	return link_policy;
}

char* kdk_bluetooth_get_bus(int id)
{
	int ctl;
	static struct hci_dev_info di;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}
	
	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		return NULL;
	}

	char *bus = (char *)malloc(sizeof(char) * 64);
	if(!bus)
	{
		return NULL;
	}
	strcpy(bus, hci_bustostr(di.type & 0x0f));
	return bus;
}

char* kdk_bluetooth_get_scomtu(int id)
{
	int ctl;
	static struct hci_dev_info di;
	char addr[18];
	char tmp[20] = "\0";

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		return NULL;
	}

	ba2str(&di.bdaddr, addr);
	sprintf(tmp, "%d:%d", di.sco_mtu, di.sco_pkts);
	char *scomtu = (char *)malloc(sizeof(char) * 64);
	if(!scomtu)
	{
		return NULL;
	}
	strcpy(scomtu, tmp);
	return scomtu;
}

char* kdk_bluetooth_get_alcmtu(int id)
{
	int ctl;
	static struct hci_dev_info di;
	char tmp[20] = "\0";
	char *alcmtu = (char *)malloc(sizeof(char) * 64);
	if(!alcmtu)
	{
		return NULL;
	}

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		free(alcmtu);
		return NULL;
	}

	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		free(alcmtu);
		return NULL;
	}

	sprintf(tmp, "%d:%d", di.acl_mtu, di.acl_pkts);
	strcpy(alcmtu, tmp);
	return alcmtu;
}

char* kdk_bluetooth_get_packettype(int id)
{
	char *str;
	int ctl;
	static struct hci_dev_info di;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		return NULL;
	}
	str = hci_ptypetostr(di.pkt_type);
	char *packettype = (char *)malloc(sizeof(char) * 64);
	if(!packettype)
	{
		return NULL;
	}
	strcpy(packettype, str);
	bt_free(str);
	return packettype;
}

char* kdk_bluetooth_get_features(int id)
{
	int ctl;
	static struct hci_dev_info di;
	char tmp[50] = "\0";
	char *features = (char *)malloc(sizeof(char) * 64);
	if(!features)
		return NULL;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		free(features);
		return NULL;
	}

	di.dev_id = id;

	if (ioctl(ctl, HCIGETDEVINFO, (void *)&di))
	{
		free(features);
		return NULL;
	}

	sprintf(tmp, "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x",
			di.features[0], di.features[1], di.features[2], di.features[3], 
			di.features[4], di.features[5], di.features[6], di.features[7]);
	strcpy(features, tmp);
	return features;
}

char** kdk_bluetooth_get_device_addr(int id)
{
    // hci_for_each_dev(HCI_UP, conn_list, id);
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int i;
	char** con_addr = NULL;

	if (id < 0)
		return NULL;

	int ctl;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
		return NULL;
	}
	cl->dev_id = id;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if (ioctl(ctl, HCIGETCONNLIST, (void *) cl)) {
		return NULL;
	}
	int index = 0;

	for (i = 0; i < cl->conn_num; i++, ci++) {
		char addr[18];
		char *str;
		ba2str(&ci->bdaddr, addr);
		str = hci_lmtostr(ci->link_mode);
		if(strlen(addr) != 0)
		{
			char **tmp = (char **)realloc(con_addr, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
			if (!tmp)
				return NULL;
			con_addr = tmp;
			con_addr[index] = (char *)calloc(1, sizeof(addr));
			if (!con_addr[index])
			{
				free(tmp);
				return NULL;
			}
			strcpy(con_addr[index], addr);
			index++;
		}
		bt_free(str);
	}
	if(con_addr)
		con_addr[index] = NULL;
	free(cl);
    return con_addr;
}

char** kdk_bluetooth_get_device_name(int id)
{
	struct hci_dev_info di;
	di.dev_id = id;
	char addr[18];
	int ctl;
	char** con_name = NULL;
	int num = 0;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	if (ioctl(ctl, HCIGETDEVINFO, (void *) &di))
		return NULL;

	ba2str(&di.bdaddr, addr);
	char path[1024] = {0};

	char** con_addr = kdk_bluetooth_get_device_addr(id);
	int index = 0;
	if(con_addr != NULL)
	{
		while (con_addr[index])
		{
			char addr[256] = {0};
			strcpy(addr, con_addr[index]);
			int length = strlen(addr);
			for (int i = 0; i < length; i++) {
				if (addr[i] == ':') {
					addr[i] = '_';
				}
			}
			
			sprintf(path, "/org/bluez/%s/dev_%s", di.name, addr);
			
			DBusConnection *conn;
			DBusError err;
			DBusMessageIter iter;
			char *result = NULL;

			dbus_error_init(&err);
			conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

			if (dbus_error_is_set(&err))
			{
				fprintf(stderr, "Connection Error (%s)\n", err.message);
				dbus_error_free(&err);
			}

			if (NULL == conn)
			{
				goto err_out;
			}

			DBusMessage *info_msg = NULL;
			DBusPendingCall *sendMsgPending = NULL;
			DBusMessage *reply = NULL;
			char *interface = "org.bluez.Device1";
        	char *name = "Name";

			//创建用户
			info_msg = dbus_message_new_method_call("org.bluez",  // target for the method call
													path, // object to call on
													"org.freedesktop.DBus.Properties",  // interface to call on
													"Get");               // method name
			if (!info_msg)
			{ // -1 is default timeout
				klog_err("status_msg：dbus_message_new_method_call调用失败\n");
				goto err_out;
			}

			if(!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
			{
				klog_err("kdk : d-bus append args fail !\n");
				dbus_message_unref(info_msg);
				dbus_connection_unref(conn);
				goto err_out;
			}

			// 发送DBus消息并等待回复
			reply = dbus_connection_send_with_reply_and_block(conn, info_msg, -1, &err);
			if (reply == NULL)
			{
				klog_err("[%s]-> DBus reply error: %s\n", __func__, err.message);
				dbus_message_unref(info_msg);
				dbus_connection_unref(conn);
				goto err_out;
			}

			// 释放message
			dbus_message_unref(info_msg);
			info_msg = NULL;

			// 解析回复消息
			if (!dbus_message_iter_init(reply, &iter) ||
				dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
			{
				klog_err("[%s]-> DBus reply parsing failed\n");
				dbus_message_unref(reply);
				dbus_connection_unref(conn);
				goto err_out;
			}

			// 释放reply
			dbus_message_unref(reply);
			reply = NULL;

			dbus_message_iter_recurse(&iter, &iter);
			dbus_message_iter_get_basic(&iter, &result);

			dbus_connection_unref(conn);
			if(result)
			{
				char **tmp = (char **)realloc(con_name, sizeof(char *) * (num + 1 + 1)); // 以NULL结尾
				if (!tmp)
					goto err_out;
				con_name = tmp;
				strstripspace(result);
				con_name[num] = (char *)calloc(1, strlen(result) + 1);
				if (!con_name[num])
				{
					free(tmp);
					goto err_out;
				}
				klog_info("result = %s, len = %d\n", result, strlen(result));
				strcpy(con_name[num], result);
				num++;
			}
			
			index++;
		}
		free(con_addr);
	}
	if(con_name)
		con_name[index] = NULL;
	return con_name;

err_out:
    return NULL;
}

bool kdk_bluetooth_get_connection_status(int id)
{
	bool result = false;
	char** con_addr = kdk_bluetooth_get_device_addr(id);
	
	if(con_addr)
	{
		result = true;
		free(con_addr);
	}
	else
		result = false;

    return result;
}

char** kdk_bluetooth_get_addr_type(int id)
{
	struct hci_dev_info di;
	di.dev_id = id;
	char addr[18];
	int ctl;
	char** addr_type = NULL;
	int num = 0;

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		return NULL;
	}

	if (ioctl(ctl, HCIGETDEVINFO, (void *) &di))
		return NULL;

	ba2str(&di.bdaddr, addr);
	char path[1024] = {0};

	char** con_addr = kdk_bluetooth_get_device_addr(id);
	int index = 0;
	if(con_addr != NULL)
	{
		while (con_addr[index])
		{
			char addr[256] = {0};
			strcpy(addr, con_addr[index]);
			int length = strlen(addr);
			for (int i = 0; i < length; i++) {
				if (addr[i] == ':') {
					addr[i] = '_';
				}
			}
			
			sprintf(path, "/org/bluez/%s/dev_%s", di.name, addr);
			
			DBusConnection *conn;
			DBusError err;
			DBusMessageIter iter;
			char *result = NULL;

			dbus_error_init(&err);
			conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

			if (dbus_error_is_set(&err))
			{
				fprintf(stderr, "Connection Error (%s)\n", err.message);
				dbus_error_free(&err);
			}

			if (NULL == conn)
			{
				goto err_out;
			}

			DBusMessage *info_msg = NULL;
			DBusPendingCall *sendMsgPending = NULL;
			DBusMessage *reply = NULL;
			char *interface = "org.bluez.Device1";
        	char *type = "AddressType";

			//创建用户
			info_msg = dbus_message_new_method_call("org.bluez",  // target for the method call
													path, // object to call on
													"org.freedesktop.DBus.Properties",  // interface to call on
													"Get");               // method name
			if (!info_msg)
			{ // -1 is default timeout
				klog_err("status_msg：dbus_message_new_method_call调用失败\n");
				goto err_out;
			}

			if(!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &type, DBUS_TYPE_INVALID))
			{
				klog_err("kdk : d-bus append args fail !\n");
				dbus_message_unref(info_msg);
				dbus_connection_unref(conn);
				goto err_out;
			}

			// 发送DBus消息并等待回复
			reply = dbus_connection_send_with_reply_and_block(conn, info_msg, -1, &err);
			if (reply == NULL)
			{
				klog_err("[%s]-> DBus reply error: %s\n", __func__, err.message);
				dbus_message_unref(info_msg);
				dbus_connection_unref(conn);
				goto err_out;
			}

			// 释放message
			dbus_message_unref(info_msg);
			info_msg = NULL;

			// 解析回复消息
			if (!dbus_message_iter_init(reply, &iter) ||
				dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
			{
				klog_err("[%s]-> DBus reply parsing failed\n");
				dbus_message_unref(reply);
				dbus_connection_unref(conn);
				goto err_out;
			}

			// 释放reply
			dbus_message_unref(reply);
			reply = NULL;

			dbus_message_iter_recurse(&iter, &iter);
			dbus_message_iter_get_basic(&iter, &result);

			dbus_connection_unref(conn);
			if(result)
			{
				char **tmp = (char **)realloc(addr_type, sizeof(char *) * (num + 1 + 1)); // 以NULL结尾
				if (!tmp)
					goto err_out;
				addr_type = tmp;
				addr_type[num] = (char *)calloc(1, sizeof(result));
				if (!addr_type[num])
				{
					free(tmp);
					goto err_out;
				}
				strcpy(addr_type[num], result);
				num++;
			}
			
			index++;
		}
		free(con_addr);
	}
	if(addr_type)
		addr_type[index] = NULL;
	return addr_type;

err_out:
    return NULL;
}

kdk_major_class kdk_bluetooth_get_major_class(int id)
{
	inquiry_info *ii = NULL;
	int max_rsp, num_rsp;
	int sock, len, flags;
	int i;
	char addr[256] = {0};
	kdk_major_class list = NULL, currnode = NULL, prenode = NULL;

	// int dev_id = hci_get_route(NULL);
	sock = hci_open_dev(id);
	if (id < 0 || sock < 0)
	{
		return NULL;
	}

	len = 8;
	max_rsp = 255;
	flags = IREQ_CACHE_FLUSH;
	ii = (inquiry_info *)malloc(max_rsp * sizeof(inquiry_info));

	num_rsp = hci_inquiry(id, len, max_rsp, NULL, &ii, flags);
	if (num_rsp < 0)
	{
		hci_close_dev(sock);
        return NULL;
	}

	for (i = 0; i < num_rsp; i++)
	{
		ba2str(&(ii + i)->bdaddr, addr);
		// printf("\t%s\tclock offset: 0x%4.4x\tclass: %2.2x\n",
		// 	   addr, btohs((ii + i)->clock_offset),
		// 	   (ii + i)->dev_class[0]);
		currnode = (kdk_major_class)calloc(1, sizeof(major_class));
		if (!currnode)
		{
			free(ii);
            hci_close_dev(sock);
            return list;
		}

		// printf("addr = %s\n", addr);
		// if(strlen(addr) != 0)
			// strncpy(currnode->addr, addr, sizeof(addr));

		// printf("currnode->addr = %s\n", currnode->addr);
		switch ((ii + i)->dev_class[0])
		{
		case 0x01:
			currnode->type = COMPUTER;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("COMPUTER\n");
			break;
		case 0x02:
			currnode->type = PHONE;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("PHONE\n");
			break;
		case 0x03:
			currnode->type = NETWORK_ACCESS_POINT;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("NETWORK_ACCESS_POINT\n");
			break;
		case 0x04:
			currnode->type = AUDIO_VIDEO;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("AUDIO_VIDEO\n");
			break;
		case 0x05:
			currnode->type = PERIPHERAL;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("PERIPHERAL\n");
			break;
		case 0x06:
			currnode->type = IMAGING;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("IMAGING\n");
			break;
		case 0x07:
			currnode->type = WEARABLE;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("WEARABLE\n");
			break;
		case 0x08:
			currnode->type = TOY;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("TOY\n");
			break;
		case 0x09:
			currnode->type = HEALTH;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("HEALTH\n");
			break;

		default:
			currnode->type = MISCELLANEOUS;
			strncpy(currnode->addr, addr, sizeof(addr));
			// printf("MISCELLANEOUS\n");
			break;
		}
		currnode->next = NULL;
		if (!list)
			list = currnode;
		else
			prenode->next = currnode;
		prenode = currnode;
	}

	free(ii);
	hci_close_dev(sock);
	return list;
}

void kdk_bluetooth_free_major_class(kdk_major_class info)
{
	kdk_major_class tmp = NULL;
    while (info)
    {
        tmp = info;
        info = info->next;
        free(tmp);
    }
}

inline void kdk_bluetooth_freeall(char **list)
{
	if (!list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index++;
    }
    free(list);
}
