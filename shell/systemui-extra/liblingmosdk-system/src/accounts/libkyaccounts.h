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

#ifndef LINGMOSDK_SYSTEM_SETTINGS_H__
#define LINGMOSDK_SYSTEM_SETTINGS_H__

/**
 * @file        libkysystemsettings.h
 * @brief       设置用户信息
 * @author      shaozhimin (shaozhimin@kylinos.cn)
 * @version     0.1
 * @date        2023-10-07
 * @copyright   Copyright (c) 2023
 * 
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 创建用户，并创建家目录
 * @param name 用户名
 * @param fullName 用户全名
 * @param accountType 创建的类型，0标准用户，1管理员用户
 * 
 * @return char* 成功返回用户家目录路径，失败返回NULL；返回的字符串需要被 free 释放。
 */
char* kdk_system_create_user(char* name, char* fullName, int accountType);

/**
 * @brief 修改用户密码
 * @param username 用户名
 * @param password 新的用户密码
 * @param err_num 0,修改密码成功；1,身份验证失败；2,用户不存在；3,密码不规范；4,参数为空。
 * 
 * @return bool 是否修改成功；false代表修改失败，true代表修改成功。
 */
bool kdk_system_change_password(char* username, char* password, unsigned int *err_num);

/**
 * @brief 获取用户是否存在
 * @param username 用户名
 * 
 * @return bool 返回用户是否存在；false代表不存在，true代表存在。
 */
bool kdk_system_check_has_user(char *username);

/**
 * @brief 获取加密使用的公钥
 * 
 * @return char* 返回非对称加密使用的公钥，加密使用rsa算法
 */
char* kdk_login_get_public_encrypt();

/**
 * @brief 发送用户名和密码进行登录
 * 
 * @param username 用户名
 * @param password 加密后的密码
 * @param length 密码长度
 * 
 * @return bool true表示开始拉起桌面会话，false表示未能开始拉起桌面会话
 */
bool kdk_login_send_password(const char* username, unsigned char* password, int length);

#ifdef __cplusplus
}
#endif

#endif  // LINGMOSDK_SYSTEM_SETTINGS_H__

/**
  * @}
  */