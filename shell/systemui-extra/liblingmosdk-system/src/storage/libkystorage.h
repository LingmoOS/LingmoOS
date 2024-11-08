#ifndef LINGMOSDK_SYSTEM_SOTRAGE_H_
#define LINGMOSDK_SYSTEM_SOTRAGE_H_

/**
 * @file libkystorage.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief 存储模块
 * @version 2.4
 * @date 2024-4-30
 *
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-storage
 * @{
 *
 */

typedef struct _FileInfo
{
    char name[256];
    int is_dir;
    struct _FileInfo *next;
} FileInfo;


/**
 * @brief 获取默认浏览器 Cookie 的路径，目前仅支持qax，chrome，chromium和firefox
 *
 * @return char* 浏览器 Cookie 的路径
 */
extern char *kdk_storage_get_default_cookie_path();

/**
 * @brief 获取当前用户桌面文件夹的路径
 *
 * @return char* 桌面文件夹的路径
 */
extern char *kdk_storage_get_default_desktop_path();

/**
 * @brief 获取当前用户文档文件夹的路径
 *
 * @return char* 文档文件夹的路径
 */
extern char *kdk_storage_get_default_documents_path();

/**
 * @brief 获取当前用户指定浏览器的Internet 缓存文件夹的路径，目前仅支持qax，chrome，chromium和firefox
 *
 * @param name 浏览器名称，入参为null，获取默认浏览器
 * @return char* Internet 缓存文件夹的路径
 */
extern char *kdk_storage_get_default_internetCache_path(char *name);

/**
 * @brief 获取当前用户下载文件夹的路径
 *
 * @return char* 下载文件夹的路径
 */
extern char *kdk_storage_get_default_download_path();

/**
 * @brief 获取文件内容的 MIME 类型
 *
 * @param filename 文件路径
 * @return char* 文件内容的 MIME 类型
 */
extern char *kdk_storage_get_file_ContentType(char *filename);

/**
 * @brief 获取文件的最近改动的日期和时间（是指最近修改INode时间）
 *
 * @param filename 文件路径
 * @return char* 创建当前文件的日期和时间
 */
extern char *kdk_storage_get_file_dateChange(char *filename);

/**
 * @brief 获取文件最近更改的日期和时间（是指最近修改文件内容的时间）
 *
 * @param filename 文件路径
 * @return char* 最近更改的日期和时间
 */
extern char *kdk_storage_get_file_dateModify(char *filename);

/**
 * @brief 获取文件最近访问的日期和时间
 *
 * @param filename 文件路径
 * @return char* 最近访问的日期和时间
 */
extern char *kdk_storage_get_file_dateAccess(char *filename);

/**
 * @brief 获取文件大小
 *
 * @param filename 文件路径
 * @return long 文件大小
 */
extern long kdk_storage_get_file_size(char *filename);

/**
 * @brief 读取文件内容
 *
 * @param filename 要读取的文件的路径和名称
 * @param buffer 缓冲区
 * @return int 如果成功读取文件，则返回读取的字节数；如果发生错误，则返回 -1
 */
extern int kdk_storage_read_file(const char *filename, char *buffer, unsigned long size);

/**
 * @brief 将数据追加到指定的文件中
 *
 * @param filename 要追加的文件的路径和名称
 * @param data 要追加到文件中的数据
 * @param len 数据长度
 * @return int 如果成功写入文件，则返回 0；如果发生错误，则返回 -1
 */
extern int kdk_storage_write_file(const char *filename, const char *data, unsigned long length);

/**
 * @brief 移动文件（仅支持桌面、下载、文档、音乐或视频目录）
 *
 * @param source 要移动的源文件的路径和名称
 * @param destination 目标文件的路径和名称
 * @return int 如果成功移动文件，则返回 0；如果发生错误，则返回 -1
 */
extern int kdk_storage_move_file(const char *source, const char *destination);

/**
 * @brief 获取文件权限
 *
 * @param filename 要获取权限的文件的路径和名称
 * @return char* 文件权限
 */
extern char *kdk_storage_get_file_permissions(const char *filename);

/**
 * @brief 获取文件所有者
 *
 * @param filename 要获取所有者的文件的路径和名称
 * @return char* 文件所有者
 */
extern char *kdk_storage_get_file_owner(const char *filename);

/**
 * @brief 获取文件所属组
 *
 * @param filename 要获取所属组的文件的路径和名称
 * @return char* 文件所属组
 */
extern char *kdk_storage_get_file_group(const char *filename);

/**
 * @brief 列出指定路径下的所有文件和文件夹
 *
 * @param path 要列出的路径
 * @return  FileInfo 结构体
 */
extern FileInfo *kdk_storage_list_files(const char *path);

/**
 * @brief 释放FileInfo链表
 *
 * @param info FileInfo链表
 */
extern void kdk_storage_free_file_info(FileInfo *info);

/**
 * @brief 解析指定文件的符号链接（软链接），获取实际文件路径
 *
 * @param filename 指定的文件名
 * @return char* 实际文件路径
 */
extern char *kdk_storage_resolve_symbolic_link(const char *filename);

#endif // LINGMOSDK_SYSTEM_SOTRAGE_H_