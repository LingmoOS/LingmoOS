// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
* @file aria2const.h
* @brief 接口宏东定义,这个是规范
* @author denglinglong  <denglinglong@uniontech.com>
* @version 1.0.0
* @date 2020-05-26 11:55
* @copyright 2020-2020Uniontech Technology Co., Ltd.
*/

#ifndef ARIA2CONST_H
#define ARIA2CONST_H

#define ARIA2C_METHOD_SHUTDOWN "aria2.shutdown"
#define ARIA2C_METHOD_FORCE_SHUTDOWN "aria2.forceShutdown"

#define ARIA2C_METHOD_ADD_URI "aria2.addUri"
#define ARIA2C_METHOD_ADD_TORRENT "aria2.addTorrent"
#define ARIA2C_METHOD_ADD_METALINK "aria2.addMetalink"

#define ARIA2C_METHOD_PAUSE "aria2.pause"
#define ARIA2C_METHOD_FORCE_PAUSE "aria2.forcePause"
#define ARIA2C_METHOD_PAUSE_ALL "aria2.pauseAll"
#define ARIA2C_METHOD_FORCE_PAUSE_ALL "aria2.forcePauseAll"

#define ARIA2C_METHOD_UNPAUSE "aria2.unpause"
#define ARIA2C_METHOD_UNPAUSE_ALL "aria2.unpauseAll"

#define ARIA2C_METHOD_REMOVE "aria2.remove"
#define ARIA2C_METHOD_FORCE_REMOVE "aria2.forceRemove"

#define ARIA2C_METHOD_TELL_STATUS "aria2.tellStatus"

#define ARIA2C_PARAM_POS_SET "POS_SET"
#define ARIA2C_PARAM_POS_CUR "POS_CUR"
#define ARIA2C_PARAM_POS_END "POS_END"
#define ARIA2C_METHOD_CHANGE_POSITION "aria2.changePosition"

#define ARIA2C_METHOD_GET_SESSION_INFO "aria2.getSessionInfo"
#define ARIA2C_METHOD_SAVE_SESSION "aria2.saveSession"
#define ARIA2C_METHOD_GET_URIS "aria2.getUris"
#define ARIA2C_METHOD_GET_FILES "aria2.getFiles"
#define ARIA2C_METHOD_GET_PEERS "aria2.getPeers"
#define ARIA2C_METHOD_GET_SERVERS "aria2.getServers"
#define ARIA2C_METHOD_TELL_ACTIVE "aria2.tellActive"
#define ARIA2C_METHOD_TELL_WAITING "aria2.tellWaiting"
#define ARIA2C_METHOD_TELL_STOPPED "aria2.tellStopped"
#define ARIA2C_METHOD_CHANGE_URI "aria2.changeUri"
#define ARIA2C_METHOD_GET_OPTION "aria2.getOption"
#define ARIA2C_METHOD_CHANGE_OPTION "aria2.changeOption"
#define ARIA2C_METHOD_GET_GLOBAL_OPTION "aria2.getGlobalOption"
#define ARIA2C_METHOD_CHANGE_GLOBAL_OPTION "aria2.changeGlobalOption"
#define ARIA2C_METHOD_GET_GLOBAL_STAT "aria2.getGlobalStat"
#define ARIA2C_METHOD_PURGE_DOWNLOAD_RESULT "aria2.purgeDownloadResult"
#define ARIA2C_METHOD_REMOVE_DOWNLOAD_RESULT "aria2.removeDownloadResult"
#define ARIA2C_METHOD_GET_VERSION "aria2.getVersion"
#define ARIA2C_METHOD_MULTICALL "system.multicall"
#define ARIA2C_METHOD_LIST_METHODS "system.listMethods"
#define ARIA2C_METHOD_LIST_NOTIFICATIONS "system.listNotifications"

//active waiting paused error complete removed
#define ARIA2C_STATUS_ACTIVE "active"
#define ARIA2C_STATUS_WAITING "waiting"
#define ARIA2C_STATUS_PAUSED "paused"
#define ARIA2C_STATUS_ERROR "error"
#define ARIA2C_STATUS_COMPLETE "complete"
#define ARIA2C_STATUS_REMOVED "removed"

#endif // ARIA2CONST_H
