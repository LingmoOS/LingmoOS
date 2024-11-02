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

#define _GNU_SOURCE     # required for NI_NUMERICHOST
#include "libkync.h"
#include "cstring-extension.h"
#include "kerr.h"
#include "libkylog.h"
#include "sdkmarcos.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <glib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/wireless.h>

#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/genl/genl.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/route/link.h>
#include <libnl3/netlink/route/addr.h>
#include <libnl3/netlink/addr.h>
#include <libnl3/netlink/types.h>
#include <libnl3/netlink/route/route.h>

#include "libnm/NetworkManager.h"

/**
 * Type:
 * @UNKNOWN: unknown device
 * @GENERIC: generic support for unrecognized device types
 * @ETHERNET: a wired ethernet device
 * @WIFI: an 802.11 Wi-Fi device
 * @UNUSED1: not used
 * @UNUSED2: not used
 * @BT: a Bluetooth device supporting PAN or DUN access protocols
 * @OLPC_MESH: an OLPC XO mesh networking device
 * @WIMAX: an 802.16e Mobile WiMAX broadband device
 * @MODEM: a modem supporting analog telephone, CDMA/EVDO,
 * GSM/UMTS, or LTE network access protocols
 * @INFINIBAND: an IP-over-InfiniBand device
 * @BOND: a bond master interface
 * @VLAN: an 802.1Q VLAN interface
 * @ADSL: ADSL modem
 * @BRIDGE: a bridge master interface
 * @TEAM: a team master interface
 * @TUN: a TUN or TAP interface
 * @IP_TUNNEL: a IP tunnel interface
 * @MACVLAN: a MACVLAN interface
 * @VXLAN: a VXLAN interface
 * @VETH: a VETH interface
 * @MACSEC: a MACsec interface
 * @DUMMY: a dummy interface
 * @PPP: a PPP interface
 * @OVS_INTERFACE: a Open vSwitch interface
 * @OVS_PORT: a Open vSwitch port
 * @OVS_BRIDGE: a Open vSwitch bridge
 * @WPAN: a IEEE 802.15.4 (WPAN) MAC Layer Device
 * @6LOWPAN: 6LoWPAN interface
 * @WIREGUARD: a WireGuard interface
 * @WIFI_P2P: an 802.11 Wi-Fi P2P device (Since: 1.16)
 **/
const char *conn_types[] =
    {
        "UNKNOWN",
        "ETHERNET",
        "WIFI",
        "UNUSED1",
        "UNUSED2",
        "BT",
        "OLPC_MESH",
        "WIMAX",
        "MODEM",
        "INFINIBAND",
        "BOND",
        "VLAN",
        "ADSL",
        "BRIDGE",
        "GENERIC",
        "TEAM",
        "TUN",
        "IP_TUNNEL",
        "MACVLAN",
        "VXLAN",
        "VETH",
        "MACSEC",
        "DUMMY",
        "PPP",
        "OVS_INTERFACE",
        "OVS_PORT",
        "OVS_BRIDGE",
        "WPAN",
        "6LOWPAN",
        "WIREGUARD",
        "WIFI_P2P"};

enum cardspec
{
    NCSPEC_ALL,
    NCSPEC_UP,
    NCSPEC_DOWN
};

enum cardstat
{
    NCSTAT_LINK_UP,
    NCSTAT_LINK_DOWN,
    NCSTAT_ERROR
};

enum cardcfg
{
    NCCFG_MAC,
    NCCFG_IPv4,
    NCCFG_IPv6,
    NCCFG_IPv4_MASK,
    NCCFG_IPv6_MASK
};

#define NC_IPv4_SIZE 16
#define NC_MAC_SIZE 18
#define SIOCGIWNAME 0x8B01
#define PCIID_PATH "/usr/share/misc/pci.ids"

struct get_ipv4_addr_ctx_t {
	struct nl_sock *sk;
	int flag; /* 是否找到ip */
	int index; /* 网卡索引 */
	unsigned char *ip;
	unsigned char *netmask;
    unsigned char *broadaddr;
};

struct get_ipv6_addr_ctx_t {
	struct nl_sock *sk;
	int flag; /* 是否找到ip */
	int index; /* 网卡索引 */
	unsigned char *ip;
	int scope;
    int prefixlen;
    int type;
};

static GKeyFile *
configure_open_file(const gchar *filename)
{
    GError *err = NULL;
    GKeyFile *keyfile;
    keyfile = g_key_file_new();
    g_key_file_set_list_separator(keyfile, ',');
    if (!g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_NONE, &err))
    {
        g_error_free(err);
        g_key_file_free(keyfile);
        return NULL;
    }
    return keyfile;
}

static void
configure_read_string(GKeyFile *keyfile, GString *ret, gchar *sec, gchar *key, gchar *default_val)
{
    GError *err = NULL;
    gchar *val = g_key_file_get_string(keyfile,
                                       sec,
                                       key,
                                       &err);
    if (err)
    {
        g_string_assign(ret, default_val);
        g_error_free(err);
        return;
    }
    g_string_assign(ret, val);
    g_free(val);
}

static char **
get_ip_list(enum cardcfg type, const char *nc)
{
    int count = 0;
    char *curAddr = NULL;
    char **result = NULL;
    DIR *dir = NULL;
    struct dirent *file;
    char *path = "/etc/NetworkManager/system-connections/";
    char *value = NULL;
    if (type == NCCFG_IPv4)
    {
        value = "ipv4";
        curAddr = kdk_nc_get_private_ipv4(nc);
    }
    else if (type == NCCFG_IPv6)
    {
        value = "ipv6";
        curAddr = kdk_nc_get_private_ipv6(nc);
    }
    else
        value = NULL;

    if ((dir = opendir(path)) == NULL)
    {
        klog_err("opendir %s failed!", dir);
        free(curAddr);
        return NULL;
    }

    // 网卡当前地址加入返回列表
    if (NULL != curAddr)
    {
        result = (char **)calloc(count + 2, sizeof(char *));
        if (!result)
            goto err_out;
        result[count++] = curAddr;
        curAddr = NULL;
    }

    while (file = readdir(dir))
    {
        // 判断是否为文件
        if (file->d_type != 8)
            continue;
        // 为文件加上相对路径
        char fileName[512] = "\0";
        strncpy(fileName, path, strlen(path));
        strcat(fileName, file->d_name);

        GKeyFile *keyFile = configure_open_file(fileName);
        if (NULL == keyFile)
        {
            klog_err("load config file %s failed\n", fileName);
            continue;
        }

        GString *val = g_string_new("");
        configure_read_string(keyFile, val, "connection", "interface-name", "null");

        if (0 == strcmp(val->str, nc))
        {
            configure_read_string(keyFile, val, value, "address1", "null");
            if (0 != strcmp(val->str, "null"))
            {
                int i = -1;
                while (val->str[++i] != 0x2f) // 0x2f是'/'的ascii码值
                    continue;
                val->str[i] = '\0';
                if (0 != strcmp(result[0], val->str))
                {
                    char **tmp = (char **)realloc(result, (count + 2) * sizeof(char *));
                    if (!tmp)
                    {
                        for (int i = 0; result[i]; i++)
                            free(result[i]);
                        free(result);
                        goto err_out;
                    }
                    result = tmp;

                    result[count++] = strndup(val->str, i);
                    result[count] = NULL;
                }
            }
        }
        g_string_free(val, TRUE);
        g_key_file_free(keyFile);
    }
    closedir(dir);
    return result;

err_out:
    if (result)
    {
        while (count)
        {
            free(result[--count]);
        }
        free(result);
        result = NULL;
    }
    closedir(dir);
    return result;
}

static enum cardstat _get_card_stat(const char *nc)
{
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (!sfd)
    {
        klog_err("网卡[%s]状态获取失败：%s\n", nc, strerror(errno));
        return NCSTAT_ERROR;
    }

    struct ifreq stIf;
    strcpy(stIf.ifr_ifrn.ifrn_name, nc);

    if (ioctl(sfd, SIOCGIFFLAGS, &stIf) < 0)
    {
        klog_err("网卡[%s]状态获取失败：%s\n", nc, strerror(errno));
        close(sfd);
        return NCSTAT_ERROR;
    }

    close(sfd);
    if (stIf.ifr_ifru.ifru_flags & IFF_RUNNING)
        return NCSTAT_LINK_UP;

    return NCSTAT_LINK_DOWN;
}

static char **_get_cardlist(enum cardspec spec)
{
    char **cards = NULL;
    size_t cardnum = 0;
#ifdef __linux__
    FILE *fp = fopen("/proc/net/dev", "r");
    ASSERT_NOT_NULL(fp, NULL);
    char buffer[1024];
    while (fgets(buffer, 1024, fp))
    {
        if (strncmp(strskipblank(buffer), "Inter", 5) == 0 || strncmp(strskipblank(buffer), "face", 4) == 0)
            continue;
        int pos = strfirstof(buffer, ':');
        if (pos > 0)
        {
            buffer[pos] = 0;
            strstripblank(buffer);
            if (spec == NCSPEC_UP)
            {
                if (_get_card_stat(buffer) != NCSTAT_LINK_UP)
                    continue;
            }
            else if (spec == NCSPEC_DOWN)
            {
                if (_get_card_stat(buffer) != NCSTAT_LINK_DOWN)
                    continue;
            }
            cardnum++;
            char **tmp = realloc(cards, sizeof(char *) * cardnum);
            if (!tmp)
            {
                klog_err("内存申请失败：%s\n", strerror(errno));
                goto err_out;
            }
            cards = tmp;
            int index = cardnum - 1;
            cards[index] = malloc(sizeof(char) * (strlen(buffer) + 1));
            if (!cards[index])
            {
                klog_err("内存申请失败：%s\n", strerror(errno));
                goto err_out;
            }
            strcpy(cards[index], buffer);
        }
    }
    char **tmp = realloc(cards, sizeof(char *) * (cardnum + 1));
    if (!tmp)
    {
        klog_err("内存申请失败：%s\n", strerror(errno));
        goto err_out;
    }
    cards = tmp;
    cards[cardnum] = NULL;
    goto out;
#endif // __linux__
err_out:
#ifdef __linux__
    fclose(fp);
#endif
    while (cardnum)
    {
        free(cards[cardnum - 1]);
        cardnum--;
    }
    SAFE_FREE(cards);
    return NULL;
out:
    fclose(fp);
    return cards;
}

char **_get_nc_cfg(const char *nc, enum cardcfg cfg)
{
    char **res = NULL;
    struct ifreq stIf;
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        klog_err("网卡[%s]信息获取失败：%s\n", nc, strerror(errno));
        return NULL;
    }
    strncpy(stIf.ifr_ifrn.ifrn_name, nc, sizeof(stIf.ifr_ifrn.ifrn_name) - 1);

    switch (cfg)
    {
    case NCCFG_MAC:
    {
        if (ioctl(sfd, SIOCGIFHWADDR, &stIf) < 0)
        {
            klog_err("网卡[%s]MAC获取失败：%s\n", nc, strerror(errno));
            close(sfd);
            return NULL;
        }
        res = malloc(sizeof(char *));
        if (!res)
        {
            klog_err("内存申请失败:%s\n", strerror(errno));
            close(sfd);
            return NULL;
        }
        res[0] = malloc(sizeof(char) * NC_MAC_SIZE);
        if (!res[0])
        {
            free(res);
            klog_err("内存申请失败:%s\n", strerror(errno));
            close(sfd);
            return NULL;
        }
        snprintf(res[0], NC_MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
                 (unsigned char)stIf.ifr_ifru.ifru_hwaddr.sa_data[0],
                 (unsigned char)stIf.ifr_ifru.ifru_hwaddr.sa_data[1],
                 (unsigned char)stIf.ifr_ifru.ifru_hwaddr.sa_data[2],
                 (unsigned char)stIf.ifr_ifru.ifru_hwaddr.sa_data[3],
                 (unsigned char)stIf.ifr_ifru.ifru_hwaddr.sa_data[4],
                 (unsigned char)stIf.ifr_ifru.ifru_hwaddr.sa_data[5]);
    }
    break;
    case NCCFG_IPv4:
    {
        if (ioctl(sfd, SIOCGIFADDR, &stIf) < 0)
        {
            klog_err("网卡[%s]IPv4获取失败：%s\n", nc, strerror(errno));
            close(sfd);
            return NULL;
        }
        res = malloc(sizeof(char *));
        if (!res)
        {
            klog_err("内存申请失败:%s\n", strerror(errno));
            close(sfd);
            return NULL;
        }
        res[0] = malloc(NC_IPv4_SIZE * sizeof(char));
        if (!res[0])
        {
            free(res);
            klog_err("内存申请失败:%s\n", strerror(errno));
            close(sfd);
            return NULL;
        }
        struct sockaddr_in sin;
        memcpy(&sin, &stIf.ifr_ifru.ifru_addr, sizeof(sin));
        snprintf(res[0], NC_IPv4_SIZE, "%s", inet_ntoa(sin.sin_addr));
    }
    break;
    case NCCFG_IPv6:
    {
        struct ifaddrs *ifap, *ifa;
        struct sockaddr_in6 *sa;
        char addr[INET6_ADDRSTRLEN] = {0};
        getifaddrs(&ifap);
        for (ifa = ifap; ifa; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr)
            {
                klog_err("网卡地址为空，检查网卡是否存在\n");
                return NULL;
            }
            if (ifa->ifa_addr->sa_family == AF_INET6 && !strcmp(ifa->ifa_name, nc))
            {
                sa = (struct sockaddr_in6 *)ifa->ifa_addr;
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), addr,
                            sizeof(addr), NULL, 0, NI_NUMERICHOST);
            }
        }

        if(strlen(addr) != 0)
        {
            res = malloc(sizeof(char *));
            if (!res)
            {
                klog_err("内存申请失败:%s\n", strerror(errno));
                close(sfd);
                freeifaddrs(ifap);
                return NULL;
            }
            res[0] = malloc(sizeof(char) * INET6_ADDRSTRLEN);
            if (!res[0])
            {
                free(res);
                klog_err("内存申请失败:%s\n", strerror(errno));
                close(sfd);
                freeifaddrs(ifap);
                return NULL;
            }

            int i = 0;
            while (addr[i] != '%' && addr[i] != '\0')
                i++;
            addr[i] = '\0';
            
            sprintf(res[0], "%s", addr);

            freeifaddrs(ifap);
        }
    }
    break;
    default:
        break;
    }
    close(sfd);
    return res;
}

char **kdk_nc_get_cardlist()
{
    return _get_cardlist(NCSPEC_ALL);
}

int kdk_nc_is_up(const char *nc)
{
    if (!nc)
        return -1;
    enum cardstat stat = _get_card_stat(nc);
    if (stat == NCSTAT_LINK_UP)
        return 1;
    else if (stat == NCSTAT_LINK_DOWN)
        return 0;
    return -1;
}

char **kdk_nc_get_upcards()
{
    return _get_cardlist(NCSPEC_UP);
}

char *kdk_nc_get_phymac(const char *nc)
{
    if (!nc)
        return NULL;

    char **maclist = _get_nc_cfg(nc, NCCFG_MAC);
    if (!maclist)
        return NULL;
    char *mac = maclist[0];
    free(maclist);
    return mac;
}

char *kdk_nc_get_private_ipv4(const char *nc)
{
    if (!nc)
        return NULL;

    char **ipv4list = _get_nc_cfg(nc, NCCFG_IPv4);
    if (!ipv4list)
        return NULL;
    char *ipv4 = ipv4list[0];
    free(ipv4list);
    return ipv4;
}

char **kdk_nc_get_ipv4(const char *nc)
{
    if (!nc)
        return NULL;
    return get_ip_list(NCCFG_IPv4, nc);
}

char *kdk_nc_get_private_ipv6(const char *nc)
{
    if (!nc)
        return NULL;

    char *ipv6 = NULL;

    char **ipv6list = _get_nc_cfg(nc, NCCFG_IPv6);
    if (!ipv6list)
        return NULL;
    ipv6 = ipv6list[0];
    free(ipv6list);
    return ipv6;
}

char **kdk_nc_get_ipv6(const char *nc)
{
    if (!nc)
        return NULL;
    return get_ip_list(NCCFG_IPv6, nc);
}

int kdk_nc_is_wireless(const char *nc)
{
    if (!nc)
        return -1;

    GError *error = NULL;

    NMClient *client = nm_client_new(NULL, &error);
    if (!client) {
        klog_err("Error creating NMClient: %s\n", error->message);
        g_error_free(error);
        return -1;
    }

    NMDevice *device = nm_client_get_device_by_iface(client, nc);
    if (!device)
    {
        klog_err("Failed to get device for interface eth0\n");
        return -1;
    }
    int type = nm_device_get_device_type(device);
    if (2 == type)
        return 1;
    return 0;
}

// 根据mac地址获取从hwinfo --usb中获取指定网卡的vendor和device
static int __hwinfo_usb(const char *mac, char *vendor, char *product)
{
    FILE *fp = popen("hwinfo --usb", "r");
    if (!fp)
        return -1;
    char line[1024] = "\0";
    char ven[256] = "\0";
    char pro[256] = "\0";
    int flags = 0;
    while (fgets(line, 1024, fp))
    {
        int index = 0;
        if (flags && (0 < strlen(ven)) && (0 < strlen(pro)))
        {
            break;
        }

        if (strstr(line, "HW Address") && strstr(line, mac))
        {
            flags = 1;
        }

        if (strstr(line, "Vendor:"))
        {
            // 返回数据为Vendor: usb 0xxxxx "Linux Foundation"，取最后双引号中的内容
            strcpy(ven, &line[22]);
        }

        if (strstr(line, "Device:"))
        {
            // 返回数据为Device: usb 0xxxxx "2.0 root hub"，取最后双引号中的内容
            strcpy(pro, &line[22]);
        }

        if (0 < sscanf(line, "%d:%*s", &index))
        {
            memset(ven, 0, 256);
            memset(pro, 0, 256);
        }
    }
    if (flags)
    {
        strncpy(vendor, ven, strlen(ven) - 2);
        strncpy(product, pro, strlen(pro) - 2);
    }
    fclose(fp);
    return 0;
}

int kdk_nc_get_vendor_and_product(const char *nc, char *vendor, char *product)
{
    if (!nc)
        return -1;
    char filename[64] = "0";
    sprintf(filename, "/sys/class/net/%s/device", nc);
    if (0 != access(filename, F_OK))
        return -1; // 非物理网卡，可能是逻辑网卡如 lo: 本地环回
    FILE *fp = NULL;

    // usb网卡通过hwinfo获取
    char ncType[64] = "\0";
    sprintf(ncType, "%s/%s", filename, "uevent");
    if (NULL != (fp = fopen(ncType, "r")))
    {
        char text[1024] = "\0";
        if (0 > fread(text, sizeof(char), 1024, fp))
        {
            klog_err("open %s failed", ncType);
            return -1;
        }
        if (strstr(text, "usb_interface"))
        {
            char *mac = kdk_nc_get_phymac(nc);
            int ret = __hwinfo_usb(mac, vendor, product);
            free(mac);
            return ret;
        }
    }
    char vendorId[64] = "0";
    char productId[64] = "0";
    strcpy(vendorId, filename);
    strcpy(productId, filename);
    // 读厂家id
    if (NULL != (fp = fopen(strcat(vendorId, "/vendor"), "r")))
    {
        fgets(vendorId, sizeof(vendorId), fp);
        fclose(fp);
    }
    // 读设备id
    if (NULL != (fp = fopen(strcat(productId, "/device"), "r")))
    {
        fgets(productId, sizeof(vendorId), fp);
        fclose(fp);
    }

    long u[4] = {0, 1, 2, 3}; // 0,1存储网卡的厂家和设备ID，2，3存储pci.ids文件读到的ID
    // memset(u, 0, sizeof(u));
    int count = sscanf(vendorId, "%lx", &u[0]);
    if (count < 1)
        return -1;
    sscanf(productId, "%lx", &u[1]);
    if (count < 1)
        return -1;

    char line[512] = "0";
    if (NULL != (fp = fopen(PCIID_PATH, "r")))
    {
        long unsigned int level = 0;
        while (fgets(line, sizeof(line), fp))
        {
            // 忽略空行和注释
            if (strlen(line) == 0 || line[0] == '#')
                continue;

            // 行前无制表符代表厂家id，一个制表符代表设备id
            level = 0;
            while (level < strlen(line) && line[level] == '\t')
                level++;
            // 获取厂家名称
            if (0 == level)
            {
                sscanf(line, "%lx", &u[2]);
                if (u[0] == u[2])
                    strcpy(vendor, line + level + 6);
            }
            // 在对应的厂家id下查找设备id
            if (1 == level && u[0] == u[2])
            {
                sscanf(line, "%lx", &u[3]);
                if (u[1] == u[3])
                {
                    strcpy(product, line + level + 6);
                    break;
                }
            }
        }
        fclose(fp);
    }
    return 0;
}

char *kdk_nc_get_driver(const char *nc)
{
    char *driver = NULL;
    int ret;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, nc);

    struct ethtool_drvinfo driverInfo;
    memset(&driverInfo, 0, sizeof(struct ethtool_drvinfo));
    driverInfo.cmd = ETHTOOL_GDRVINFO;

    ifr.ifr_data = (char *)&driverInfo;
    ret = ioctl(fd, SIOCETHTOOL, &ifr);
    if (0 != ret)
        goto out;
    
    driver = strdup(driverInfo.driver);
out:
    close(fd);
    return driver;
}

inline void kdk_nc_freeall(char **list)
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

char *kdk_nc_get_uuid(char *nc)
{
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return NULL;
    }

    strcpy(ifr.ifr_name, nc); // 网卡接口名，可以根据实际情况修改

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
    {
        perror("IOCTL failed");
        return NULL;
    }

    close(sock);

    unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

    char uuid[512] = {0};
    // 将MAC地址转换为UUID格式
    sprintf(uuid, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[0], mac[1], mac[2], mac[3]);

    return strdup(uuid);
}

int kdk_nc_get_carrier(char *nc)
{
    struct nl_sock *sock = nl_socket_alloc();
    if (!sock)
    {
        klog_err("[%s]Failed to allocate netlink socket", __func__);
        return -1;
    }

    if (nl_connect(sock, NETLINK_ROUTE) < 0)
    {
        klog_err("[%s]Failed to connect to netlink socket", __func__);
        nl_socket_free(sock);
        return -1;
    }

    struct nl_cache *link_cache = NULL;
    struct rtnl_link *link = NULL;
    int if_index = if_nametoindex(nc);

    if (!if_index)
    {
        klog_err("[%s]Failed to get interface index", __func__);
        nl_socket_free(sock);
        return -1;
    }

    if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &link_cache) < 0)
    {
        klog_err("[%s]Failed to allocate link cache", __func__);
        nl_socket_free(sock);
        return -1;
    }

    link = rtnl_link_get(link_cache, if_index);
    if (!link)
    {
        klog_err("[%s]Failed to get link information", __func__);
        nl_cache_free(link_cache);
        nl_socket_free(sock);
        return -1;
    }

    uint8_t carrier = rtnl_link_get_carrier(link);

    nl_cache_free(link_cache);
    nl_socket_free(sock);
    return carrier;
}

char *kdk_nc_get_duplex(char *nc)
{
    char path[1024] = {0};
    char res[1024] = {0};
    char *result = NULL;
    sprintf(path, "/sys/class/net/%s/duplex", nc);
    // if (NULL == realpath(path, real_path))
    //     return NULL;
    FILE *fp = fopen(path, "r");
    if(NULL == fp)
        return NULL;
    fgets(res, 1024, fp);
    if(strlen(res) != 0)
    {
        strstripspace(res);
        result = strdup(res);
    }
    fclose(fp);
    return result;
}

static void ipv4_prefixlen2netmask(int prefixlen, unsigned char *netmask)
{
    char buffer[32] = {0};
 
    uint32_t mask = 0XFFFFFFFF;
    mask = -1 << (32 - prefixlen);
    sprintf(buffer, "%d.%d.%d.%d", (mask >> 24) & 0xFF, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
    
    strcpy(netmask, buffer);
}

static void get_ipv4_addr_cache_cb(struct nl_object *obj, void *arg)
{
    const size_t bufsiz = 128;
    char tmpbuf[bufsiz];
    char broadcast[bufsiz];
    
	struct get_ipv4_addr_ctx_t *ctx = (struct get_ipv4_addr_ctx_t *)arg;
	struct rtnl_addr *rtnl_addr = (struct rtnl_addr *)obj;
 
	if (rtnl_addr_get_ifindex(rtnl_addr) == ctx->index) {
		struct nl_addr *nl_addr = rtnl_addr_get_local(rtnl_addr);
 
		if (nl_addr_get_family(nl_addr) == AF_INET && nl_addr_get_len(nl_addr) == 4) {
			ctx->flag = 0x01;

            nl_addr2str(nl_addr, tmpbuf, bufsiz);
            tmpbuf[bufsiz - 1] = '\0';
            char ** res = strsplit(tmpbuf, '/');
            if(strstr(res[0], ctx->ip))
            {
                struct nl_addr *nl_broad = rtnl_addr_get_broadcast(rtnl_addr);
                nl_addr2str(nl_broad, broadcast, bufsiz);
                broadcast[bufsiz - 1] = '\0';
                ctx->broadaddr = (char *)malloc(sizeof(char) * 32);
                strcpy(ctx->broadaddr, broadcast);

                int netmask_prefixlen = rtnl_addr_get_prefixlen(rtnl_addr);
                if (netmask_prefixlen <= 32) {
                    ctx->netmask = (char *)malloc(sizeof(char) * 32);
                    memset(ctx->netmask, 0, 32);
				    ipv4_prefixlen2netmask(netmask_prefixlen, ctx->netmask);
			    }
            }
		}
	}
}

static void get_ipv6_addr_cache_cb(struct nl_object *obj, void *arg)
{
    const size_t bufsiz = 128;
    char tmpbuf[bufsiz];
    char broadcast[bufsiz];
    
	struct get_ipv6_addr_ctx_t *ctx = (struct get_ipv6_addr_ctx_t *)arg;
	struct rtnl_addr *rtnl_addr = (struct rtnl_addr *)obj;
 
	if (rtnl_addr_get_ifindex(rtnl_addr) == ctx->index) {
		struct nl_addr *nl_addr = rtnl_addr_get_local(rtnl_addr);

        if(ctx->type == 1)
        {
            if (nl_addr_get_family(nl_addr) == AF_INET && nl_addr_get_len(nl_addr) == 4) {
                ctx->flag = 0x01;

                nl_addr2str(nl_addr, tmpbuf, bufsiz);
                tmpbuf[bufsiz - 1] = '\0';
                char ** res = strsplit(tmpbuf, '/');
                if(strstr(res[0], ctx->ip))
                {
                    int netmask_prefixlen = rtnl_addr_get_prefixlen(rtnl_addr);
                    if (netmask_prefixlen <= 32) {
                        ctx->prefixlen = netmask_prefixlen;
                    }
                }
            }
        }
        else{
            if (nl_addr_get_family(nl_addr) == AF_INET6 && nl_addr_get_len(nl_addr) == 16) {
                ctx->flag = 0x01;
                nl_addr2str(nl_addr, tmpbuf, bufsiz);
                tmpbuf[bufsiz - 1] = '\0';
                char ** res = strsplit(tmpbuf, '/');
                if(strstr(res[0], ctx->ip))
                {
                    int netmask_prefixlen = rtnl_addr_get_prefixlen(rtnl_addr);
                    ctx->prefixlen = netmask_prefixlen;

                    int scope = rtnl_addr_get_scope(rtnl_addr);
                    ctx->scope = scope;
                }
            }
        }
	}
}

char *kdk_nc_get_broadAddr(const char *eth_name, const char *address)
{
    if (!address || !eth_name)
        return NULL;
    
    int ret = 0;
	struct nl_cache *addr_cache;
	struct rtnl_link *link;
    char *ip = (char *)malloc(sizeof(char) * 256);
    if(!ip)
    {
        return NULL;
    }
    memset(ip, 0, 256);
    strcpy(ip, address);

	struct nl_sock *sk = nl_socket_alloc();
	nl_connect(sk, NETLINK_ROUTE);
 
	ret = rtnl_link_get_kernel(sk, 0, eth_name, &link);
 
	if (ret < 0) {
		ret = -1;
		goto error_out;
	}
 
	struct get_ipv4_addr_ctx_t ctx;
	memset(&ctx, 0x00, sizeof(struct get_ipv4_addr_ctx_t));
 
	ctx.index = rtnl_link_get_ifindex(link);
	ctx.ip = ip;
 
	rtnl_addr_alloc_cache(sk, &addr_cache);
	nl_cache_foreach(addr_cache, get_ipv4_addr_cache_cb, &ctx);
 
	if (ctx.flag == 0) {
		goto error_out;
	} 
 
	nl_cache_put(addr_cache);
	rtnl_link_put(link);
    free(ip);

    char *broadAddr = NULL;

    if(ctx.broadaddr)
    {
        broadAddr = (char *)malloc(sizeof(char) * 256);
        if(!broadAddr)
        {
            return NULL;
        }
        memset(broadAddr, 0, 256);
        strcpy(broadAddr, ctx.broadaddr);
        free(ctx.broadaddr);
    }
    nl_close(sk);
	nl_socket_free(sk);
    return broadAddr;
 
error_out:
	nl_close(sk);
	nl_socket_free(sk);
    free(ip);
	return NULL;

}

char *kdk_nc_get_netmask(const char *eth_name, const char *addr)
{
    if (!addr || !eth_name)
        return NULL;
    
    int ret = 0;
	struct nl_cache *addr_cache;
	struct rtnl_link *link;
    char *ip = (char *)malloc(sizeof(char) * 512);
    if(!ip)
    {
        return NULL;
    }
    memset(ip, 0, 512);
    strcpy(ip, addr);

    char *netmask = NULL;

	struct nl_sock *sk = nl_socket_alloc();
	nl_connect(sk, NETLINK_ROUTE);
 
	ret = rtnl_link_get_kernel(sk, 0, eth_name, &link);
 
	if (ret < 0) {
		ret = -1;
		goto error_out;
	}
 
	struct get_ipv4_addr_ctx_t ctx;
	memset(&ctx, 0x00, sizeof(struct get_ipv4_addr_ctx_t));
 
	ctx.index = rtnl_link_get_ifindex(link);
	ctx.ip = ip;
 
	rtnl_addr_alloc_cache(sk, &addr_cache);
	nl_cache_foreach(addr_cache, get_ipv4_addr_cache_cb, &ctx);
 
	if (ctx.flag == 0) {
		goto error_out;
	} 
 
	nl_cache_put(addr_cache);
	rtnl_link_put(link);
    free(ip);
    if(ctx.netmask)
    {
        netmask = (char *)malloc(sizeof(char) * 256);
        if(!netmask)
        {
            return NULL;
        }
        memset(netmask, 0, 256);
        strcpy(netmask, ctx.netmask);
        free(ctx.netmask);
    }
    
    nl_close(sk);
	nl_socket_free(sk);
    return netmask;
 
error_out:
	nl_close(sk);
	nl_socket_free(sk);
    free(ip);
	return NULL;
}

int kdk_nc_get_mask_length(int type, const char *eth_name, const char *addr)
{
    if (!addr || !eth_name)
        return -1;
    
    int ret = 0;
	struct nl_cache *addr_cache;
	struct rtnl_link *link;
    char *ip = (char *)malloc(sizeof(char) * 256);
    if(!ip)
    {
        return -1;
    }
    memset(ip, 0, 256);
    strcpy(ip, addr);

	struct nl_sock *sk = nl_socket_alloc();
	nl_connect(sk, NETLINK_ROUTE);
 
	ret = rtnl_link_get_kernel(sk, 0, eth_name, &link);
 
	if (ret < 0) {
		ret = -1;
		goto error_out;
	}
 
	struct get_ipv6_addr_ctx_t ctx;
	memset(&ctx, 0x00, sizeof(struct get_ipv6_addr_ctx_t));
 
	ctx.index = rtnl_link_get_ifindex(link);
	ctx.ip = ip;
 
	rtnl_addr_alloc_cache(sk, &addr_cache);
	nl_cache_foreach(addr_cache, get_ipv6_addr_cache_cb, &ctx);
 
	if (ctx.flag == 0) {
		goto error_out;
	} 
 
	nl_cache_put(addr_cache);
	rtnl_link_put(link);
    free(ip);
    nl_close(sk);
	nl_socket_free(sk);
    return ctx.prefixlen;
 
error_out:
	nl_close(sk);
	nl_socket_free(sk);
    free(ip);
	return -1;
}

char *kdk_nc_get_conn_type(const char *nc)
{
    if(!nc)
        return NULL;

    GError *error = NULL;

    NMClient *client = nm_client_new(NULL, &error);
    if (!client) {
        klog_err("Error creating NMClient: %s\n", error->message);
        g_error_free(error);
        return NULL;
    }

    NMDevice *device = nm_client_get_device_by_iface(client, nc);
    if (!device)
    {
        klog_err("Failed to get device for interface eth0\n");
        return NULL;
    }
    int type = nm_device_get_device_type(device);

    return conn_types[type];
}

char *kdk_nc_get_wifi_name(const char *nc)
{
    if(!nc)
        return NULL;
    struct iwreq wrq;
    struct iw_statistics stats;
    char *wifi_name = (char *)malloc(sizeof(char) * 512);
    if(!wifi_name)
        return NULL;
    memset(wifi_name, 0, 512);
    char buffer[512];
    memset(&wrq,0,sizeof(wrq));
    memset(&stats,0,sizeof(stats));
    memset(buffer, 0, 512);
    strcpy(wrq.ifr_name, nc);
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = sizeof(struct iw_statistics);
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        close(sock);
        free(wifi_name);
        return NULL;
     }

    if (ioctl(sock, SIOCGIWSTATS, &wrq) == -1) {
        close(sock);
        free(wifi_name);
        return NULL;
    }

    wrq.u.essid.pointer = buffer;//如果不写这行可能会错误
    wrq.u.essid.length = 512;
    if (ioctl(sock, SIOCGIWESSID, &wrq) == -1) {
        close(sock);
        free(wifi_name);
        return NULL;
    }    

    if(wrq.u.essid.flags != 0){
        strncpy(wifi_name, buffer, 512);    //wifi名称
    }
    close(sock);
    return wifi_name;
}

int kdk_nc_get_wifi_signal_qual(const char *nc)
{
    struct iwreq wrq;
    struct iw_statistics stats;
    char buffer[512];
    memset(&wrq,0,sizeof(wrq));
    memset(&stats,0,sizeof(stats));
    memset(buffer, 0, 512);
    strcpy(wrq.ifr_name, nc);
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = sizeof(struct iw_statistics);
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
          close(sock);
          return -1;
     }

    if (ioctl(sock, SIOCGIWSTATS, &wrq) == -1) {
        close(sock);
        return -1;
    }

    wrq.u.essid.pointer = buffer;//如果不写这行可能会错误
    wrq.u.essid.length = 512;
    if (ioctl(sock, SIOCGIWESSID, &wrq) == -1) {
        close(sock);
        return -1;
    }    

    close(sock);
    return stats.qual.qual;
}

int kdk_nc_get_wifi_signal_level(const char *nc)
{
    struct iwreq wrq;
    struct iw_statistics stats;
    char buffer[512];
    memset(&wrq,0,sizeof(wrq));
    memset(&stats,0,sizeof(stats));
    memset(buffer, 0, 512);
    strcpy(wrq.ifr_name, nc);
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = sizeof(struct iw_statistics);
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
          close(sock);
          return -1;
     }

    if (ioctl(sock, SIOCGIWSTATS, &wrq) == -1) {
        close(sock);
        return -1;
    }

    wrq.u.essid.pointer = buffer;//如果不写这行可能会错误
    wrq.u.essid.length = 512;
    if (ioctl(sock, SIOCGIWESSID, &wrq) == -1) {
        close(sock);
        return -1;
    }    
    
    close(sock);
    return stats.qual.level;
}

int kdk_nc_get_wifi_noise(const char *nc)
{
    struct iwreq wrq;
    struct iw_statistics stats;
    char buffer[512];
    memset(&wrq,0,sizeof(wrq));
    memset(&stats,0,sizeof(stats));
    memset(buffer, 0, 512);
    strcpy(wrq.ifr_name, nc);
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = sizeof(struct iw_statistics);
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
          close(sock);
          return -1;
     }

    if (ioctl(sock, SIOCGIWSTATS, &wrq) == -1) {
        close(sock);
        return -1;
    }

    wrq.u.essid.pointer = buffer;//如果不写这行可能会错误
    wrq.u.essid.length = 512;
    if (ioctl(sock, SIOCGIWESSID, &wrq) == -1) {
        close(sock);
        return -1;
    }    
    
    close(sock);
    return stats.qual.noise;
}

char *kdk_nc_get_speed(const char *nc)
{
    if(!nc)
        return NULL;
    struct ifreq ifr;
    struct ethtool_cmd ecmd;
    char *speed = (char *)malloc(sizeof(char) * 32);
    if(!speed)
        return NULL;
    memset(speed, 0, 32);

    ecmd.cmd = 0x00000001;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, nc);

    ifr.ifr_data = (char*)&ecmd;
    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (!ioctl(fd, SIOCETHTOOL, &ifr)) {
        sprintf(speed, "%d Mb/s", ecmd.speed);
        close(fd);
        return speed;
    }
    else{
        close(fd);
        free(speed);
        return NULL;
    }
}

int kdk_nc_get_rx_packets(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int packets = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/rx_packets", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &packets);
    fclose(fp);
    return packets;
}

int kdk_nc_get_rx_bytes(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int bytes = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/rx_bytes", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &bytes);
    fclose(fp);
    return bytes; 
}

int kdk_nc_get_rx_errors(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int errors = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/rx_errors", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &errors);
    fclose(fp);
    return errors; 
}

int kdk_nc_get_rx_dropped(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int dropped = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/rx_dropped", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &dropped);
    fclose(fp);
    return dropped; 
}

int kdk_nc_get_rx_fifo_errors(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int fifo_errors = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/rx_fifo_errors", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &fifo_errors);
    fclose(fp);
    return fifo_errors; 
}

int kdk_nc_get_rx_frame_errors(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int frame_errors = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/rx_frame_errors", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &frame_errors);
    fclose(fp);
    return frame_errors; 
}

int kdk_nc_get_tx_packets(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int packets = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/tx_packets", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &packets);
    fclose(fp);
    return packets; 
}

int kdk_nc_get_tx_bytes(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int tx_bytes = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/tx_bytes", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &tx_bytes);
    fclose(fp);
    return tx_bytes;
}

int kdk_nc_get_tx_errors(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};   
    char res[64] = {0};
    int tx_errors = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/tx_errors", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)   
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &tx_errors);
    fclose(fp);
    return tx_errors;
}

int kdk_nc_get_tx_dropped(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int tx_dropped = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/tx_dropped", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &tx_dropped);
    fclose(fp);
    return tx_dropped;
}

int kdk_nc_get_tx_fifo_errors(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int fifo_errors = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/tx_fifo_errors", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &fifo_errors);
    fclose(fp);
    return fifo_errors;
}

int kdk_nc_get_tx_carrier_errors(const char *nc)
{
    if(!nc)
        return -1;
    char filename[256] = {0};
    char res[64] = {0};
    int carrier_errors = 0;
    sprintf(filename, "/sys/class/net/%s/statistics/tx_carrier_errors", nc);
    FILE *fp = fopen(filename, "r");
    if (! fp)
        return -1;
    
    fgets(res, sizeof(res), fp);
    strstrip(res, '\n');
    sscanf(res, "%d", &carrier_errors);
    fclose(fp);
    return carrier_errors;
}

int kdk_nc_get_scope(const char *eth_name, const char *addr)
{
    if (!addr || !eth_name)
        return -1;
    
    int ret = 0;
	struct nl_cache *addr_cache;
	struct rtnl_link *link;
    char *ip = (char *)malloc(sizeof(char) * 256);
    if(!ip)
    {
        return -1;
    }
    memset(ip, 0, 256);
    strcpy(ip, addr);

	struct nl_sock *sk = nl_socket_alloc();
	nl_connect(sk, NETLINK_ROUTE);
 
	ret = rtnl_link_get_kernel(sk, 0, eth_name, &link);
 
	if (ret < 0) {
		ret = -1;
		goto error_out;
	}
 
	struct get_ipv6_addr_ctx_t ctx;
	memset(&ctx, 0x00, sizeof(struct get_ipv6_addr_ctx_t));
 
	ctx.index = rtnl_link_get_ifindex(link);
	ctx.ip = ip;
 
	rtnl_addr_alloc_cache(sk, &addr_cache);
	nl_cache_foreach(addr_cache, get_ipv6_addr_cache_cb, &ctx);
 
	if (ctx.flag == 0) {
		goto error_out;
	} 
 
	nl_cache_put(addr_cache);
	rtnl_link_put(link);
    free(ip);
    nl_close(sk);
	nl_socket_free(sk);
    return ctx.scope;
 
error_out:

	nl_close(sk);
	nl_socket_free(sk);
    free(ip);
	return -1;
}
