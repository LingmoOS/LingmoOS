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


#include <stdio.h>
#include <netinet/in.h>
//#include <net/if.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
//#include <linux/sockios.h>
//#include <linux/ioctl.h>
//#include <asm/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/route.h>
#include <netinet/ether.h>
#include <net/ethernet.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <stdlib.h>
#include "lingmo-network-interface.h"

//获取网络接口信息
int lingmo_network_network_getifacename(char **if_name)
{
    int i = 0;
    int sockfd;
    struct ifconf ifconf;
    struct ifreq *ifreq;
    unsigned char buf[1024];

    //初始化ifconf
    ifconf.ifc_len = 1024;
    //字符串指针ifconf.ifc_buf指向buf
    ifconf.ifc_buf = buf;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return 0;
    }

    //获取所有接口信息
    ioctl(sockfd, SIOCGIFCONF, &ifconf);
    //    printf("%s\n",ifconf.ifc_buf);

    //逐个获取Ip地址
    //结构体指针ifreq指向buf，即ifconf.ifc_buf
    ifreq = (struct ifreq*)buf;
    int number=ifconf.ifc_len/sizeof(struct ifreq);
    printf("%d\n",number);

    for(i = number; i>0; i--)
    {
//        printf("name = [%s] \n",ifreq->ifr_name);

        int j=number-i;
        if_name[j]=ifreq->ifr_name;
//        printf("if_name[%d]:%s\n",j,if_name[j]);

        ifreq++;
    }
    return number;
}

//获取ip地址
int lingmo_network_network_getipaddr(char *if_name,char *ipaddr)
{
    int sock_ip;
    struct sockaddr_in sin_ip;
    struct ifreq ifr_ip;
    if((sock_ip=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("sockket error");
        return -1;
    }

    strcpy(ifr_ip.ifr_name,if_name);

    if(ioctl(sock_ip,SIOCGIFADDR,&ifr_ip)<0)//直接获取ip地址
    {
        perror("Not setup interface\n");
        return -1;
    }

    memcpy(&sin_ip,&ifr_ip.ifr_addr,sizeof(sin_ip));
    strcpy(ipaddr,inet_ntoa(sin_ip.sin_addr));//#include <arpa/inet.h>
    close(sock_ip);
    return 0;

}

//设置ip地址
int lingmo_network_network_setip(char *if_name,char *ipaddr)
{
    int sock_ip;
    struct sockaddr_in sin_ip; //#include <netinet/in.h>
    struct ifreq ifr_ip; //#include <net/if.h>

//    bzero(&ifr_ip, sizeof(ifr_ip)); //#include <string.h>

    if(ipaddr==NULL)
        return -1;

    if((sock_ip=socket(AF_INET,SOCK_STREAM,0))==-1)//#include <sys/socket.h>
    {
        perror("Create socket failure!\n");
        return -1;
    }

    memset(&sin_ip,0,sizeof(sin_ip));
    memset(&ifr_ip.ifr_name,0,sizeof(ifr_ip));
    strcpy(ifr_ip.ifr_name,if_name);
//    printf("-----%d------\n",ioctl(sock_ip,SIOCSIFADDR,&ifr_ip));

    sin_ip.sin_family=AF_INET;
    sin_ip.sin_addr.s_addr=inet_addr(ipaddr);
    memcpy(&ifr_ip.ifr_addr,&sin_ip, sizeof(sin_ip));

//    printf("-----%d------\n",ioctl(sock_ip,SIOCSIFADDR,&ifr_ip));

    //此处非root用户运行报错，待解
    if(ioctl(sock_ip,SIOCSIFADDR,&ifr_ip)<0)
    {
        perror("Not setup interface\n");
        return -1;
    }

    close(sock_ip);
    return 0;
}

//获取广播地址
int lingmo_network_network_getbrdaddr(char *if_name,char *brdaddr)
{
    int sock_brdaddr;
    struct sockaddr_in sin_brd;
    struct ifreq ifr_brd;
    if((sock_brdaddr=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("sockket error");
        return -1;
    }

    strcpy(ifr_brd.ifr_name,if_name);

    if(ioctl(sock_brdaddr,SIOCGIFBRDADDR,&ifr_brd)<0)//直接获取广播地址
    {
        perror("Not setup interface\n");
        return -1;
    }

    memcpy(&sin_brd,&ifr_brd.ifr_broadaddr,sizeof(sin_brd));
    strcpy(brdaddr,inet_ntoa(sin_brd.sin_addr));
    close(sock_brdaddr);
    return 0;

}

//设置广播地址
int lingmo_network_network_setbrdaddr(char *if_name,char *brdaddr)
{
    int sock_brdaddr;
//    char netmask_addr[32];

    struct ifreq ifr_brd;
    struct sockaddr_in sin_brd;

    sock_brdaddr=socket(AF_INET,SOCK_STREAM,0);
    if(sock_brdaddr==-1)
    {
        perror("Create socket failure!\n");
        return -1;
    }

    memset(&sin_brd,0,sizeof(sin_brd));
    memset(&ifr_brd,0,sizeof(ifr_brd));
    strcpy(ifr_brd.ifr_name,if_name);


    sin_brd.sin_family=AF_INET;
    sin_brd.sin_addr.s_addr=inet_addr(brdaddr);
    memcpy(&ifr_brd.ifr_addr,&sin_brd,sizeof(sin_brd));

    if(ioctl(sock_brdaddr,SIOCSIFBRDADDR,&ifr_brd)<0)
    {
        perror("Not setup interface:SIOCSIFBRDADDR\n");
        return -1;
    }

    close(sock_brdaddr);
    return 0;

}

//获取子网掩码
int lingmo_network_network_getnetmask(char *if_name,char *netmask)
{
    int sock_mask;
    struct sockaddr_in sin_netmask;
    struct ifreq ifr_mask;
    if((sock_mask=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("sockket error");
        return -1;
    }

    strcpy(ifr_mask.ifr_name,if_name);

    if(ioctl(sock_mask,SIOCGIFNETMASK,&ifr_mask)<0)//直接获取子网掩码
    {
        perror("Not setup interface\n");
        return -1;
    }

    memcpy(&sin_netmask,&ifr_mask.ifr_netmask,sizeof(sin_netmask));
    strcpy(netmask,inet_ntoa(sin_netmask.sin_addr));
    close(sock_mask);
    return 0;

}

//设置子网掩码
int lingmo_network_network_setnetmask(char *if_name,char *netmask)
{
    int sock_netmask;
//    char netmask_addr[32];

    struct ifreq ifr_mask;
    struct sockaddr_in sin_netmask;

    sock_netmask=socket(AF_INET,SOCK_STREAM,0);
    if(sock_netmask==-1)
    {
        perror("Create socket failure!\n");
        return -1;
    }

    memset(&sin_netmask,0,sizeof(sin_netmask));
    memset(&ifr_mask,0,sizeof(ifr_mask));
    strcpy(ifr_mask.ifr_name,if_name);


    sin_netmask.sin_family=AF_INET;
    sin_netmask.sin_addr.s_addr=inet_addr(netmask);
    memcpy(&ifr_mask.ifr_addr,&sin_netmask,sizeof(sin_netmask));


//    printf("---%d---\n",ioctl(sock_netmask,SIOCSIFNETMASK,&ifr_mask));

    if(ioctl(sock_netmask,SIOCSIFNETMASK,&ifr_mask)<0)
    {
        perror("Not setup interface\n");
        return -1;
    }

    close(sock_netmask);
    return 0;

}

//增加路由表
int lingmo_network_network_addgateway(char *gateway)
{
    int sock_gateway;
    struct rtentry rmten;
    struct sockaddr_in sin_gateway;
    sock_gateway=socket(AF_INET,SOCK_DGRAM,0);

    if(sock_gateway==-1)
    {
        perror("Create socket failure!\n");
        return -1;
    }

    memset(&rmten,0,sizeof(rmten));

    sin_gateway.sin_family=AF_INET;
    sin_gateway.sin_addr.s_addr=inet_addr(gateway);
    sin_gateway.sin_port=0;

    ((struct sockaddr_in *)&rmten.rt_dst)->sin_family=AF_INET;

    ((struct sockaddr_in *)&rmten.rt_genmask)->sin_family=AF_INET;

    memcpy(&rmten.rt_gateway,&sin_gateway,sizeof(sin_gateway));
//    rmten.rt_flags=RTF_UP | RTF_GATEWAY;
    rmten.rt_flags=RTF_GATEWAY;

    if(ioctl(sock_gateway,SIOCADDRT,&rmten)<0)
    {
        perror("Not setup interface\n");
        return -1;
    }

    close(sock_gateway);
    return 0;
}

//删除路由表
int lingmo_network_network_delgateway(char *gateway)
{
    int sock_gateway;
    struct rtentry rmten;
    struct sockaddr_in sin_gateway;
    sock_gateway=socket(AF_INET,SOCK_DGRAM,0);

    if(sock_gateway==-1)
    {
        perror("Create socket failure!\n");
        return -1;
    }

    memset(&rmten,0,sizeof(rmten));

    sin_gateway.sin_family=AF_INET;
    sin_gateway.sin_addr.s_addr=inet_addr(gateway);
    sin_gateway.sin_port=0;

    ((struct sockaddr_in *)&rmten.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rmten.rt_genmask)->sin_family=AF_INET;

    memcpy(&rmten.rt_gateway,&sin_gateway,sizeof(sin_gateway));
//    rmten.rt_flags=RTF_UP | RTF_GATEWAY;
    rmten.rt_flags=RTF_GATEWAY;

    if(ioctl(sock_gateway,SIOCDELRT,&rmten)<0)
    {
        perror("Not setup interface\n");
        return -1;
    }

    close(sock_gateway);
    return 0;
}

//获取MAC
int lingmo_network_network_getmac(char *if_name,char *macaddr)
{
    int sock_mac;
    struct ifreq ifr_mac;
    struct sockaddr_in sin_mac;
    struct ether_addr ethaddr;//#include <net/ethernet.h>

    sock_mac=socket(AF_INET,SOCK_STREAM,0);

    strcpy(ifr_mac.ifr_name,if_name);

    if(-1==ioctl(sock_mac,SIOCGIFHWADDR,&ifr_mac))
    {
        perror("Not setup interface\n");
        return -1;
    }
    memcpy(&ethaddr,&ifr_mac.ifr_hwaddr.sa_data,sizeof(ethaddr));
    strcpy(macaddr,ether_ntoa(&ethaddr));//#include <netinet/ether.h>

//    strcpy(macaddr,ether_ntoa((struct ether_addr*)ifr_mac.ifr_hwaddr.sa_data));

    close(sock_mac);
    return 0;


}

//启用网络接口
int lingmo_network_network_setifup(char *if_name)
{
    struct ifreq ifr;
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return 1;
    }//创建用来检查网络的套接字


    strcpy(ifr.ifr_name,if_name);

    if(ioctl(sockfd,SIOCGIFFLAGS,&ifr)<0)
    {
        perror("ioctl SIOCGIFFLAGS error\n");
        return -1;
    }

//    printf("%d\n",ifr.ifr_flags);
    ifr.ifr_flags|=IFF_UP;
//    printf("%d\n",ifr.ifr_flags);
    if(ioctl(sockfd,SIOCSIFFLAGS,&ifr)<0)
    {
        perror("ioctl SIOCSIFFLAGS ERROR\n");
        return -1;
    }

    close(sockfd);
    return 0;

}

//关闭网络接口
int lingmo_network_network_setifdown(char *if_name)
{
    struct ifreq ifr;
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return 1;
    }//创建用来检查网络的套接字


    strcpy(ifr.ifr_name,if_name);

    if(ioctl(sockfd,SIOCGIFFLAGS,&ifr)<0)
    {
        perror("ioctl SIOCGIFFLAGS error\n");
        return -1;
    }

//    printf("%d\n",ifr.ifr_flags);
    ifr.ifr_flags &=~IFF_UP; //将IFF_UP取反后与原来的标志进行与运算
//    printf("%d\n",ifr.ifr_flags);
    if(ioctl(sockfd,SIOCSIFFLAGS,&ifr)<0)
    {
        perror("ioctl SIOCSIFFLAGS ERROR\n");
        return -1;
    }

    close(sockfd);
    return 0;

}

//获取MTU
int lingmo_network_network_getmtu(char *if_name)
{
    int sock_mtu;
//    struct sockaddr_in sin_ip;
    struct ifreq ifr_MTU;
    if((sock_mtu=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("sockket error");
        return -1;
    }

    strcpy(ifr_MTU.ifr_name,if_name);

    if(ioctl(sock_mtu,SIOCGIFMTU,&ifr_MTU))
    {
        perror("Not setup interface\n");
        return -1;
    }

    int mtu=ifr_MTU.ifr_mtu;
//    printf("%d\n",ifr_MTU.ifr_mtu);
//    printf("%d\n",mtu);

    close(sock_mtu);
    return mtu;

}

static void getBuf(char *bufstr)
{
    //O_RDONLY以只读方式打开文件
    int fd=open("/proc/net/dev", O_RDONLY);
    if(-1==fd)
    {
        printf("/proc/net/dev not exists!\n");
    }

    char buf[1024*2];
    //将读写位置移到文件开头
    lseek(fd,0,SEEK_SET);
    int nBytes=read(fd,buf,sizeof(buf)-1);
    if(-1==nBytes)
    {
        perror("read error");
        close(fd);
    }

    buf[nBytes]='\0';
    strcpy(bufstr,buf);
}

//总计上传下载数据量
long *lingmo_network_network_getbytes(char *if_name)
{
    char buf[1024*2];
    getBuf(buf);

    //返回第一次指向if_name位置的指针
    char *pDev=strstr(buf,if_name);
    if(NULL==pDev)
    {
        printf("don't find dev %s\n",if_name);
        return NULL;
    }

    char *p;
    char *value;
    int i=0;
    static long rtbyt[2];
    /*去除空格，制表符，换行符等不需要的字段*/
    for (p = strtok(pDev, " \t\r\n"); p; p = strtok(NULL, " \t\r\n"))
    {
        i++;
        value = (char*)malloc(20);
        strcpy(value, p);
        /*得到的字符串中的第二个字段是接收流量*/
        if(i == 2)
        {
            rtbyt[0] = atol(value);
        }
        /*得到的字符串中的第十个字段是发送流量*/
        if(i == 10)
        {
            rtbyt[1] = atol(value);
            break;
        }
        free(value);
    }
    return rtbyt;
}

//总计上传下载数据包
long *lingmo_network_network_getpackets(char *if_name)
{
    char buf[1024*2];
    getBuf(buf);

    //返回第一次指向if_name位置的指针
    char *pDev=strstr(buf,if_name);
    if(NULL==pDev)
    {
        printf("don't find dev %s\n",if_name);
        return NULL;
    }

    char *p;
    char *value;
    int i=0;
    static long rtpkt[2];
    /*去除空格，制表符，换行符等不需要的字段*/
    for (p = strtok(pDev, " \t\r\n"); p; p = strtok(NULL, " \t\r\n"))
    {
        i++;
        value = (char*)malloc(20);
        strcpy(value, p);
        /*得到的字符串中的第三个字段是接收流量*/
        if(i == 3)
        {
            rtpkt[0] = atol(value);
        }
        /*得到的字符串中的第十一个字段是发送流量*/
        if(i == 11)
        {
            rtpkt[1] = atol(value);
            break;
        }
        free(value);
    }
    return rtpkt;
}

//总计上传下载错误数据包数量
long *lingmo_network_network_geterrs(char *if_name)
{
    char buf[1024*2];
    getBuf(buf);

    //返回第一次指向if_name位置的指针
    char *pDev=strstr(buf,if_name);
    if(NULL==pDev)
    {
        printf("don't find dev %s\n",if_name);
        return NULL;
    }

    char *p;
    char *value;
    int i=0;
    static long rterrs[2];
    /*去除空格，制表符，换行符等不需要的字段*/
    for (p = strtok(pDev, " \t\r\n"); p; p = strtok(NULL, " \t\r\n"))
    {
        i++;
        value = (char*)malloc(20);
        strcpy(value, p);
        /*得到的字符串中的第四个字段是接收流量*/
        if(i == 4)
        {
            rterrs[0] = atol(value);
        }
        /*得到的字符串中的第十二个字段是发送流量*/
        if(i == 12)
        {
            rterrs[1] = atol(value);
            break;
        }
        free(value);
    }
    return rterrs;
}

//总计上传下载丢弃数据包数量
long *lingmo_network_network_getdrop(char *if_name)
{
    char buf[1024*2];
    getBuf(buf);

    //返回第一次指向if_name位置的指针
    char *pDev=strstr(buf,if_name);
    if(NULL==pDev)
    {
        printf("don't find dev %s\n",if_name);
        return NULL;
    }

    char *p;
    char *value;
    int i=0;
    static long rtdrop[2];
    /*去除空格，制表符，换行符等不需要的字段*/
    for (p = strtok(pDev, " \t\r\n"); p; p = strtok(NULL, " \t\r\n"))
    {
        i++;
        value = (char*)malloc(20);
        strcpy(value, p);
        /*得到的字符串中的第五个字段是接收流量*/
        if(i == 5)
        {
            rtdrop[0] = atol(value);
        }
        /*得到的字符串中的第十三个字段是发送流量*/
        if(i == 13)
        {
            rtdrop[1] = atol(value);
            break;
        }
        free(value);
    }
    return rtdrop;
}

//总计上传下载过载数据包数量
long *lingmo_network_network_getfifo(char *if_name)
{
    char buf[1024*2];
    getBuf(buf);

    //返回第一次指向if_name位置的指针
    char *pDev=strstr(buf,if_name);
    if(NULL==pDev)
    {
        printf("don't find dev %s\n",if_name);
        return NULL;
    }

    char *p;
    char *value;
    int i=0;
    static long rtfifo[2];
    /*去除空格，制表符，换行符等不需要的字段*/
    for (p = strtok(pDev, " \t\r\n"); p; p = strtok(NULL, " \t\r\n"))
    {
        i++;
        value = (char*)malloc(20);
        strcpy(value, p);
        /*得到的字符串中的第六个字段是接收流量*/
        if(i == 6)
        {
            rtfifo[0] = atol(value);
        }
        /*得到的字符串中的第十四个字段是发送流量*/
        if(i == 14)
        {
            rtfifo[1] = atol(value);
            break;
        }
        free(value);
    }
    return rtfifo;
}

