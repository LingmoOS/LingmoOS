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

#include "libkynetinfo.h"
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <paths.h>
#include <unistd.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <linux/route.h>
#include <sys/ioctl.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>
#include <dbus-1.0/dbus/dbus.h>

#include <netdb.h>
#include <linux/wireless.h>

#include <libkylog.h>

enum cardstat
{
    NCSTAT_LINK_UP,
    NCSTAT_LINK_DOWN,
    NCSTAT_ERROR
};

#define PROGNAME_WIDTH 20
#define SELINUX_WIDTH 50
#define PRG_HASH_SIZE 211

static struct prg_node
{
    struct prg_node *next;
    unsigned long inode;
    char name[PROGNAME_WIDTH];
    char scon[SELINUX_WIDTH];
} *prg_hash[PRG_HASH_SIZE];

static char prg_cache_loaded = 0;

#define PRG_HASHIT(x) ((x) % PRG_HASH_SIZE)

#define PRG_LOCAL_ADDRESS "local_address"
#define PRG_INODE "inode"
#define PRG_SOCKET_PFX "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
#define PRG_SOCKET_PFX2 "[0000]:"
#define PRG_SOCKET_PFX2l (strlen(PRG_SOCKET_PFX2))

#ifndef LINE_MAX
#define LINE_MAX 4096
#endif

#define PATH_PROC "/proc"
#define PATH_FD_SUFF "fd"
#define PATH_FD_SUFFl strlen(PATH_FD_SUFF)
#define PATH_PROC_X_FD PATH_PROC "/%s/" PATH_FD_SUFF
#define PATH_CMDLINE "cmdline"
#define PATH_CMDLINEl strlen(PATH_CMDLINE)

static inline char* lookup(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strlen(key) + 1;
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/proc", strlen("/proc"));
}

static char *safe_strncpy(char *dst, const char *src, size_t size)
{
    dst[size - 1] = '\0';
    return strncpy(dst, src, size - 1);
}

static void prg_cache_add(unsigned long inode, char *name, const char *scon)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node **pnp, *pn;

    prg_cache_loaded = 2;
    for (pnp = prg_hash + hi; (pn = *pnp); pnp = &pn->next)
    {
        if (pn->inode == inode)
        {
            /* Some warning should be appropriate here
               as we got multiple processes for one i-node */
            return;
        }
    }
    if (!(*pnp = (struct prg_node *)malloc(sizeof(**pnp))))
        return;
    pn = *pnp;
    pn->next = NULL;
    pn->inode = inode;
    safe_strncpy(pn->name, name, sizeof(pn->name));

    {
        int len = (strlen(scon) - sizeof(pn->scon)) + 1;
        if (len > 0)
            safe_strncpy(pn->scon, &scon[len + 1], sizeof(pn->scon));
        else
            safe_strncpy(pn->scon, scon, sizeof(pn->scon));
    }
}

static int extract_type_1_socket_inode(const char lname[], unsigned long *inode_p)
{

    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFXl + 3)
        return (-1);

    if (memcmp(lname, PRG_SOCKET_PFX, PRG_SOCKET_PFXl))
        return (-1);
    if (lname[strlen(lname) - 1] != ']')
        return (-1);

    {
        char inode_str[strlen(lname + 1)]; /* e.g. "12345" */
        const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname + PRG_SOCKET_PFXl, inode_str_len);
        inode_str[inode_str_len] = '\0';
        *inode_p = strtoul(inode_str, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return (-1);
    }
    return (0);
}

static int extract_type_2_socket_inode(const char lname[], unsigned long *inode_p)
{

    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFX2l + 1)
        return (-1);
    if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l))
        return (-1);

    {
        char *serr;

        *inode_p = strtoul(lname + PRG_SOCKET_PFX2l, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return (-1);
    }
    return (0);
}

static void parse_port_state(int *st, int port, FILE *fp)
{
    char line[1024] = "\0";
    int lnr = 0;
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128] = "0", local_addr[128] = "0";
    struct sockaddr_storage localsas, remsas;
    struct sockaddr_in *localaddr = (struct sockaddr_in *)&localsas;
    struct sockaddr_in *remaddr = (struct sockaddr_in *)&remsas;

    // #if HAVE_AFINET6
    //     char addr6[INET6_ADDRSTRLEN];
    //     struct in6_addr in6;
    //     extern struct aftype inet6_aftype;
    // #endif

    while (fgets(line, 1024, fp))
    {
        if (lnr++ == 0)
            continue;
        num = sscanf(line,
                     "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                     &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                     &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
        if (local_port == port)
        {
            *st = state;
            break;
        }
    }
    //     if (strlen(local_addr) > 8)
    //     {
    // #if HAVE_AFINET6
    //         /* Demangle what the kernel gives us */
    //         sscanf(local_addr, "%08X%08X%08X%08X",
    //                &in6.s6_addr32[0], &in6.s6_addr32[1],
    //                 &in6.s6_addr32[2], &in6.s6_addr32[3]);
    //         inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
    //         inet6_aftype.input(1, addr6, &localsas);
    //         sscanf(rem_addr, "%08X%08X%08X%08X",
    //                &in6.s6_addr32[0], &in6.s6_addr32[1],
    //                 &in6.s6_addr32[2], &in6.s6_addr32[3]);
    //         inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
    //         inet6_aftype.input(1, addr6, &remsas);
    //         localsas.ss_family = AF_INET6;
    //         remsas.ss_family = AF_INET6;
    // #endif
    //     }
    //     else
    //     {
    //         sscanf(local_addr, "%X", &localaddr->sin_addr.s_addr);
    //         sscanf(rem_addr, "%X", &remaddr->sin_addr.s_addr);
    //         localsas.ss_family = AF_INET;
    //         remsas.ss_family = AF_INET;
    //     }
}

int kdk_net_get_port_stat(int port)
{
    FILE *fp = fopen("/proc/net/tcp", "r");
    if (!fp)
        return -1;
    int state = 0;
    parse_port_state(&state, port, fp);
    fclose(fp);

    fp = fopen("/proc/net/tcp6", "r");
    if (!fp)
        return -1;
    parse_port_state(&state, port, fp);
    fclose(fp);

    return state;
}

int kdk_net_get_multiple_port_stat(int start, int end, int *result)
{
    if (end < start)
        return -1;
    for (int i = start; i <= end; i++)
    {
        result[i - start] = kdk_net_get_port_stat(i);
        if (-1 == result[i - start])
            return i - start;
    }
    return 0;
}

prouteMapList kdk_net_get_route()
{
    prouteMapList list = NULL, currnode = NULL, prenode = NULL;
    FILE *fp;
    char devname[64];
    unsigned long d, g, m;
    int r;
    int flgs, ref, use, metric, mtu, win, ir;

    fp = fopen("/proc/net/route", "r");

    /* Skip the first line. */
    r = fscanf(fp, "%*[^\n]\n");

    if (r < 0)
    {
        /* Empty line, read error, or EOF. Yes, if routing table
         * is completely empty, /proc/net/route has no header.
         */
        goto ERROR;
    }

    while (1)
    {
        r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
                   devname, &d, &g, &flgs, &ref, &use, &metric, &m,
                   &mtu, &win, &ir);

        if (r != 11)
        {
            if ((r < 0) && feof(fp))
            {
                /* EOF with no (nonspace) chars read. */
                break;
            }
        }
        /*
         * # cat /proc/net/route
         * Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT
         * eth0    00000000        013CA8C0        0003    0       0       0       00000000        0       0       0
         * eth0    003CA8C0        00000000        0001    0       0       0       00FFFFFF        0       0       0
         * 默认网关的特性: dst为0，netmask为0, Gateway不为0
         */
        if ((flgs & (RTF_GATEWAY | RTF_UP)) &&
            g != 0 && d == 0 && m == 0)
        {
            currnode = (prouteMapList)calloc(1, sizeof(routeMapList));
            if (!currnode)
            {
                kdk_net_free_route(list);
                goto ERROR;
            }
            struct sockaddr_in sin;
            memset(&sin, 0, sizeof(struct sockaddr_in));
            memcpy(&sin.sin_addr, &g, 4);
            strcpy(currnode->name, devname);
            strcpy(currnode->addr, inet_ntoa(sin.sin_addr));
            if (!list)
                list = currnode;
            else
                prenode->next = currnode;
            prenode = currnode;
        }
    }

ERROR:
    fclose(fp);
    return list;
}

void kdk_net_free_route(prouteMapList list)
{
    prouteMapList tmp = NULL;
    while (list)
    {
        tmp = list;
        list = list->next;
        free(tmp);
    }
}

pChain _kdk_net_get_iptable_rules()
{
    pChain result = NULL, tmp = NULL;
    char line[512] = "\0";
    FILE *fp = popen("iptables -L -n", "r");
    if (NULL == fp)
    {
        pclose(fp);
        return NULL;
    }
    while (fgets(line, 512, fp))
    {
        if (strstr(line, "Chain"))
        {
            // 申请Chain内存
            if (!result)
            {
                result = (pChain)calloc(1, sizeof(Chain));
                tmp = result;
            }
            else
            {
                tmp->next = (pChain)calloc(1, sizeof(Chain));
                tmp = tmp->next;
            }
            if (NULL == tmp)
            {
                kdk_net_free_chain(result);
                result = NULL;
                break;
            }
            int count = sscanf(line, "%*s %s (%[^)]", tmp->total, tmp->policy);
            continue;
        }

        if (!tmp)
            continue;

        if (strstr(line, "target     prot opt source               destination"))
            continue;
        char target[24] = "0", prot[4 + 1] = "0", opt[4 + 1] = "0", source[16] = "0", destination[16] = "0", option[128] = "0";
        pChainData tmpData;
        int count = sscanf(line, "%s %s %s %s %s %[^\n]", target, prot, opt, source, destination, option);
        if (count < 5)
        {
            continue;
        }
        // Chain下每条规则申请一个chainData结构体
        if (!tmp->data)
        {
            tmp->data = (pChainData)calloc(1, sizeof(chainData));
            tmpData = tmp->data;
        }
        else
        {
            tmpData->next = (pChainData)calloc(1, sizeof(chainData));
            tmpData = tmpData->next;
        }
        if (NULL == tmpData)
        {
            kdk_net_free_chain(result);
            result = NULL;
            break;
        }
        if (target)
            strcpy(tmpData->target, target);
        if (prot)
            strcpy(tmpData->prot, prot);
        if (opt)
            strcpy(tmpData->opt, opt);
        if (source)
            strcpy(tmpData->source, source);
        if (destination)
            strcpy(tmpData->destination, destination);
        if (option)
            strcpy(tmpData->option, option);
    }
    pclose(fp);
    return result;
}

pChain kdk_net_get_iptable_rules()
{
    char **ret = NULL;
    DBusConnection *conn;
    DBusError error;

    dbus_error_init(&error);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (NULL == conn)
    {
        goto out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/net", // object to call on
                                            "com.lingmo.lingmosdk.net",  // interface to call on
                                            "getFirewallState");               // method name

    if (info_msg == NULL)
    {
        fprintf(stderr, "DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    replyMsg = dbus_connection_send_with_reply_and_block(conn, info_msg, -1, &error);
    if (replyMsg == NULL)
    {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(info_msg);

    DBusMessageIter iter;
    // 解析回复消息
    if (!dbus_message_iter_init(replyMsg, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRING)
    {
        fprintf(stderr, "DBus reply parsing failed\n");
        goto out;
    }

    // 获取数组长度  获取的长度不对 弃用
    DBusMessageIter args;
    dbus_message_iter_recurse(&iter, &args);
    pChain result = NULL, tmp = NULL;

    while (dbus_message_iter_get_arg_type (&args) != DBUS_TYPE_INVALID) {
        char *iter_name = NULL;

        dbus_message_iter_get_basic(&args, &iter_name); // 获取name
        char total[128] = "0", policy[128] = "0", target[128] = "0", prot[128] = "0", opt[128] = "0", source[128] = "0", destination[128] = "0", option[128] = "0";
        char tmp_name[256] = "\0";
        char line[256] = "\0";
        if(iter_name)
            strcpy(line, iter_name);

        if(strstr(line, "total"))
        {
            strcpy(tmp_name, line);
            sscanf(tmp_name, "total:%[^,],policy: %[^\n]", total, policy);

            // 申请Chain内存
            if (!result)
            {
                result = (pChain)calloc(1, sizeof(Chain));
                tmp = result;
            }
            else
            {
                tmp->next = (pChain)calloc(1, sizeof(Chain));
                tmp = tmp->next;
            }
            if (NULL == tmp)
            {
                kdk_net_free_chain(result);
                result = NULL;
                break;
            }

            if(total)
            {
                strcpy(tmp->total, total);
            }

            if(policy)
            {
                strcpy(tmp->policy, policy);
            }
            dbus_message_iter_next(&args);
            continue;
        }

        if(strstr(line, "target"))
        {
            strcpy(tmp_name, line);
            char tmp_target[128] = "\0", tmp_prot[128] = "\0", tmp_opt[128] = "\0", tmp_source[128] = "\0", tmp_destination[128] = "\0", tmp_option[128] = "\0", other[128] = "\0";

            sscanf(tmp_name, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]", tmp_target, tmp_prot, tmp_opt, tmp_source, tmp_destination, tmp_option, other);

            if(tmp_target[0] != '\0')
            {
                char **fg = strsplit(tmp_target, ':');
                if (fg[1])
                {
                    strcpy(target, fg[1]);
                }
                free(fg);
            }
            
            if(tmp_prot[0] != '\0')
            {
                char **fg = strsplit(tmp_prot, ':');
                if (fg[1])
                {
                    strcpy(prot, fg[1]);
                }
                free(fg);
            }

            if(tmp_opt[0] != '\0')
            {
                char **fg = strsplit(tmp_opt, ':');
                if (fg[1])
                {
                    strcpy(opt, fg[1]);
                }
                free(fg);
            }

            if(tmp_source[0] != '\0')
            {
                char **fg = strsplit(tmp_source, ':');
                if (fg[1])
                {
                    strcpy(source, fg[1]);
                }
                free(fg);
            }

            if(tmp_destination[0] != '\0')
            {
                char **fg = strsplit(tmp_destination, ':');
                if (fg[1])
                {
                    strcpy(destination, fg[1]);
                }
                free(fg);
            }

            if(tmp_option[0] != '\0')
            {
                char **fg = strsplit(tmp_option, ':');
                if (fg[1])
                {
                    strcpy(option, fg[1]);
                }
                free(fg);
            }
            if(other[0] != '\0')
            {
                strcat(option, ",");
                strcat(option, other);
            }

        }

        if (!tmp)
            continue;

        pChainData tmpData;
        
        // Chain下每条规则申请一个chainData结构体
        if (!tmp->data)
        {
            tmp->data = (pChainData)calloc(1, sizeof(chainData));
            tmpData = tmp->data;
        }
        else
        {
            tmpData->next = (pChainData)calloc(1, sizeof(chainData));
            tmpData = tmpData->next;
        }
        if (NULL == tmpData)
        {
            kdk_net_free_chain(result);
            result = NULL;
            break;
        }
        if (target)
            strcpy(tmpData->target, target);
        if (prot)
            strcpy(tmpData->prot, prot);
        if (opt)
            strcpy(tmpData->opt, opt);
        if (source)
            strcpy(tmpData->source, source);
        if (destination)
            strcpy(tmpData->destination, destination);
        if (option)
            strcpy(tmpData->option, option);

        dbus_message_iter_next(&args);
    }
    
    // 释放reply
    dbus_message_unref(replyMsg);
    dbus_connection_close(conn);
    return result;

out:
    dbus_error_free(&error);
    if (conn)
        dbus_connection_close(conn);
    return NULL;
}

void kdk_net_free_chain(pChain list)
{
    pChain tmpChain = NULL;
    pChainData tmpData = NULL;
    while (list)
    {
        tmpChain = list;
        list = list->next;
        while (tmpChain->data)
        {
            tmpData = tmpChain->data;
            tmpChain->data = tmpData->next;
            free(tmpData);
        }

        free(tmpChain);
    }
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

    // close(sfd);
    if (stIf.ifr_ifru.ifru_flags & IFF_RUNNING)
        return NCSTAT_LINK_UP;

    return NCSTAT_LINK_DOWN;
}

void kdk_net_get_netmask(IN const char *nc, OUT char *mask)
{
    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, nc);
    int ret = ioctl(fd, SIOCGIFNETMASK, &ifr);
    if (0 != ret)
    {
        return;
    }
    struct sockaddr_in sin_addr;
    if (NCSTAT_LINK_UP == _get_card_stat(nc))
        strcpy(mask, (char *)inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr));
    else
        strcpy(mask, "-");
}

static const char *prg_cache_get(unsigned long inode)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node *pn;

    for (pn = prg_hash[hi]; pn; pn = pn->next)
        if (pn->inode == inode)
            return (pn->name);
    return ("-");
}

static void prg_cache_clear()
{
    struct prg_node **pnp, *pn;

    if (prg_cache_loaded == 2)
        for (pnp = prg_hash; pnp < prg_hash + PRG_HASH_SIZE; pnp++)
            while ((pn = *pnp))
            {
                *pnp = pn->next;
                free(pn);
            }
    prg_cache_loaded = 0;
}

static void prg_cache_load(void)
{
    char line[LINE_MAX], eacces = 0;
    int procfdlen, fd, cmdllen, lnamelen;
    char lname[30], cmdlbuf[512], finbuf[PROGNAME_WIDTH];
    unsigned long inode;
    const char *cs, *cmdlp;
    DIR *dirproc = NULL, *dirfd = NULL;
    struct dirent *direproc, *direfd;
#if HAVE_SELINUX
    security_context_t scon = NULL;
#endif

    prg_cache_loaded = 1;
    cmdlbuf[sizeof(cmdlbuf) - 1] = '\0';
    if (!(dirproc = opendir(PATH_PROC)))
        goto fail;
    while (errno = 0, direproc = readdir(dirproc))
    {
        for (cs = direproc->d_name; *cs; cs++)
            if (!isdigit(*cs))
                break;
        if (*cs)
            continue;
        procfdlen = snprintf(line, sizeof(line), PATH_PROC_X_FD, direproc->d_name);
        if (procfdlen <= 0 || procfdlen >= sizeof(line) - 5)
            continue;
        errno = 0;
        char canonical_filename[4096] = {"\0"};
        char filename[4096] = {"\0"};
        if (!realpath(line, canonical_filename) || !verify_file(canonical_filename))
        {
            closedir(dirproc);
            return;
        }
        dirfd = opendir(canonical_filename);
        if (!dirfd)
        {
            if (errno == EACCES)
                eacces = 1;
            continue;
        }
        canonical_filename[procfdlen] = '/';
        cmdlp = NULL;
        while ((direfd = readdir(dirfd)))
        {
            /* Skip . and .. */
            if (!isdigit(direfd->d_name[0]))
                continue;
            if (procfdlen + 1 + strlen(direfd->d_name) + 1 > sizeof(canonical_filename))
                continue;
            memcpy(canonical_filename + procfdlen - PATH_FD_SUFFl, PATH_FD_SUFF "/",
                   PATH_FD_SUFFl + 1);
            safe_strncpy(canonical_filename + procfdlen + 1, direfd->d_name,
                         sizeof(canonical_filename) - procfdlen - 1);
            lnamelen = readlink(canonical_filename, lname, sizeof(lname) - 1);
            if (lnamelen == -1)
                continue;
            lname[lnamelen] = '\0'; /*make it a null-terminated string*/

            if (extract_type_1_socket_inode(lname, &inode) < 0)
                if (extract_type_2_socket_inode(lname, &inode) < 0)
                    continue;

            if (!cmdlp)
            {
                if (procfdlen - PATH_FD_SUFFl + PATH_CMDLINEl >=
                    sizeof(canonical_filename) - 5)
                    continue;
                safe_strncpy(canonical_filename + procfdlen - PATH_FD_SUFFl, PATH_CMDLINE,
                             sizeof(canonical_filename) - procfdlen + PATH_FD_SUFFl);
                if (!realpath(canonical_filename, filename) || !verify_file(filename))
                {
                    closedir(dirfd);
                    closedir(dirproc);
                    return;
                }
                fd = open(filename, O_RDONLY);
                if (fd < 0)
                    continue;
                cmdllen = read(fd, cmdlbuf, sizeof(cmdlbuf) - 1);
                if (close(fd))
                    continue;
                if (cmdllen == -1)
                    continue;
                if (cmdllen < sizeof(cmdlbuf) - 1)
                    cmdlbuf[cmdllen] = '\0';
                if (cmdlbuf[0] == '/' && (cmdlp = strrchr(cmdlbuf, '/')))
                    cmdlp++;
                else
                    cmdlp = cmdlbuf;
            }

            snprintf(finbuf, sizeof(finbuf), "%s/%s", direproc->d_name, cmdlp);
#if HAVE_SELINUX
            if (getpidcon(atoi(direproc->d_name), &scon) == -1)
            {
                scon = xstrdup("-");
            }
            prg_cache_add(inode, finbuf, scon);
            freecon(scon);
#else
            prg_cache_add(inode, finbuf, "-");
#endif
        }
        closedir(dirfd);
        dirfd = NULL;
    }
    if (dirproc)
        closedir(dirproc);
    if (dirfd)
        closedir(dirfd);
    if (!eacces)
        return;
    if (prg_cache_loaded == 1)
    {
    fail:
        fprintf(stderr, "(No info could be read for \"-p\": geteuid()=%d but you should be root.)\n",
                geteuid());
    }
    else
        fprintf(stderr, "(Not all processes could be identified, non-owned process info\n"
                        " will not be shown, you would have to be root to see it all.)\n");
}

char **_kdk_net_get_proc_port()
{
    prg_cache_load();
    char **result = NULL;
    int index = -1;

    char line[1024] = "\0";
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128] = "0", local_addr[128] = "0";

    FILE *fp = fopen("/proc/net/tcp", "r");
    if (!fp)
        return NULL;
    while (fgets(line, 1024, fp))
    {
        num = sscanf(line,
                     "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                     &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                     &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
        const char *proc = prg_cache_get(inode);
        if (0 == strcmp(proc, "-"))
            continue;

        index++;
        result = (char **)realloc(result, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
        result[index] = (char *)calloc(1, 64);
        sprintf(result[index], "%d/%s/%s", local_port, proc, "tcp");
    }
    fclose(fp);

    fp = fopen("/proc/net/tcp6", "r");
    if (!fp)
    {
        while (result && (index >= 0))
        {
            free(result[index--]);
        }
        free(result);
        return NULL;
    }
    while (fgets(line, 1024, fp))
    {
        num = sscanf(line,
                     "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                     &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                     &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
        const char *proc = prg_cache_get(inode);
        if (0 == strcmp(proc, "-"))
            continue;

        index++;
        result = (char **)realloc(result, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
        result[index] = (char *)calloc(1, 64);
        sprintf(result[index], "%d/%s/%s", local_port, proc, "tcp6");
    }
    fclose(fp);

    fp = fopen("/proc/net/udp", "r");
    if (!fp)
    {
        while (result && (index >= 0))
        {
            free(result[index--]);
        }
        free(result);
        return NULL;
    }
    while (fgets(line, 1024, fp))
    {
        num = sscanf(line,
                     "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                     &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                     &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
        const char *proc = prg_cache_get(inode);
        if (0 == strcmp(proc, "-"))
            continue;

        index++;
        result = (char **)realloc(result, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
        result[index] = (char *)calloc(1, 64);
        sprintf(result[index], "%d/%s/%s", local_port, proc, "udp");
    }
    fclose(fp);

    fp = fopen("/proc/net/udp6", "r");
    if (!fp)
    {
        while (result && (index >= 0))
        {
            free(result[index--]);
        }
        free(result);
        return NULL;
    }
    while (fgets(line, 1024, fp))
    {
        num = sscanf(line,
                     "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                     &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                     &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
        const char *proc = prg_cache_get(inode);
        if (0 == strcmp(proc, "-"))
            continue;

        index++;
        result = (char **)realloc(result, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
        result[index] = (char *)calloc(1, 64);
        sprintf(result[index], "%d/%s/%s", local_port, proc, "udp6");
    }
    fclose(fp);

    result[index + 1] = NULL;

    prg_cache_clear();
    return result;
}

char **kdk_net_get_proc_port()
{
    DBusConnection *conn;
    DBusError error;

    dbus_error_init(&error);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (NULL == conn)
    {
        goto out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessageIter iter, args;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/net", // object to call on
                                            "com.lingmo.lingmosdk.net",  // interface to call on
                                            "getProcPort");               // method name

    if (info_msg == NULL)
    {
        fprintf(stderr, "DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    replyMsg = dbus_connection_send_with_reply_and_block(conn, info_msg, -1, &error);
    if (replyMsg == NULL)
    {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(info_msg);

    // 解析回复消息
    if (!dbus_message_iter_init(replyMsg, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRING)
    {
        fprintf(stderr, "DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(replyMsg);

    // 解析数组中的每个元素

    dbus_message_iter_recurse(&iter, &args);
    char **result = NULL;
    int index = -1;

    while (dbus_message_iter_get_arg_type (&args) != DBUS_TYPE_INVALID) {
        char *iter_name = NULL;

        dbus_message_iter_get_basic(&args, &iter_name); // 获取name

        index++;
        result = (char **)realloc(result, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
        result[index] = (char *)calloc(1, 64);
        strcpy(result[index], iter_name);

        dbus_message_iter_next(&args);
    }
    result[index + 1] = NULL;
    return result;

out:
    dbus_error_free(&error);
    if (conn)
    {
        dbus_connection_close(conn);
    }

    return _kdk_net_get_proc_port();
}

char **kdk_net_get_up_port()
{
    char **result = NULL;
    int index = -1;

    char line[1024] = "\0";
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128] = "0", local_addr[128] = "0";

    FILE *fp = fopen("/proc/net/tcp", "r");
    if (!fp)
        return NULL;
    while (fgets(line, 1024, fp))
    {
        num = sscanf(line,
                     "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                     &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                     &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
        if (0x0a == state)
        {
            int i = 0;
            int flag = 1;
            while (i <= index) // 查重
            {
                if (local_port == atoi(result[i++]))
                    flag = 0; // 有重复  flag置为1
            }

            if (flag)
            {
                index++;
                char **tmp = (char **)realloc(result, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
                if (!tmp)
                    goto error;
                result = tmp;
                result[index] = (char *)calloc(1, 8);
                if (!result[index])
                    goto error;
                sprintf(result[index], "%d", local_port);
            }
        }
    }
    fclose(fp);

    fp = fopen("/proc/net/tcp6", "r");
    if (!fp)
        return NULL;
    while (fgets(line, 1024, fp))
    {
        num = sscanf(line,
                     "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                     &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                     &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
        if (0x0a == state)
        {
            int i = 0;
            int flag = 1;
            while (i <= index) // 查重
            {
                if (local_port == atoi(result[i++]))
                    flag = 0; // 有重复  flag置为1
            }

            if (flag)
            {
                index++;
                result = (char **)realloc(result, sizeof(char *) * (index + 1 + 1)); // 以NULL结尾
                if (!result)
                    goto error;
                result[index] = (char *)calloc(1, 8);
                if (!result[index])
                    goto error;
                sprintf(result[index], "%d", local_port);
            }
        }
    }
    fclose(fp);
    result[index + 1] = NULL;

    return result;
error:
    // for (int i = 0; result[i]; i++)
    // {
    //     free(result[i]);
    // }
    while (index)
    {
        free(result[index - 1]);
        index--;
    }
    free(result);
    result = NULL;
    return result;
}

char *kdk_net_get_hosts()
{
    FILE *fp = fopen("/etc/hosts", "r");
    if (fp == NULL)
    {
        return NULL;
    }

    char *hosts = (char *)malloc(sizeof(char) * 64);
    if (hosts == NULL)
    {
        fclose(fp);
        return NULL;
    }
    memset(hosts, 0, 64);
    char line[4096] = "\0";
    memset(hosts, 0, sizeof(hosts));
    while (fgets(line, sizeof(line), fp))
    {
        strcat(hosts, line);
        if (line[0] == '\n')
        {
            break;
        }
    }
    fclose(fp);
    strstripspace(hosts);
    return hosts;
}

char *kdk_net_get_hosts_domain()
{
    FILE *fp = fopen("/etc/hosts", "r");
    if (fp == NULL)
    {
        return NULL;
    }

    char line[4096] = "\0";
    char strtmp[4096] = "\0";
    int i = 0;
    int j = 0;
    while (fgets(line, sizeof(line), fp))
    {
        if(i == 1)
        {
            if(j != 6)
            {
                j++;
            }
        }
        else{
            if(strstr(line, "IPv6 capable hosts"))
            {
                i = 1;
            }
            else{
                continue;
            }
        }
        if((j == 6) && (strlen(line) != 0))
        {
            strcat(strtmp, line);
        }
    }

    int len = strlen(strtmp);

    char *hosts = (char *)malloc(sizeof(char) * len + 1);
    if (hosts == NULL)
    {
        fclose(fp);
        return NULL;
    }
    memset(hosts, 0, len + 1);
    strcpy(hosts, strtmp);
    fclose(fp);
    strstripspace(hosts);
    return hosts;
}

char **kdk_net_get_resolv_conf()
{
    char nserver[64] = "\0";
    char addr[64] = "\0";
    char **result = NULL;
    result = (char **)realloc(result, sizeof(char *) * 256); // 以NULL结尾
    if (!result)
    {
        return NULL;
    }
    int index = 0;
    FILE *fp = fopen("/etc/resolv.conf", "r");
    if (fp == NULL)
    {
        free(result);
        return NULL;
    }

    char line[4096] = "\0";
    while (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, "nameserver"))
        {
            result[index] = (char *)malloc(sizeof(char) * 256);
            if (!result[index])
            {
                fclose(fp);
                free(result);
                return NULL;
            }
            sscanf(line, "%s %s", nserver, addr);
            strcpy(result[index], addr);
            index++;
        }
    }
    fclose(fp);
    result[index] = NULL;
    return result;
}

inline void kdk_net_freeall(char **list)
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

int kdk_net_get_link_type()
{
    enum NetLinkType result = LINK_TYPE_UNKNOWN;

    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->Socket creation failed\n", __func__);
        return result;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
    {
        klog_err("[%s]->ioctl failed\n", __func__);
        close(sockfd);
        return result;
    }

    struct ifreq *it = ifc.ifc_req;
    const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    int i = 0;
    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
        {
            klog_err("[%s]->ioctl failed\n", __func__);
            continue;
        }
        if (!(ifr.ifr_flags & IFF_UP) || (ifr.ifr_flags & IFF_LOOPBACK))
        {
            continue; // Skip loopback and inactive interfaces
        }
#define SIOCGIWNAME 0x8B01
        if (ioctl(sockfd, SIOCGIWNAME, &ifr) == 0)
            result += LINK_TYPE_WIFI;
        else
            result += LINK_TYPE_ETHERNET;
#undef SIOCGIWNAME
    }

    close(sockfd);

    return result;
}

int kdk_net_get_link_status(char *ip, char *port)
{
    if (NULL == ip || NULL == port)
        return 0;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port));
    inet_pton(AF_INET, ip, &server_address.sin_addr);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->Socket creation failed\n", __func__);
        return 0;
    }

    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        close(sockfd);
        return 0;
    }

    close(sockfd);
    return 1;
}

char **kdk_net_get_link_ncNmae()
{
    char **result = NULL;

    struct ifreq ifr;
    struct ifconf ifc;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->Socket creation failed\n", __func__);
        return NULL;
    }

    char buf[1024];
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
    {
        klog_err("[%s]->ioctl failed\n", __func__);
        close(sockfd);
        return NULL;
    }

    struct ifreq *it = ifc.ifc_req;
    const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    result = calloc(end - it, sizeof(char *));
    if (NULL == result)
    {
        close(sockfd);
        return NULL;
    }

    int i = 0;
    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
        {
            klog_err("[%s]->ioctl failed\n", __func__);
            continue;
        }
        if (!(ifr.ifr_flags & IFF_UP) || (ifr.ifr_flags & IFF_LOOPBACK))
        {
            continue; // Skip loopback and inactive interfaces
        }
        result[i++] = strdup(ifr.ifr_name);
    }

    close(sockfd);
    return result;
}

char *kdk_net_get_primary_conType()
{
    char *result = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        klog_err("[%s]-> DBus connection error: %s\n", __func__, error.message);
        return NULL;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                           "/org/freedesktop/NetworkManager",
                                           "org.freedesktop.DBus.Properties",
                                           "Get");
    if (message == NULL)
    {
        klog_err("[%s]-> DBus message allocation failed\n", __func__);
        dbus_connection_unref(connection);
        return NULL;
    }

    char *interface = "org.freedesktop.NetworkManager";
    char *property = "PrimaryConnectionType";
    dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL)
    {
        klog_err("[%s]-> DBus reply error: %s\n", __func__, error.message);
        dbus_message_unref(message);
        dbus_connection_unref(connection);
        return NULL;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
    {
        klog_err("[%s]-> DBus reply parsing failed\n");
        dbus_message_unref(reply);
        dbus_connection_unref(connection);
        return NULL;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    dbus_message_iter_recurse(&iter, &iter);
    dbus_message_iter_get_basic(&iter, &result);

    dbus_connection_unref(connection);

    return strdup(result);
}

char *kdk_net_get_wifi_mode(char *nc)
{
    if (NULL == nc)
        return NULL;

    char *result = NULL;

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->Socket creation failed\n", __func__);
        return NULL;
    }

    struct iwreq wrq;
    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, nc, IFNAMSIZ);

    if (ioctl(sockfd, SIOCGIWMODE, &wrq) < 0)
    {
        klog_err("[%s]->ioctl failed\n", __func__);
        close(sockfd);
        return NULL;
    }

    switch (wrq.u.mode)
    {
    case IW_MODE_MASTER:
        result = strdup("Master");
        break;
    case IW_MODE_INFRA:
        result = strdup("Managed");
        break;
    case IW_MODE_ADHOC:
        result = strdup("Ad-Hoc");
        break;
    case IW_MODE_MONITOR:
        result = strdup("Monitor");
        break;
    default:
        result = strdup("Unknown");
        break;
    }
    close(sockfd);

    return result;
}

char *kdk_net_get_wifi_freq(char *nc)
{
    if (NULL == nc)
        return NULL;

    char *result = NULL;

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->open socket failed\n", __func__);
        return NULL;
    }

    struct iwreq req;
    memset(&req, 0, sizeof(struct iwreq));
    strncpy(req.ifr_name, nc, IFNAMSIZ);

    // 获取工作频率
    if (ioctl(sockfd, SIOCGIWFREQ, &req) < 0)
        klog_err("[%s]->ioctl failed\n", __func__);
    else
    {
        char tmp[32];
        sprintf(tmp, "%d", req.u.freq.m);
        result = strdup(tmp);
    }
    close(sockfd);

    return result;
}

char *kdk_net_get_wifi_channel(char *nc)
{
    if (NULL == nc)
        return NULL;

    char *result = NULL;

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->open socket failed\n", __func__);
        return NULL;
    }

    struct iwreq req;
    memset(&req, 0, sizeof(struct iwreq));
    strncpy(req.ifr_name, nc, IFNAMSIZ);

    // 获取通道
    if (ioctl(sockfd, SIOCGIWFREQ, &req) < 0)
        klog_err("[%s]->ioctl failed\n", __func__);
    else
    {
        char tmp[32];
        sprintf(tmp, "%d", req.u.freq.i);
        result = strdup(tmp);
    }
    close(sockfd);

    return result;
}

char *kdk_net_get_wifi_rate(char *nc)
{
    if (NULL == nc)
        return NULL;

    char *result = NULL;

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->open socket failed\n", __func__);
        return NULL;
    }

    struct iwreq req;
    memset(&req, 0, sizeof(struct iwreq));
    strncpy(req.ifr_name, nc, IFNAMSIZ);

    // 获取工作速率
    if (ioctl(sockfd, SIOCGIWRATE, &req) < 0)
        klog_err("[%s]->ioctl failed\n", __func__);
    else
    {
        char tmp[32];
        sprintf(tmp, "%d", req.u.bitrate.value);
        result = strdup(tmp);
    }
    close(sockfd);

    return result;
}

char *kdk_net_get_wifi_sens(char *nc)
{
    if (NULL == nc)
        return NULL;

    char *result = NULL;

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        klog_err("[%s]->open socket failed\n", __func__);
        return NULL;
    }
    
    struct iwreq req;
    memset(&req, 0, sizeof(struct iwreq));
    strncpy(req.ifr_name, nc, IFNAMSIZ);

    struct iw_range range;
    req.u.data.pointer = &range;
    req.u.data.length = sizeof(struct iw_range);
    req.u.data.flags = 0;

    // 获取接收灵敏度下限
    if (ioctl(sockfd, SIOCGIWRANGE, &req) < 0)
        klog_err("[%s]->ioctl failed %s\n", __func__, strerror(errno));
    else
    {
        char tmp[32];
        sprintf(tmp, "%d", range.sensitivity);
        result = strdup(tmp);
    }
    close(sockfd);

    return result;
}

char **kdk_net_get_addr_by_name(char *name)
{
    if (NULL == name)
        return NULL;

    char **result = NULL;

    struct hostent *host_entry;
    struct in_addr addr;

    // 将域名解析为IP地址
    host_entry = gethostbyname(name);
    if (host_entry == NULL)
        klog_err("[%s]->无法解析域名\n", __func__);
    else
    {
        int i = 0;
        while (host_entry->h_addr_list[i])
            i++;

        result = calloc(i + 1, sizeof(char *));

        for (int j = 0; j < i; j++)
        {
            addr.s_addr = *(unsigned long *)host_entry->h_addr_list[0];
            result[j] = strdup(inet_ntoa(addr));
        }
    }
    return result;
}

char *kdk_net_get_name_by_addr(char *ip)
{
    char *result = NULL;

    struct hostent *host_entry;
    struct in_addr addr;

    struct hostent *host_entry_ip;
    host_entry_ip = gethostbyaddr(ip, sizeof(ip), AF_INET);
    if (host_entry_ip == NULL)
        klog_err("[%s]->无法解析IP地址\n", __func__);
    else
        result = strdup(host_entry_ip->h_name);

    return result;
}

static int has_dhcp_config(const char *dhcp_type)
{
    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        klog_err("[%s]-> DBus connection error: %s\n", __func__, error.message);
        return 0;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                           "/org/freedesktop/NetworkManager",
                                           "org.freedesktop.DBus.Introspectable",
                                           "Introspect");
    if (message == NULL)
    {
        klog_err("[%s]-> DBus message allocation failed\n", __func__);
        dbus_connection_unref(connection);
        return 0;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL)
    {
        klog_err("[%s]-> DBus reply error: %s\n", __func__, error.message);
        dbus_message_unref(message);
        dbus_connection_unref(connection);
        return 0;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
    {
        klog_err("[%s]-> DBus reply parsing failed\n");
        dbus_message_unref(reply);
        dbus_connection_unref(connection);
        return 0;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    char *introspect = NULL;
    dbus_message_iter_get_basic(&iter, &introspect);
    int result = strstr(introspect, dhcp_type) ? 1 : 0;

    dbus_connection_unref(connection);

    return result;
}

Dhcp4Config *kdk_net_get_ipv4_dhcp_config()
{
    if (0 == has_dhcp_config("DHCP4Config"))
        return NULL;

    Dhcp4Config *result = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        klog_err("[%s]-> DBus connection error: %s\n", __func__, error.message);
        return 0;
    }

    for (int i = 0; 1; i++)
    {
        char path[128];
        memset(path, 0, 128);
        sprintf(path, "/org/freedesktop/NetworkManager/DHCP4Config/%d", i);
        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                               path,
                                               "org.freedesktop.DBus.Properties",
                                               "Get");

        char *interface = "org.freedesktop.NetworkManager.DHCP4Config";
        char *property = "Options";
        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply != NULL)
        {
            break;
        }
        dbus_error_free(&error);
        dbus_message_unref(message);
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
    {
        klog_err("[%s]-> DBus reply parsing failed\n");
        dbus_message_unref(reply);
        dbus_connection_unref(connection);
        return 0;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    result = calloc(1, sizeof(Dhcp4Config));
    if (NULL == result)
    {
        dbus_connection_unref(connection);
        return NULL;
    }

    dbus_message_iter_recurse(&iter, &iter);
    if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY)
    {
        dbus_message_iter_recurse(&iter, &iter);
        while (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INVALID)
        {
            if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_DICT_ENTRY)
            {
                DBusMessageIter dict_iter;
                dbus_message_iter_recurse(&iter, &dict_iter);
                while (dbus_message_iter_get_arg_type(&dict_iter) != DBUS_TYPE_INVALID)
                {
                    if (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_STRING)
                    {
                        char *key = NULL;
                        dbus_message_iter_get_basic(&dict_iter, &key);

                        dbus_message_iter_next(&dict_iter);

                        DBusMessageIter value_iter;
                        dbus_message_iter_recurse(&dict_iter, &value_iter);

                        char *value = NULL;
                        dbus_message_iter_get_basic(&value_iter, &value);

                        if (0 == strcmp(key, "broadcast_address"))
                            result->broadcast_address = strdup(value);
                        else if (0 == strcmp(key, "dad_wait_time"))
                            result->dad_wait_time = atoi(value);
                        else if (0 == strcmp(key, "dhcp_lease_time"))
                            result->dhcp_lease_time = atoi(value);
                        else if (0 == strcmp(key, "dhcp_message_type"))
                            result->dhcp_message_type = atoi(value);
                        else if (0 == strcmp(key, "dhcp_server_identifier"))
                            result->dhcp_server_identifier = strdup(value);
                        else if (0 == strcmp(key, "domain_name_servers"))
                            result->domain_name_servers = strdup(value);
                        else if (0 == strcmp(key, "expiry"))
                            result->expiry = strdup(value);
                        else if (0 == strcmp(key, "ip_address"))
                            result->ip_address = strdup(value);
                        else if (0 == strcmp(key, "network_number"))
                            result->network_number = strdup(value);
                        else if (0 == strcmp(key, "next_server"))
                            result->next_server = strdup(value);
                        else if (0 == strcmp(key, "requested_broadcast_address"))
                            result->requested_broadcast_address = atoi(value);
                        else if (0 == strcmp(key, "requested_domain_name"))
                            result->requested_domain_name = atoi(value);
                        else if (0 == strcmp(key, "requested_domain_name_servers"))
                            result->requested_domain_name_servers = atoi(value);
                        else if (0 == strcmp(key, "requested_domain_search"))
                            result->requested_domain_search = atoi(value);
                        else if (0 == strcmp(key, "requested_host_name"))
                            result->requested_host_name = atoi(value);
                        else if (0 == strcmp(key, "requested_interface_mtu"))
                            result->requested_interface_mtu = atoi(value);
                        else if (0 == strcmp(key, "requested_ms_classless_static_routes"))
                            result->requested_ms_classless_static_routes = atoi(value);
                        else if (0 == strcmp(key, "requested_netbios_name_servers"))
                            result->requested_netbios_name_servers = atoi(value);
                        else if (0 == strcmp(key, "requested_netbios_scope"))
                            result->requested_netbios_scope = atoi(value);
                        else if (0 == strcmp(key, "requested_ntp_servers"))
                            result->requested_ntp_servers = atoi(value);
                        else if (0 == strcmp(key, "requested_rfc3442_classless_static_routes"))
                            result->requested_rfc3442_classless_static_routes = atoi(value);
                        else if (0 == strcmp(key, "requested_root_path"))
                            result->requested_root_path = atoi(value);
                        else if (0 == strcmp(key, "requested_routers"))
                            result->requested_routers = atoi(value);
                        else if (0 == strcmp(key, "requested_static_routes"))
                            result->requested_static_routes = atoi(value);
                        else if (0 == strcmp(key, "requested_subnet_mask"))
                            result->requested_subnet_mask = atoi(value);
                        else if (0 == strcmp(key, "requested_time_offset"))
                            result->requested_time_offset = atoi(value);
                        else if (0 == strcmp(key, "requested_wpad"))
                            result->requested_wpad = atoi(value);
                        else if (0 == strcmp(key, "routers"))
                            result->routers = strdup(value);
                        else if (0 == strcmp(key, "server_name"))
                            result->server_name = strdup(value);
                        else if (0 == strcmp(key, "subnet_mask"))
                            result->subnet_mask = strdup(value);
                        else
                            klog_err("[%s]-> %s is not in structure\n", __func__, key);
                    }
                    dbus_message_iter_next(&dict_iter);
                }
            }
            dbus_message_iter_next(&iter);
        }
    }

    dbus_connection_unref(connection);
    return result;
}

Dhcp6Config *kdk_net_get_ipv6_dhcp_config()
{
    if (0 == has_dhcp_config("DHCP6Config"))
        return NULL;

    Dhcp6Config *result = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        klog_err("[%s]-> DBus connection error: %s\n", __func__, error.message);
        return 0;
    }

    for (int i = 0; 1; i++)
    {
        char path[128];
        memset(path, 0, 128);
        sprintf(path, "/org/freedesktop/NetworkManager/DHCP6Config/%d", i);
        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                               path,
                                               "org.freedesktop.DBus.Properties",
                                               "Get");

        char *interface = "org.freedesktop.NetworkManager.DHCP6Config";
        char *property = "Options";
        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply != NULL)
        {
            break;
        }
        dbus_error_free(&error);
        dbus_message_unref(message);
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
    {
        klog_err("[%s]-> DBus reply parsing failed\n");
        dbus_message_unref(reply);
        dbus_connection_unref(connection);
        return 0;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    result = calloc(1, sizeof(Dhcp6Config));
    if (NULL == result)
    {
        dbus_connection_unref(connection);
        return NULL;
    }

    dbus_message_iter_recurse(&iter, &iter);
    if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY)
    {
        dbus_message_iter_recurse(&iter, &iter);
        while (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INVALID)
        {
            if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_DICT_ENTRY)
            {
                DBusMessageIter dict_iter;
                dbus_message_iter_recurse(&iter, &dict_iter);
                while (dbus_message_iter_get_arg_type(&dict_iter) != DBUS_TYPE_INVALID)
                {

                    if (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_STRING)
                    {
                        char *key = NULL;
                        dbus_message_iter_get_basic(&dict_iter, &key);

                        dbus_message_iter_next(&dict_iter);

                        DBusMessageIter value_iter;
                        dbus_message_iter_recurse(&dict_iter, &value_iter);

                        char *value = NULL;
                        dbus_message_iter_get_basic(&value_iter, &value);

                        if (0 == strcmp(key, "dad_wait_time"))
                            result->dad_wait_time = atoi(value);
                        else if (0 == strcmp(key, "dhcp6_client_id"))
                            result->dhcp6_client_id = strdup(value);
                        else if (0 == strcmp(key, "dhcp6_name_servers"))
                            result->dhcp6_name_servers = strdup(value);
                        else if (0 == strcmp(key, "dhcp6_server_id"))
                            result->dhcp6_server_id = strdup(value);
                        else if (0 == strcmp(key, "requested_dhcp6_client_id"))
                            result->requested_dhcp6_client_id = atoi(value);
                        else if (0 == strcmp(key, "requested_dhcp_domain_search"))
                            result->requested_dhcp_domain_search = atoi(value);
                        else if (0 == strcmp(key, "requested_dhp6_name_servers"))
                            result->requested_dhp6_name_servers = atoi(value);
                        // else if (0 == strcmp(key, "ip_address"))
                        //     result->ip_address = strdup(value);
                        // else if (0 == strcmp(key, "network_number"))
                        //     result->network_number = strdup(value);
                        // else if (0 == strcmp(key, "next_server"))
                        //     result->next_server = strdup(value);
                        // else if (0 == strcmp(key, "requested_broadcast_address"))
                        //     result->requested_broadcast_address = atoi(value);
                        // else if (0 == strcmp(key, "requested_domain_name"))
                        //     result->requested_domain_name = atoi(value);
                        // else if (0 == strcmp(key, "requested_domain_name_servers"))
                        //     result->requested_domain_name_servers = atoi(value);
                        // else if (0 == strcmp(key, "requested_domain_search"))
                        //     result->requested_domain_search = atoi(value);
                        // else if (0 == strcmp(key, "requested_host_name"))
                        //     result->requested_host_name = atoi(value);
                        // else if (0 == strcmp(key, "requested_interface_mtu"))
                        //     result->requested_interface_mtu = atoi(value);
                        // else if (0 == strcmp(key, "requested_ms_classless_static_routes"))
                        //     result->requested_ms_classless_static_routes = atoi(value);
                        // else if (0 == strcmp(key, "requested_netbios_name_servers"))
                        //     result->requested_netbios_name_servers = atoi(value);
                        // else if (0 == strcmp(key, "requested_netbios_scope"))
                        //     result->requested_netbios_scope = atoi(value);
                        // else if (0 == strcmp(key, "requested_ntp_servers"))
                        //     result->requested_ntp_servers = atoi(value);
                        // else if (0 == strcmp(key, "requested_rfc3442_classless_static_routes"))
                        //     result->requested_rfc3442_classless_static_routes = atoi(value);
                        // else if (0 == strcmp(key, "requested_root_path"))
                        //     result->requested_root_path = atoi(value);
                        // else if (0 == strcmp(key, "requested_static_routes"))
                        //     result->requested_static_routes = atoi(value);
                        // else if (0 == strcmp(key, "requested_subnet_mask"))
                        //     result->requested_subnet_mask = atoi(value);
                        // else if (0 == strcmp(key, "requested_time_offset"))
                        //     result->requested_time_offset = atoi(value);
                        // else if (0 == strcmp(key, "requested_wpad"))
                        //     result->requested_wpad = atoi(value);
                        // else if (0 == strcmp(key, "routers"))
                        //     result->routers = strdup(value);
                        // else if (0 == strcmp(key, "server_name"))
                        //     result->server_name = strdup(value);
                        // else if (0 == strcmp(key, "subnet_mask"))
                        //     result->subnet_mask = strdup(value);
                        else
                            klog_err("[%s]-> %s is not in structure\n", __func__, key);
                    }
                    dbus_message_iter_next(&dict_iter);
                }
            }
            dbus_message_iter_next(&iter);
        }
    }

    dbus_connection_unref(connection);
    return result;
}
