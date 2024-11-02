/*
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
 * Authors: jishengjie <jishengjie@kylinos.cn>
 *
 */

#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <dbus-1.0/dbus/dbus.h>
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/sha.h"
#include "openssl/bio.h"
#include "openssl/buffer.h"
#include "openssl/hmac.h"
#include "nlohmann/json.hpp"
#include "buriedpoint.h"

namespace kdk
{
namespace
{
constexpr char pkgInfoKeyPackageName[] = "packageName";
constexpr char pkgInfoKeyMessageType[] = "messageType";
constexpr char pkgInfoKeyTid[] = "tid";

constexpr char dbusServerName[] = "com.lingmo.daq";
constexpr char dbusObjectName[] = "/com/lingmo/daq";
constexpr char dbusInterfaceName[] = "com.lingmo.daq.interface";
constexpr char dbusMehtodName[] = "UploadMessage";

const std::string configPath = getenv("HOME") + std::string("/") + ".config/buriedpoint/uploadmessage.conf";
}

BuriedPoint::BuriedPoint() = default;

BuriedPoint::~BuriedPoint() = default;

bool BuriedPoint::uploadMessage(std::string packageName , std::string messageType , std::map<std::string , std::string> data)
{
    if (!checkDir()) {
        std::cout << "kdk : Failed to create configuration directory !";
        return false;
    }

    /* 从配置文件中获取 tid */
    std::string tid = readTid();

    /* 生成 package info */
    nlohmann::json pkgInfoObj;
    pkgInfoObj[pkgInfoKeyPackageName] = packageName;
    pkgInfoObj[pkgInfoKeyMessageType] = messageType;
    pkgInfoObj[pkgInfoKeyTid] = tid;
    std::string pkgInfo = pkgInfoObj.dump();

    /* 获取上传数据 */
    std::string uploadData = getUploadData(data);

    /* 调用 d-bus */
    if (!callDbus(pkgInfo , uploadData , "")) {
        std::cout << "kdk : buried point d-bus call fail !" << std::endl;
        return false;
    }

    return true;
}

/* 配置文件存放路径 ~/.config/buriedpoint/ */
bool BuriedPoint::checkDir(void)
{
    std::string homePath = std::string(getenv("HOME"));

    std::string subPath = homePath + "/.config";
    if (access(subPath.c_str() , F_OK)) {
        if (mkdir(subPath.c_str() , 0775)) {
            return false;
        }
    }

    std::string destPath = subPath + "/buriedpoint";
    if (access(destPath.c_str() , F_OK)) {
        if (mkdir(destPath.c_str() , 0775)) {
            return false;
        }
    }

    return true;
}

std::string BuriedPoint::getUploadData(std::map<std::string , std::string> &data)
{
    nlohmann::json jsonData;
    std::map<std::string , std::string>::iterator it = data.begin();
    while(it != data.end()) {
        jsonData[it->first] = it->second;
        it++;
    }

    jsonData["createTimeStamp"] = getCurrentTime();

    return jsonData.dump();
}

std::string BuriedPoint::getCurrentTime(void)
{
    struct timeval tp;
    gettimeofday(&tp , NULL);
    int msec = tp.tv_usec / 1000;

    struct tm t;
    localtime_r(&tp.tv_sec , &t);

    char buf[128] = {0};
    strftime(buf , sizeof(buf) , "%Y-%m-%d %H:%M:%S" , &t);

    char currTime[512] = {0};
    snprintf(currTime , sizeof(currTime) , "%s.%03d" , buf , msec);

    return std::string(currTime);
}

bool BuriedPoint::callDbus(const std::string &pkgInfo , const std::string &uploadData , const std::string &uploadDataSha256)
{
    DBusConnection *conn;
    DBusError error;

    dbus_error_init(&error);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM , &error);
    if (dbus_error_is_set(&error)) {
        std::cout << "d-bus connect fail !" << std::endl;
        return false;
    }

    if (conn == NULL) {
        return -1;
    }

    DBusMessage *sendMsg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    sendMsg = dbus_message_new_method_call(dbusServerName , dbusObjectName , dbusInterfaceName , dbusMehtodName);
    const char *tmpPkgInfo = pkgInfo.c_str();
    const char *tmpUploadData = uploadData.c_str();
    const char *tmpUploadDataSha256 = uploadDataSha256.c_str();
    if (!dbus_message_append_args(sendMsg , DBUS_TYPE_STRING , &tmpPkgInfo , DBUS_TYPE_STRING , &tmpUploadData , DBUS_TYPE_STRING , &tmpUploadDataSha256 , DBUS_TYPE_INVALID)) {
        std::cout << "kdk : d-bus append args fail !" << std::endl;
        return false;
    }

    if (!dbus_connection_send_with_reply(conn , sendMsg , &sendMsgPending , -1)) {
        std::cout << "kdk : d-bus send message fail !" << std::endl;
        return false;
    }

    if (sendMsgPending == NULL) {
        std::cout << "kdk : d-bus pending message is NULL !" << std::endl;
        return false;
    }

    dbus_connection_flush(conn);

    if (sendMsg) {
        dbus_message_unref(sendMsg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL) {
        std::cout << "d-bus get reply message fail !" << std::endl;
        return false;
    }

    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;
    int retState = -1;
    char *nowTid = NULL;

    if (!dbus_message_iter_init(replyMsg , &args)) {
        dbus_message_unref(replyMsg);
        std::cout << "kdk : d-bus init reply message fail !";
        return false;
    } else {
        dbus_message_iter_get_basic(&args , &retState);
    }

    if (dbus_message_iter_has_next(&args)) {
        if (!dbus_message_iter_next(&args)) {
            dbus_message_unref(replyMsg);
            std::cout << "kdk : d-bus next reply message fail !";
            return false;
        } else {
            dbus_message_iter_get_basic(&args , &nowTid);
        }
    }

    /* 处理 dbus 返回值 */
    bool retvalue = false;
    switch (retState) {
    case returnState::OK:
        retvalue = true;
        break;
    case returnState::InvalidTid:
        if (nowTid != NULL) {
            if (!writeTid(nowTid)) {
                std::cout << "kdk : tid write fail !" << std::endl;
            }
        }
        retvalue = true;
        break;
    default:
        std::cout << "kdk : dbus return error ! return state " << retState << std::endl;
        break;
    }

    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }

    return retvalue;
}

/* 配置文件存放路径 ~/.config/buriedpoint/uploadmessage.conf */
std::string BuriedPoint::readTid(void)
{
    std::string ret("");

    std::ifstream ifs;
    ifs.open(configPath , std::ios::in);
    if (ifs.is_open()) {
        std::getline(ifs , ret);
    } else {
        return "";
    }

    ifs.close();

    size_t found = ret.find('=');
    if (found == std::string::npos) {
        return "";
    }

    return ret.substr(found + 1);
}

bool BuriedPoint::writeTid(std::string tid)
{
    const std::string dest = "tid=" + tid;

    std::ofstream ofs;
    ofs.open(configPath , std::ios::out | std::ios::trunc);
    if (ofs.is_open()) {
        ofs << dest << std::endl;
    } else {
        std::cout << "kdk : open uploadmessage file fail !" << std::endl;
        return false;
    }

    ofs.close();

    return true;
}

}
