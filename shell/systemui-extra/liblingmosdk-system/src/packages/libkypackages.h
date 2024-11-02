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

#ifndef KDK_SYSTEM_PACKAGES_H__
#define KDK_SYSTEM_PACKAGES_H__

/**
 * @file libkypackages.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 包列表信息
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-package
 * @{
 * 
 * 
 */
// typedef enum kdk_packageStatus {
//     UNINIT,
//     INSTALLED,
//     UNCONFIG,
//     NONE
// }kdk_packageStatus;
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _kdk_package_t{
    char *name;     // 包名
    char *version;  // 版本号
    char *section;  // 包类型
    char *status;   // 状态
    unsigned long size_kb;   // 包大小
    struct _kdk_package_t **depends; // 依赖包
}kdk_package_t;

typedef struct {
    unsigned int nums;
    kdk_package_t **list;
}kdk_package_list;

typedef struct _kdk_startmenu_t
{
    char *name;    // 程序名称
    char *version; // 版本号
    char *company; // 所属公司
    char *cmd;     // 可执行文件路径
    char *param;   // 启动参数
    char *icon;    // 图标信息
} kdk_startmenu_t;

typedef struct _kdk_startmenu_list
{
    unsigned int nums;      //list成员个数
    kdk_startmenu_t **list; //开始菜单应用信息数组
} kdk_startmenu_list;

/**
 * @brief 获取系统中所有包列表
 * 
 * @return kdk_package_list* 包描述结构体列表；返回的结构体需要由kdk_package_free_packagelist()回收
 */
extern kdk_package_list* kdk_package_get_packagelist();

/**
 * @brief 回收由kdk_package_get_packagelist()返回的结构体
 * 
 * @param list 由kdk_package_get_packagelist()返回的结构体
 */
extern void kdk_package_free_packagelist(kdk_package_list *list);

/**
 * @brief [未启用]安装指定的软件包
 * 
 * @param packagePath 软件包路径
 * @return int 成功返回0，失败返回错误码
 */
extern int kdk_package_install_package(const char *packagePath);

/**
 * @brief [未启用]删除指定的包
 * 
 * @param name 包名
 * @param version 指定版本号，大部分情况下为NULL，为NULL时删除所有版本，该参数只是为以后snap兼容做准备
 * @return int 成功返回0，失败返回错误码
 */
extern int kdk_package_remove_package(const char *name, const char *version);

/**
 * @brief 获取系统中指定包的版本号
 * 
 * @param name 软件包名
 * @return char* 版本号，由alloc生成，需要被free；若给定的包不存在，则返回NULL
 */
extern char* kdk_package_get_version(const char *name);

/**
 * @brief 检测指定包名的软件包是否正确安装
 * 
 * @param name 包名
 * @param version 版本号，大部分情况下为NULL，预留为snap做准备
 * @return int 成功返回0，失败返回错误码
 */
extern int kdk_package_is_installed(const char *name, const char *version);

/**
 * @brief 获取指定应用的描述
 * 
 * @param name 应用名
 * @return char* 应用的描述信息
 */
extern char *kdk_package_get_description(const char *name);

/**
 * @brief 获取指定应用的安装目录路径
 * 
 * @param name 应用名
 * @return char** 应用的安装目录路径
 */
extern char **kdk_package_get_code_path(const char *name);

/**
 * @brief 获取指定应用的安装文件总数
 * 
 * @param name 应用名
 * @return int 应用的安装文件总数
 */
extern int kdk_package_get_file_count(const char *name);

/**
 * @brief [未启用] 判断指定应用是否可以被移除
 * 
 * @param name 应用名
 * @return int 布尔值 1 true; 0 false
 */
extern int kdk_package_is_removable(const char *name);

/**
 * @brief 判断安装deb包是否有足够的磁盘空间
 * 
 * @param path deb包路径
 * @return int 布尔值 1 true； 0 false
 */
extern int kdk_package_verify_disk_space(const char *path);

/**
 * @brief 获取默认浏览器名称
 * 
 * @return char* 默认浏览器名称
 */
extern char* kdk_package_get_default_browser(void);

/**
 * @brief 获取默认图片查看器名称
 * 
 * @return char* 默认图片查看器名称
 */
extern char* kdk_package_get_default_image_viewer(void);

/**
 * @brief 获取默认音频播放器名称
 * 
 * @return char* 默认音频播放器名称
 */
extern char* kdk_package_get_default_audio_player(void);

/**
 * @brief 获取默认视频播放器名称
 * 
 * @return char* 默认视频播放器名称
 */
extern char* kdk_package_get_default_video_player(void);

/**
 * @brief 获取默认PDF文档查看器名称
 * 
 * @return char* 默认PDF文档查看器名称
 */
extern char* kdk_package_get_default_pdf_viewer(void);

/**
 * @brief 获取默认WORD文档查看器名称
 * 
 * @return char* 默认WORD文档查看器名称
 */
extern char* kdk_package_get_default_word_viewer(void);

/**
 * @brief 获取默认EXCEL文档查看器名称
 * 
 * @return char* 默认EXCEL文档查看器名称
 */
extern char* kdk_package_get_default_excel_viewer(void);

/**
 * @brief 获取默认PPT文档查看器名称
 * 
 * @return char* 默认PPT文档查看器名称
 */
extern char* kdk_package_get_default_ppt_viewer(void);

/**
 * @brief 获取开始菜单中的所有应用信息
 * 
 * @return kdk_startmenu_list* 开始菜单中的所有应用信息
 */
extern kdk_startmenu_list* kdk_package_get_startmenu_list(void);

/**
 * @brief 释放kdk_package_get_startmenu_list的返回
 * 
 * @param list kdk_package_get_startmenu_list返回的指针
 */
extern void kdk_package_free_startmenu_list(kdk_startmenu_list *list);



#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/packages/test/kypackage-test.c
 * 
 */

/**
  * @}
  */
