/*
* liblingmosdk-diagnosetest's Library
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
* Authors: Zhen Sun <sunzhen1@kylinos.cn>
*
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cjson/cJSON.h>
#include <dbus/dbus.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/hmac.h>
#include "libkydatacollect.h"
#include <libkylog.h>

#define MACHINE_ID_FILE "/etc/machine-id"

#define DBUS_SERVICE "com.lingmo.daq"
#define DBUS_PATH "/com/lingmo/daq"
#define DBUS_INTERFACE "com.lingmo.daq.interface"
#define DBUS_METHOD "UploadEventMessage"

static long g_startup_timestamp = 0;

KTrackData* kdk_dia_data_init(KEventSourceType sourceType, KEventType evnetType);
void kdk_dia_data_free(KTrackData* node);

void kdk_dia_upload_program_startup();
void kdk_dia_upload_program_exit();
void kdk_dia_upload_search_content(KTrackData*node,char* eventCode, char* pageName, char*content);
void kdk_dia_upload_default(KTrackData*node,char* eventCode, char* pageName);

void kdk_dia_append_page_info(KTrackData* node, char* pageUrl, char* pagePath);
void kdk_dia_append_page_flag(KTrackData*node, bool is_entrancePage, bool is_exitPageFlag);
void kdk_dia_append_custom_property(KTrackData*node,KCustomProperty*property, int property_length);


bool isNewUser()
{
    return true;
}

char* base64Encode(const char*src ,int length)
{
    BUF_MEM *bptr;

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64 , BIO_FLAGS_BASE64_NO_NL);
    BIO *bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64 , bmem);
    BIO_write(b64 , src , length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64 , &bptr);

    char *buff = (char *)malloc(bptr->length + 1);
    memcpy(buff , bptr->data , bptr->length);
    buff[bptr->length] = 0;

    BIO_free_all(b64);

    return buff;
}

char* base64Decode(const char* src , int length)
{
    char *buf = (char *)malloc(length);
    memset(buf , 0 , length);

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64 , BIO_FLAGS_BASE64_NO_NL);
    BIO_set_close(b64 , BIO_CLOSE);

    BIO *bmem = BIO_new_mem_buf(src, length);

    bmem = BIO_push(b64 , bmem);

    BIO_read(bmem , buf , length);

    BIO_free_all(bmem);
    return buf;
}

bool data_node_is_initial(KTrackData* dataNode)
{
    if(!dataNode)
        return false;

    return true;
}

void call_dbus(const char*uploadedEventMessage, const char* messageSha256)
{
    if(!uploadedEventMessage || !messageSha256)
    {
        klog_err("dbus parameter is null\n");
        return;
    }
    DBusConnection *conn;
    DBusError error;

    dbus_error_init(&error);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM , &error);
    if (dbus_error_is_set(&error)) {
        klog_err("d-bus connect fail !\n");
        return ;
    }

    if (conn == NULL) {
        klog_err("d-bus connect fail !\n");
        return ;
    }

    DBusMessage *sendMsg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    sendMsg = dbus_message_new_method_call(DBUS_SERVICE , DBUS_PATH , DBUS_INTERFACE , DBUS_METHOD);
    if (!sendMsg){
        klog_err("call method:%s failed!\n",DBUS_METHOD);
        return ;
    }

    if (!dbus_message_append_args(sendMsg, DBUS_TYPE_STRING , &uploadedEventMessage , DBUS_TYPE_STRING , &messageSha256 , DBUS_TYPE_INVALID)) {
        klog_err("kdk : d-bus append args fail !\n");
        return;
    }

    if (!dbus_connection_send_with_reply(conn , sendMsg , &sendMsgPending , -1)) {
        klog_err("kdk : d-bus send message fail ! \n");
        return ;
    }

    if (sendMsgPending == NULL) {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return ;
    }

    dbus_connection_flush(conn);

    if (sendMsg) {
        dbus_message_unref(sendMsg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL) {
        klog_err("d-bus get reply message fail !");
        return;
    }

    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
}

char* json_serialize(KTrackData* data_node)
{
    cJSON* root =  cJSON_CreateObject();
    if(!data_node || !root)
        return NULL;
    if(data_node->eventCode)
        cJSON_AddStringToObject(root,"eventCode",data_node->eventCode);
    cJSON_AddNumberToObject(root,"eventType",data_node->eventType);
    cJSON_AddNumberToObject(root,"eventSourceType",data_node->eventSourceType);
    cJSON_AddStringToObject(root,"ts",data_node->timeStamp);
    if(data_node->pageName)
        cJSON_AddStringToObject(root,"pageName",data_node->pageName);
    if(data_node->pageUrl)
        cJSON_AddStringToObject(root,"pageUrl",data_node->pageUrl);
    if(data_node->pagePath)
        cJSON_AddStringToObject(root,"pagePath",data_node->pagePath);
    cJSON_AddNumberToObject(root,"duration",data_node->duration);
    if(data_node->appName)
        cJSON_AddStringToObject(root,"appName",data_node->appName);
    if(data_node->appVersion)
        cJSON_AddStringToObject(root,"appVersion",data_node->appVersion);
    if(data_node->searchWord)
        cJSON_AddStringToObject(root,"searchWord",data_node->searchWord);
    if(data_node->entrancePageFlag)
        cJSON_AddStringToObject(root,"entrancePageFlag",data_node->entrancePageFlag);
    if(data_node->exitPageFlag)
        cJSON_AddStringToObject(root,"exitPageFlag",data_node->exitPageFlag);


    cJSON*globalProperty = cJSON_CreateObject();
    if(!globalProperty)
        return NULL;
    if(data_node->globalProperty->umid)
        cJSON_AddStringToObject(globalProperty,"umid",data_node->globalProperty->umid);
    if(data_node->globalProperty->sourceIp)
        cJSON_AddStringToObject(globalProperty,"sourceIp",data_node->globalProperty->sourceIp);
    if(data_node->globalProperty->sourceMac)
        cJSON_AddStringToObject(globalProperty,"sourceMac",data_node->globalProperty->sourceMac);
    if(data_node->globalProperty->serviceKey)
        cJSON_AddStringToObject(globalProperty,"serviceSerialNo",data_node->globalProperty->serviceKey);
    cJSON_AddItemToObject(root,"globalProperties",globalProperty);
    if(data_node->customPropertyLength > 0 && data_node->customProperty)
    {
        cJSON*customProperty = cJSON_CreateObject();
        KCustomProperty* p = data_node->customProperty;
        for (int i = 0; i < data_node->customPropertyLength; i++)
        {
            char*tmpky = (p+i)->key;
            char*tmpva = (p+i)->value;
            cJSON_AddStringToObject(customProperty,tmpky,tmpva);
        }
        cJSON_AddItemToObject(root,"customProperties",customProperty);
    }
    char * json = cJSON_Print(root);
    cJSON_Delete(root);
    return json;

} 

long getTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long timeStamp = (long)tv.tv_sec * 1000LL + (long)tv.tv_usec / 1000LL;
    return timeStamp;
}

char* getAppName()
{
    extern char *__progname;
    return strdup(__progname);
}

char* getAppVersion()
{
    //先尝试根据进程名碰撞包名来获取包版本
    extern char *__progname;
    char cmd[256];
    memset(cmd,0,256);
    snprintf(cmd,256,"dpkg -l |grep -iw %s |awk '{print $3}'",__progname);
    FILE* f = popen(cmd,"r");
    char cmdResult[256];
    if(f && fgets(cmdResult, 256, f))
    {
        pclose(f);
        int len = strlen(cmdResult);
        if (len>0 && cmdResult[len-1] == '\n')
            cmdResult[len-1] = '\0';
        return strdup(cmdResult);
    }
    pclose(f);
    //通过二进制反查软件包版本
    int pid = getpid();
    char exe[128];
    memset(exe,0,128);
    snprintf(exe,128,"/proc/%d/exe",pid);
    char binPath[128];
    memset(binPath,0,128);
    readlink(exe,binPath,128);
    memset(cmd,0,256);
    snprintf(cmd,256,"dpkg -S %s 2>&1",binPath);
    f = popen(cmd,"r");
    char*pName = NULL;
    memset(cmdResult,0,256);
    if(f && fgets(cmdResult, 256, f))
    {
        pName =  strtok(cmdResult,":");
    }
    pclose(f);
    if(!pName)
        return NULL;
    memset(cmd,0,256);
    snprintf(cmd,256,"dpkg -l |grep -iw %s |awk '{print $3}'",pName);
    f = popen(cmd,"r");
    if(f && fgets(cmdResult, 256, f))
    {
        pclose(f);
        int len = strlen(cmdResult);
        if (len>0 && cmdResult[len-1] == '\n')
            cmdResult[len-1] = '\0';
        return strdup(cmdResult);
    }
    pclose(f);
    return strdup("0.0.0.0");
}

bool isPhysicalInterface(char*interfaceName)
{
    if(!interfaceName)
        return false;
    char cmdLine[128] = "ls -l /sys/class/net/ | grep -v virtual  | awk '{print $9}'";
    FILE* fp = popen(cmdLine,"r");
    char strLine[128];
    memset(strLine,0,128);
    while (fgets(strLine,128,fp))
    {
        if(!strncmp(strLine," ",1))
            continue;
        if(!strncmp(strLine,interfaceName,strlen(interfaceName)))
        {
            pclose(fp);
            return true;
        }
    }
    pclose(fp);
    return false;
}
void getSourceMacAndIp(char* ip, char*mac)
{
    struct ifreq buf[16];
    struct ifconf ifc;
    int ret = 0;
    int if_num = 0;
 
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
 
    int fd = 0;
 
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ret = ioctl(fd, SIOCGIFCONF, (char *)&ifc);
    if (ret) {
        klog_err("get if config info failed");
        close(fd);
        return;
    }

    if_num = ifc.ifc_len / sizeof(struct ifreq);
    while (if_num-- > 0) {

        //需要判断物理网卡还是虚拟网卡
        bool flag = isPhysicalInterface(buf[if_num].ifr_name);
        if(flag)
        {
            /* 获取第n个网口信息 */
            ret = ioctl(fd, SIOCGIFFLAGS, (char *)&buf[if_num]);
            
            if (ret)
                continue;
            /* 获取网口状态 */
            if(buf[if_num].ifr_flags & (IFF_UP))
            {
                /* 获取当前网卡的ip地址 */
                ret = ioctl(fd, SIOCGIFADDR, (char *)&buf[if_num]);
                if (ret)
                    continue;
                struct in_addr addr = ((struct sockaddr_in *)(&buf[if_num].ifr_addr))->sin_addr;
                char* ipStr = inet_ntoa(addr);
                snprintf(ip,64,"%s",ipStr);

                /* 获取当前网卡的mac */
                ret = ioctl(fd, SIOCGIFHWADDR, (char *)&buf[if_num]);
                if (ret)
                    continue;
    
                snprintf(mac,64,"%02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned char)buf[if_num].ifr_hwaddr.sa_data[0],
                    (unsigned char)buf[if_num].ifr_hwaddr.sa_data[1],
                    (unsigned char)buf[if_num].ifr_hwaddr.sa_data[2],
                    (unsigned char)buf[if_num].ifr_hwaddr.sa_data[3],
                    (unsigned char)buf[if_num].ifr_hwaddr.sa_data[4],
                    (unsigned char)buf[if_num].ifr_hwaddr.sa_data[5]);
            }
        }

    }
    close(fd);
}

char* getServiceKey()
{
    char* serviceKey = NULL;
    FILE *fp = fopen("/etc/.kyinfo", "r");
    if (!fp)
        return NULL;
    char key[3]="key";
    char buf[1024];
    memset(buf,0,1024);
    while (fgets(buf, 1024, fp))
    {
        if (strncmp(buf, key, strlen(key)) == 0)
        {
            serviceKey = strdup((char*)buf + strlen(key) +1);
            break;
        }
    }
    fclose(fp);
    if (!serviceKey)
        return NULL;
    int len = strlen(serviceKey);
    if (len>0 && serviceKey[len-1] == '\n')
        serviceKey[len-1] = '\0';
    return serviceKey;
}

char* getMachineId()
{
    char* machineId = NULL;
    FILE *fp = fopen(MACHINE_ID_FILE, "rt");
    if (fp <= 0)
        return machineId;
    char strLine[256];
    while(fgets(strLine,256,fp))
    {
        if (strlen(strLine) == 0)
            return machineId;
        char *startPos = strLine;
        while (*startPos != '\0' && *startPos == '\n')
            startPos++;
        if (*startPos == '\0')
        {
            strLine[0] = 0;
            return machineId;
        }

        char *endPos = strLine + strlen(strLine) - 1;
        while (endPos != strLine && *endPos == '\n')
            endPos --;

        memmove(strLine, startPos, endPos - startPos + 1);
        *(strLine + (endPos - startPos) + 1) = 0;

        machineId = strdup(strLine);
    }
    fclose(fp);
    return machineId;
}

KTrackData *kdk_dia_data_init(KEventSourceType sourceType, KEventType evnetType)
{
    KTrackData* node = malloc(sizeof(KTrackData));
    if(!node)
    {
        klog_err("node 内存分配失败\n");
        return NULL;
    }

    node->eventType = evnetType;
    node->eventSourceType = sourceType;
    node->globalProperty = (KGlobalProperty*)malloc(sizeof(KGlobalProperty));
    if(!node->globalProperty)
    {
        klog_err("node->globalProperty内存分配失败\n");
        return NULL;
    }
    node->globalProperty->umid = getMachineId();
    node->globalProperty->serviceKey = getServiceKey();
    node->globalProperty->sourceIp = (char*)malloc(sizeof(char)*64);
    memset(node->globalProperty->sourceIp,0,64);
    node->globalProperty->sourceMac = (char*)malloc(sizeof(char)*64);
    memset(node->globalProperty->sourceMac,0,64);
    getSourceMacAndIp(node->globalProperty->sourceIp,node->globalProperty->sourceMac);
    long ltStamp = getTimeStamp();
    snprintf(node->timeStamp,14,"%ld",ltStamp);
    node->appName = getAppName();
    node->appVersion = getAppVersion();
    node->customPropertyLength = 0;
    node->exitPageFlag = NULL;
    node->entrancePageFlag = NULL;
    node->pageUrl = NULL;
    node->pagePath = NULL;
    node->searchWord = NULL;
    node->customProperty = NULL;
    node->duration = 0;
    return node;
}

void kdk_dia_data_free(KTrackData *node)
{
    if(!node)
        return;
    if(node->eventCode)
        free(node->eventCode);
    if(node->pageName)
        free(node->pageName);
    if(node->pageUrl)
        free(node->pageUrl);
    if(node->pagePath)
        free(node->pagePath);
    if(node->appName)
        free(node->appName);
    if(node->appVersion)
        free(node->appVersion);
    if(node->searchWord)
        free(node->searchWord);
    if(node->entrancePageFlag)
        free(node->entrancePageFlag);
    if(node->exitPageFlag)
        free(node->exitPageFlag);
    if(node->globalProperty)
    {
        if(node->globalProperty->sourceIp)
            free(node->globalProperty->sourceIp);
        if(node->globalProperty->sourceMac)
            free(node->globalProperty->sourceMac);
        if(node->globalProperty->serviceKey)
            free(node->globalProperty->serviceKey);
        if(node->globalProperty->umid)
            free(node->globalProperty->umid);
        free(node->globalProperty);
    }
    free(node);
}

void kdk_dia_append_custom_property(KTrackData *node, KCustomProperty* property, int property_length)
{
    node->customProperty = property;
    node->customPropertyLength = property_length;
}

void kdk_dia_upload_default(KTrackData*node, char* eventCode, char* pageName)
{
    if(!data_node_is_initial(node))
        return;
    if(eventCode)
        node->eventCode = strdup(eventCode);
    if(pageName)
        node->pageName = strdup(pageName);
    char* eventMessage = json_serialize(node);
    char*eventMessageSha256 = base64Encode(eventMessage,strlen(eventMessage));
#ifdef DEBUG
    klog_info("%s\n",eventMessage);
#endif

    call_dbus(eventMessage,eventMessageSha256);

    free(eventMessage);
    free(eventMessageSha256);
}

void kdk_dia_upload_program_startup()
{
    KTrackData *dataNode = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_STARTUP);
    kdk_dia_upload_default(dataNode,"$$_app_startup","process_start_page");
    g_startup_timestamp = getTimeStamp();
    kdk_dia_data_free(dataNode);
}

void kdk_dia_upload_program_exit()
{
    KTrackData *dataNode = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_EXIT);
    long exitTimestamp = getTimeStamp();
    dataNode->duration = (exitTimestamp - g_startup_timestamp) /1000;
    kdk_dia_upload_default(dataNode,"$$_app_exit","process_exit_page");
    kdk_dia_data_free(dataNode);
}
void kdk_dia_upload_search_content(KTrackData*node,char* eventCode, char* pageName, char*content)
{
    if(!data_node_is_initial(node))
        return;
    node->eventType = KEVENT_SEARCH;
    if(content)
        node->searchWord = strdup(content);
    kdk_dia_upload_default(node,"$$_search",pageName);
}

void kdk_dia_append_page_info(KTrackData* node, char* pageUrl, char* pagePath)
{
    if(!data_node_is_initial(node))
        return;

    if(pageUrl)
        node->pageUrl = strdup(pageUrl);
    if(pagePath)
        node->pagePath = strdup(pagePath);
}

void kdk_dia_append_page_flag(KTrackData*node, bool is_entrancePage, bool is_exitPageFlag)
{
    if(!data_node_is_initial(node))
        return;
    
    if(is_entrancePage)
        node->entrancePageFlag = strdup("Y");
    else
        node->entrancePageFlag = strdup("N");
    if(is_exitPageFlag)
        node->exitPageFlag = strdup("Y");
    else
        node->exitPageFlag = strdup("N");
    
}