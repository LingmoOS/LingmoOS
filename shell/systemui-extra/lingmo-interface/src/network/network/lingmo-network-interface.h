/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#ifndef __LINGMONETWORKINTERFACE_H__
#define __LINGMONETWORKINTERFACE_H__

#include<stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

/*
 * Get the network interface information.
 * @if_name is used to save the interface name.
 * return the interface number.
 */
int lingmo_network_network_getifacename(char **if_name);

/* Get the ip address.
 * @if_name is the interface name.
 * @ipaddr is used to save the ip address.
 */
int lingmo_network_network_getipaddr(char *if_name,char *ipaddr);

/*
 * Set the ip address.
 * @if_name is the interface name.
 * @ipaddr is the ip address that need to be set.
 */
int lingmo_network_network_setip(char *if_name,char *ipaddr);

/*
 * Get the broadcast address.
 * @if_name is the interface name.
 * @brdaddr is used to save the broadcast address.
 */
int lingmo_network_network_getbrdaddr(char *if_name,char *brdaddr);

/*
 * Set the broadcast address.
 * @if_name is the interface name.
 * @brdaddr is the broadcast address that need to be set.
 */
int lingmo_network_network_setbrdaddr(char *if_name,char *brdaddr);

/*
 * Get the subnet mask.
 * @if_name is the interface name.
 * @netmask is used to save the subnet mask.
 */
int lingmo_network_network_getnetmask(char *if_name,char *netmask);

/* Set the subnet mask.
 * @if_name is the interface name.
 * @netmask is the subnet mask that need to be set.
 */
int lingmo_network_network_setnetmask(char *if_name,char *netmask);

/*
 * Add routing tables.
 * @gateway is the gateway address that need to be added.
 */
int lingmo_network_network_addgateway(char *gateway);

/*
 * Delete routing tables.
 * @gateway is the gateway address that need to be deleted.
 */
int lingmo_network_network_delgateway(char *gateway);

/*
 * Get MAC address.
 * @if_name is the interface name.
 * @macaddr is used to save the MAC address.
 */
int lingmo_network_network_getmac(char *if_name,char *macaddr);

/*
 * Enable network interface.
 * @if_name is the interface name that need to be enabled.
 */
int lingmo_network_network_setifup(char *if_name);

/*
 * Disabled network interface.
 * @if_name is the interface name that need to be disabled.
 */
int lingmo_network_network_setifdown(char *if_name);

/* Get the MTU.
 * @if_name is the interface name.
 * return the MTU value.
 */
int lingmo_network_network_getmtu(char *if_name);

/*
 * Total upload and download data volume.
 * @if_name is the network interface name.
 * return the address of the first element of a one-dimensional long integer array.
 */
long *lingmo_network_network_getbytes(char *if_name);

/*
 * Total upload and download data packets.
 * @if_name is the network interface name.
 * return the address of the first element of a one-dimensional long integer array.
 */
long *lingmo_network_network_getpackets(char *if_name);

/*
 * Total wrong data packets number of uploading and downloading.
 * @if_name is the network interface name.
 * return the address of the first element of a one-dimensional long integer array.
 */
long *lingmo_network_network_geterrs(char *if_name);

/*
 * Total discarded data packets number of uploading and downloading.
 * @if_name is the network interface name.
 * return the address of the first element of a one-dimensional long integer array.
 */
long *lingmo_network_network_getdrop(char *if_name);

/*
 * Total overloaded data packets number of uploading and downloading.
 * @if_name is the network interface name.
 * return the address of the first element of a one-dimensional long integer array.
 */
long *lingmo_network_network_getfifo(char *if_name);

#ifdef __cplusplus
}
#endif

#endif
