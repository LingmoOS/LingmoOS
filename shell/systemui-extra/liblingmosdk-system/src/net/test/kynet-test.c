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

#include "../libkynetinfo.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc, char *argv[])
{
    prouteMapList list = kdk_net_get_route(), tmp = list;
    while (tmp)
    {
        printf("%s default route %s\n", tmp->name, tmp->addr);
        tmp = tmp->next;
    }
    kdk_net_free_route(list);
    if (argc < 2)
        printf("please input test port num\n");
    else
        printf("port:%s's state %d\n", argv[1], kdk_net_get_port_stat(atoi(argv[1])));
    
    pChain chain = kdk_net_get_iptable_rules();
    pChain tmpchain = chain;
    while(tmpchain)
    {
        printf("Chain: %s\tpolice: %s\n",tmpchain->total,tmpchain->policy);
        pChainData tmpData = tmpchain->data;
        while (tmpData)
        {
            printf("target: %s\tprot: %s\topt: %s\tsource: %s\tdestination: %s\toption: %s\n",tmpData->target,tmpData->prot,tmpData->opt,
                                tmpData->source,tmpData->destination,tmpData->option);
            tmpData = tmpData->next;
            printf("\n");
        }
        tmpchain = tmpchain->next;
    }
    kdk_net_free_chain(chain);

    int result[65536];
    int net = kdk_net_get_multiple_port_stat(0, 1000, result);
    if(net == 0)
    {
        size_t count = 0;
        for(;count < 1000 ;count++)
        {
            printf("%d\t", result[count]);
        }
    }
    else
        printf("Failed With %d", net);
     printf("\n");
    
    char mask[32] = "\0";
    kdk_net_get_netmask("enaftgm1i0", mask);
    printf("mask = %s\n", mask);

    char **port = kdk_net_get_proc_port();
    size_t index = 0;
    while(port[index])
    {
        printf("No.%ld, port = %s\n", index+1, port[index]);
        index++;
    }
    kdk_net_freeall(port);

    port = kdk_net_get_up_port();
    index = 0;
    while(port[index])
    {
        printf("No.%ld, port = %s\n", index+1, port[index]);
        index++;
    }
    kdk_net_freeall(port);

    char *hosts = kdk_net_get_hosts();
    if (hosts != NULL)
    {
        printf("hosts = %s\n", hosts);
        free(hosts);
    }

    char *domain = kdk_net_get_hosts_domain();
    if (domain != NULL)
    {
        printf("domain = %s\n", domain);
        free(domain);
    }

    char **resolv = kdk_net_get_resolv_conf();
    index = 0;
    while(resolv[index])
    {
        printf("No.%ld, resolv = %s\n", index+1, resolv[index]);
        index++;
    }
    kdk_net_freeall(resolv);

    int link_type = kdk_net_get_link_type();
    printf("Link type : %d\n", link_type);

    char **link_list = kdk_net_get_link_ncNmae();
    if (NULL != link_list)
    {
        for (int i = 0; link_list[i]; i++)
        {
            printf("up interface :%s\n", link_list[i]);
            free(link_list[i]);
        }
        free(link_list);
    }

    char *contype = kdk_net_get_primary_conType();
    if (contype)
    {
        printf("PrimaryConnectionType :%s\n", contype);
        free(contype);
    }

#define WIRELESSNC "wlx004bf3de83b5"
    char *mode = kdk_net_get_wifi_mode(WIRELESSNC);
    if (mode)
    {
        printf("Wifi mode :%s\n", mode);
        free(mode);
    }

    char *freq = kdk_net_get_wifi_freq(WIRELESSNC);
    if (freq)
    {
        printf("Wifi freq :%s\n", freq);
        free(freq);
    }

    char *cahnnel = kdk_net_get_wifi_channel(WIRELESSNC);
    if (cahnnel)
    {
        printf("Wifi cahnnel :%s\n", cahnnel);
        free(cahnnel);
    }

    char *rate = kdk_net_get_wifi_rate(WIRELESSNC);
    if (rate)
    {
        printf("Wifi rate :%s\n", rate);
        free(rate);
    }

    char *sens = kdk_net_get_wifi_sens(WIRELESSNC);
    if (sens)
    {
        printf("Wifi sens :%s\n", sens);
        free(sens);
    }
#undef WIRELESSNC

    char **addrs = kdk_net_get_addr_by_name("www.google.com");
    if (NULL != addrs)
    {
        for (int i = 0; addrs[i]; i++)
        {
            printf("addr :%s\n", addrs[i]);
            free(addrs[i]);
        }
        free(addrs);
    }

    char *h_name = kdk_net_get_name_by_addr("108.160.170.26");
    if (h_name)
    {
        printf("h_name : %s\n", h_name);
        free(h_name);
    }

    Dhcp4Config *dhcp_4_config = kdk_net_get_ipv4_dhcp_config();
    if(NULL != dhcp_4_config)
    {
        printf("DHCP4 broadcast_address:%s\n",                          dhcp_4_config->broadcast_address);
        printf("DHCP4 dad_wait_time:%d\n",                              dhcp_4_config->dad_wait_time);
        printf("DHCP4 dhcp_lease_time:%d\n",                            dhcp_4_config->dhcp_lease_time);
        printf("DHCP4 dhcp_message_type:%d\n",                          dhcp_4_config->dhcp_message_type);
        printf("DHCP4 dhcp_server_identifier:%s\n",                     dhcp_4_config->dhcp_server_identifier);
        printf("DHCP4 domain_name_servers:%s\n",                        dhcp_4_config->domain_name_servers);
        printf("DHCP4 expiry:%s\n",                                     dhcp_4_config->expiry);
        printf("DHCP4 ip_address:%s\n",                                 dhcp_4_config->ip_address);
        printf("DHCP4 network_number:%s\n",                             dhcp_4_config->network_number);
        printf("DHCP4 next_server:%s\n",                                dhcp_4_config->next_server);
        printf("DHCP4 requested_broadcast_address:%d\n",                dhcp_4_config->requested_broadcast_address);
        printf("DHCP4 requested_domain_name:%d\n",                      dhcp_4_config->requested_domain_name);
        printf("DHCP4 requested_domain_name_servers:%d\n",              dhcp_4_config->requested_domain_name_servers);
        printf("DHCP4 requested_domain_search:%d\n",                    dhcp_4_config->requested_domain_search);
        printf("DHCP4 requested_host_name:%d\n",                        dhcp_4_config->requested_host_name);
        printf("DHCP4 requested_interface_mtu:%d\n",                    dhcp_4_config->requested_interface_mtu);
        printf("DHCP4 requested_ms_classless_static_routes:%d\n",       dhcp_4_config->requested_ms_classless_static_routes);
        printf("DHCP4 requested_netbios_name_servers:%d\n",             dhcp_4_config->requested_netbios_name_servers);
        printf("DHCP4 requested_netbios_scope:%d\n",                    dhcp_4_config->requested_netbios_scope);
        printf("DHCP4 requested_ntp_servers:%d\n",                      dhcp_4_config->requested_ntp_servers);
        printf("DHCP4 requested_rfc3442_classless_static_routes:%d\n",  dhcp_4_config->requested_rfc3442_classless_static_routes);
        printf("DHCP4 requested_root_path:%d\n",                        dhcp_4_config->requested_root_path);
        printf("DHCP4 requested_routers:%d\n",                          dhcp_4_config->requested_routers);
        printf("DHCP4 requested_static_routes:%d\n",                    dhcp_4_config->requested_static_routes);
        printf("DHCP4 requested_subnet_mask:%d\n",                      dhcp_4_config->requested_subnet_mask);
        printf("DHCP4 requested_time_offset:%d\n",                      dhcp_4_config->requested_time_offset);
        printf("DHCP4 requested_wpad:%d\n",                             dhcp_4_config->requested_wpad);
        printf("DHCP4 routers:%s\n",                                    dhcp_4_config->routers);
        printf("DHCP4 server_name:%s\n",                                dhcp_4_config->server_name);
        printf("DHCP4 subnet_mask:%s\n",                                dhcp_4_config->subnet_mask);
    }

    Dhcp6Config *dhcp_6_config = kdk_net_get_ipv6_dhcp_config();
    if(NULL != dhcp_6_config)
    {
        printf("DHCP6 dad_wait_time:%d\n",                  dhcp_6_config->dad_wait_time);
        printf("DHCP6 dhcp6_client_id:%s\n",                dhcp_6_config->dhcp6_client_id);
        printf("DHCP6 dhcp6_name_servers:%s\n",             dhcp_6_config->dhcp6_name_servers);
        printf("DHCP6 dhcp6_server_id:%s\n",                dhcp_6_config->dhcp6_server_id);
        printf("DHCP6 requested_dhcp6_client_id:%d\n",      dhcp_6_config->requested_dhcp6_client_id);
        printf("DHCP6 requested_dhcp_domain_search:%d\n",   dhcp_6_config->requested_dhcp_domain_search);
        printf("DHCP6 requested_dhp6_name_servers:%d\n\n",  dhcp_6_config->requested_dhp6_name_servers);
    }

    return 0;
}
