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

#include "../libkync.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    char **cards = kdk_nc_get_cardlist();
    size_t index = 0;
    printf("所有网卡：\n");
    while (cards[index])
    {
        char *mac = kdk_nc_get_phymac(cards[index]);
        char *ipv4 = kdk_nc_get_private_ipv4(cards[index]);
        char *ipv6 = kdk_nc_get_private_ipv6(cards[index]);
        char *driver = kdk_nc_get_driver(cards[index]);
        char vendor[256] = "\0", product[256] = "\0";
        kdk_nc_get_vendor_and_product(cards[index], vendor, product);
        printf("Card %zd: %s\tStatus: %s\tMac: %s\tIPv4: %s\tIPv6: %s\t Vendor: %s\t Product: %s\t Type: %s\t driver: %s\n",
               index + 1, cards[index], kdk_nc_is_up(cards[index]) == 1 ? "Up" : "Down",
               mac, ipv4, ipv6, vendor, product,
               kdk_nc_is_wireless(cards[index]) ? "wireless" : "ethernet", driver);

        if (mac)
            free(mac);
        if (ipv4)
            free(ipv4);
        if (ipv6)
            free(ipv6);
        if (driver)
            free(driver);

        char *uuid = kdk_nc_get_uuid(cards[index]);
        printf("%s\n", uuid ? uuid : "get uuid faile");
        free(uuid);
        
        printf("%d\n", kdk_nc_get_carrier(cards[index]));

        char *duplex = kdk_nc_get_duplex(cards[index]);
        printf("%s\n", duplex ? duplex : "get duplex failed");
        free(duplex);

        char *conn = kdk_nc_get_conn_type(cards[index]);
        printf("Conn type = %s\n", conn);

        char *wifi_name = kdk_nc_get_wifi_name(cards[index]);
        printf("wifi name = %s\n", wifi_name);
        free(wifi_name);

        char *speed = kdk_nc_get_speed(cards[index]);
        printf("speed = %s\n", speed);
        free(speed);

        int rx_packets = kdk_nc_get_rx_packets(cards[index]);
        printf("rx_packets = %d\n", rx_packets);

        int rx_bytes = kdk_nc_get_rx_bytes(cards[index]);
        printf("rx_bytes = %d\n", rx_bytes);

        int rx_errors = kdk_nc_get_rx_errors(cards[index]);
        printf("rx_errors = %d\n", rx_errors);

        int rx_dropped = kdk_nc_get_rx_dropped(cards[index]);
        printf("rx_dropped = %d\n", rx_dropped);

        int fifo_errors = kdk_nc_get_rx_fifo_errors(cards[index]);
        printf("fifo_errors = %d\n", fifo_errors);

        int frame_errors = kdk_nc_get_rx_frame_errors(cards[index]);
        printf("frame_errors = %d\n", frame_errors);

        int tx_packets = kdk_nc_get_tx_packets(cards[index]);
        printf("tx_packets = %d\n", tx_packets);

        int tx_bytes = kdk_nc_get_tx_bytes(cards[index]);
        printf("tx_bytes = %d\n", tx_bytes);

        int tx_errors = kdk_nc_get_tx_errors(cards[index]);
        printf("tx_errors = %d\n", tx_errors);

        int tx_dropped = kdk_nc_get_tx_dropped(cards[index]);
        printf("tx_dropped = %d\n", tx_dropped);

        int tx_fifo_errors = kdk_nc_get_tx_fifo_errors(cards[index]);
        printf("tx_fifo_errors = %d\n", tx_fifo_errors);

        int tx_carrier_errors = kdk_nc_get_tx_carrier_errors(cards[index]);
        printf("tx_carrier_errors = %d\n", tx_carrier_errors);

        int signal_qual = kdk_nc_get_wifi_signal_qual(cards[index]);
        printf("signal_qual = %d\n", signal_qual);

        int signal_level = kdk_nc_get_wifi_signal_level(cards[index]);
        printf("signal_level = %d\n", signal_level);

        int noise = kdk_nc_get_wifi_noise(cards[index]);
        printf("noise = %d\n", noise);

        char **list4 = kdk_nc_get_ipv4(cards[index]);
        int i = 0;
        printf("AllIpV4: ");
        while (list4 && list4[i])
        {
            printf("%s\t", list4[i]);

            char *boardaddr = kdk_nc_get_broadAddr(cards[index], list4[i]);
            printf("boardaddr = %s\n", boardaddr);
            free(boardaddr);

            char *netmask = kdk_nc_get_netmask(cards[index], list4[i]);
            printf("netmask = %s\n", netmask);
            free(netmask);

            free(list4[i++]);
        }
        printf("\n");
        char **list6 = kdk_nc_get_ipv6(cards[index]);
        printf("AllIpV6: ");
        i = 0;
        while (list6 && list6[i])
        {
            printf("%s\t", list6[i]);

            int len = kdk_nc_get_mask_length(0, cards[index], list6[i]);
            printf("netmask_length = %d\n", len);

            int scope = kdk_nc_get_scope(cards[index], list6[i]);
            printf("scope = %d\n", scope);
            
            free(list6[i++]);
        }
        printf("\n");

        free(list4);
        free(list6);
        index++;
    }
    kdk_nc_freeall(cards);

    char **upcards = kdk_nc_get_upcards();
    index = 0;
    printf("工作网卡：\n");
    while (upcards[index])
    {
        printf("Card %zd: %s\n", index + 1, upcards[index]);
        index++;
    }
    kdk_nc_freeall(upcards);
    return 0;
}
