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

#include "../libkybluetooth.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i = 0;
    char *manufacturer = "\0";
    char *version = "\0";
    char *name = "\0";
    char *address = "\0";
    char *link_mode  ="\0"; 
    char *link_policy ="\0";
    char *scomtu = "\0";
    char *alcmtu = "\0";
    char *packettype = "\0";
    char *features = "\0";
    char *bus = "\0";
    int *id = (int *)kdk_bluetooth_get_device_id();
    if(NULL == id)
        return 0;
    for(; id[i]; i++)
    {
        printf("id = %d\n",id[i]);

        manufacturer = kdk_bluetooth_get_manufacturer(i);
        printf("manufacturer: %s\n", manufacturer);
        free(manufacturer);

        version = kdk_bluetooth_get_dev_version(i);
        printf("version: %s\n", version);
        free(version);

        name = kdk_bluetooth_get_name(id[i]);
        printf("name: %s\n", name);

        address = kdk_bluetooth_get_address(id[i]);
        printf("address: %s\n", address);

        link_mode = kdk_bluetooth_get_link_mode(id[i]);
        printf("link_mode: %s\n", link_mode);

        link_policy = kdk_bluetooth_get_link_policy(id[i]);
        printf("link_policy: %s\n", link_policy);

        scomtu = kdk_bluetooth_get_scomtu(id[i]);
        printf("scomtu: %s\n", scomtu);

        alcmtu = kdk_bluetooth_get_alcmtu(id[i]);
        printf("alcmtu: %s\n", alcmtu);

        packettype = kdk_bluetooth_get_packettype(id[i]);
        printf("packettype: %s\n", packettype);

        features = kdk_bluetooth_get_features(id[i]);
        printf("features: %s\n", features);

        bus = kdk_bluetooth_get_bus(id[i]);
        printf("bus: %s\n", bus);

        char** con_addr = kdk_bluetooth_get_device_addr(id[i]);
        int index = 0;
        if(con_addr != NULL)
        {
            while (con_addr[index])
            {
                printf("addr: %s\n", con_addr[index]);
                index++;
            }

            kdk_bluetooth_freeall(con_addr);
        }

        char** con_name = kdk_bluetooth_get_device_name(id[i]);
        index = 0;
        if(con_name != NULL)
        {
            while (con_name[index])
            {
                printf("name: %s\n", con_name[index]);
                index++;
            }

            kdk_bluetooth_freeall(con_name);
        }

        bool status = kdk_bluetooth_get_connection_status(id[i]);
        printf("status = %d\n", status);

        char** addr_type =  kdk_bluetooth_get_addr_type(id[i]);
        index = 0;
        if(addr_type != NULL)
        {
            while (addr_type[index])
            {
                printf("addr type: %s\n", addr_type[index]);
                index++;
            }

            kdk_bluetooth_freeall(addr_type);
        }

        kdk_major_class list = kdk_bluetooth_get_major_class(id[i]);
        kdk_major_class major = list;
        if(major != NULL)
        {
            while (major)
            {
                printf("addr:%s, type:%d\n", major->addr, major->type);
                major = major->next;
            }

            kdk_bluetooth_free_major_class(major);
        }

        // free((int*)id[i]);
    }
    free((int**)id);
    return 0;
}
