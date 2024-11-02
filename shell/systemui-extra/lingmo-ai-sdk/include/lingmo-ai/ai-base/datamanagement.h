/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATAMANAGEMENT_H
#define DATAMANAGEMENT_H

#include <lingmo-ai/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DATA_MANAGEMENT_SUCCESS = 0,
    DATA_MANAGEMENT_UNKNOWN_ERROR, // 未知错误
    DATA_MANAGEMENT_SESSION_ERROR, // session 错误
    DATA_MANAGEMENT_INVAILD_PATH, // 无效的路径
    DATA_MANAGEMENT_CANT_OPEN_DB, // 无法打开数据库
    DATA_MANAGEMENT_INPUT_TEXT_LENGTH_INVAILD, // 输入文本长度超限
    DATA_MANAGEMENT_PARAM_ERROR, // 参数错误
    DATA_MANAGEMENT_FILE_FORMAT_ERROR, // 文件格式错误
} DataManagementResult;

typedef enum {
    SYS_SEARCH = 0,
} DataManagementType;

typedef void* DataManagementSession;

/*!
 * \brief 创建数据管理会话
 * \param session 一个未经初始化的指针，用来返回 session，用户需要在使用完 session 之后
 * 调用 data_management_destroy_session 进行销毁
 * \param type 当前会话的业务类型，目前仅支持搜索，未来可能有数据分类等类型
 * \param uid 当前系统登录用户的 UID，用来标识用户
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_create_session(DataManagementSession* session, DataManagementType type, int uid);

/*!
 * \brief 销毁数据管理会话
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_destroy_session(DataManagementSession session);

/*!
 * \brief 获取当前会话支持的功能状态
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param status 获取到的功能状态，入参不能为 nullptr，函数内会修改 status 的值
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
 * \example 示例用法
 * FeatureStatus status = FeatureStatus::UNKNOWN;
 * DataManagementResult result = data_management_get_feature_status(session, &status);
 * if (result == DataManagementResult::DATA_MANAGEMENT_SUCCESS &&
 *     status == FeatureStatus::AVAILABLE) {
 *     do_something();
 * }
*/
DataManagementResult data_management_get_feature_status(DataManagementSession session, FeatureStatus* status);

/*!
 * \brief 检查文件是否支持，返回识别到的文件格式，需要在调用 data_management_add_files 前使用
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param filepath 检查的文件绝对路径
 * \param result 检查的格式结果，为 json 格式，格式如下
 * {
 *   "supported": true
 *   "format" : "txt"
 * }
 * supported 字段代表当前文件是否支持，对于不支持的文件在 data_management_add_files 时会返回
 * DATA_MANAGEMENT_FILE_FORMAT_ERROR
 * format 字段代表识别到的格式，可以用作 data_management_add_files 时入参的 fileformat
 * result 需要使用 data_management_free_check_file_format_result 释放
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_check_file_format(DataManagementSession session, const char* filepath, char** check_result);

/*!
 * \brief 释放文件检查的结果
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param check_result 要被释放的结果，通过 data_management_check_file_format 获得
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_free_check_file_format_result(DataManagementSession session, char* check_result);

/*!
 * \brief 会通过语意相似度搜索文件内容，返回包含相关内容的文件
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param search_conditions 要搜索的内容，是一个 json 格式的字符串，格式如下：
 * {
 *   "text": "linux内核安全选项"
 *   "similarity-threshold" : 0.6
 * }
 * similarity-threshold 代表相似度阈值，只会返回超过这个阈值的结果，如果不写则使用
 * 默认的相似度阈值
 * \param search_result 返回搜索结果，一般传入一个空指针，由SDK申请内存，在使用完后
 * 通过 data_management_free_search_result 进行销毁。
 * 搜索结果为一段 json 格式的文件列表，包含"filepath"和"similarity" 两个字段，格式如下：
 * {
 *   [
 *     {
 *       "filepath": "/path/to/file1.txt",
 *       "similarity": 0.92
 *     },
 *     {
 *       "filepath": "/path/to/file2.txt",
 *       "similarity": 0.85
 *     }
 *   ]
 * }
 * 结果默认会根据相似度进行排序，越接近1认为相似度越高
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_similarity_search(DataManagementSession session, const char* search_conditions, char** search_result);

/*!
 * \brief 释放搜索结果的内存
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param search_result data_management_similarity_search 返回的结果
*/
DataManagementResult data_management_free_search_result(DataManagementSession session, char* search_result);

/*!
 * \brief 将新文件增加到会话的数据范围中
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要增加到搜索范围里的文件列表，使用 json 格式，格式参考如下：
 * {
 *   [
 *     {
 *       "filepath": "/path/to/file3.pdf",
 *       "fileformat": "pdf"
 *     },
 *     {
 *       "filepath": "/path/to/file1.txt",
 *       "fileformat": "txt"
 *     }
 *   ]
 * }
 * filepath 字段要求是文件的绝对路径
 * fileformat 字段目前支持 pdf, txt, docx, pptx 格式，可以通过 data_management_check_file_format 获取
 * 如果 fileformat 为空会默认使用文件的后缀
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_add_text_files(DataManagementSession session, const char* fileinfos);

/*!
 * \brief 将新文件增加到会话的数据中
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要增加到搜索范围里的文件列表，使用 json 格式，格式参考如下：
 * {
 *   [
 *     {
 *       "filepath": "/path/to/file3.jpeg",
 *       "fileformat": "jpeg"
 *     },
 *     {
 *       "filepath": "/path/to/file2.png",
 *       "fileformat": "png"
 *     }
 *   ]
 * }
 * fileformat 目前支持 png, jpg, jepg 格式
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_add_image_files(DataManagementSession session, const char* fileinfos);

/*!
 * \brief 将文件从会话的数据中删除
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要从会话的文件里删除的文件列表，字符串为 json 格式，其中路径为绝对路径
 * {
 *   [
 *     {
 *       "filepath": "/path/to/file2.png"
 *     },
 *     {
 *       "filepath": "/path/to/file1.pdf"
 *     }
 *   ]
 * }
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_delete_files(DataManagementSession session, const char* fileinfos);

/*!
 * \brief 更新文件名，不重建向量
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要从会话的文件里重命名的文件列表，字符串为 json 格式，其中路径为绝对路径
 * {
 *   [
 *     {
 *       "old_filepath": "/path/to/file2.png",
 *       "new_filepath": "/path/to/newfile2.png"
 *     },
 *     {
 *       "old_filepath": "/path/to/file1.txt",
 *       "new_filepath": "/path/to/newfile1.txt"
 *     }
 *   ]
 * }
 * \param new_fileinfos 新文件文件列表，格式和旧文件列表一致，和旧文件列表需要是一一对应关系
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_update_files_name(DataManagementSession session, const char* fileinfos);

/*!
 * \brief 更新文件的向量数据
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要从会话的文件要更新的文件列表，字符串为 json 格式，其中路径为绝对路径
 * {
 *   [
 *     {
 *       "filepath": "/path/to/file2.png"
 *     },
 *     {
 *       "filepath": "/path/to/file1.pdf"
 *     }
 *   ]
 * }
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_update_files_content(DataManagementSession session, const char* fileinfos);

/*!
 * \brief 获取当前会话所有文件及修改时间列表
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos_result 文件列表，返回查询结果，一般传入一个 nullptr，由SDK申请内存，
 * 需要通过 data_management_free_get_all_fileinfos_result 销毁
 * 返回的结果为 json 格式，包含文件的绝对路径和文件修改时间的UNIX时间戳
 * {
 *   [
 *     {
 *       "filepath": "/path/to/file3.jpeg",
 *       "timestamp": 123456
 *     },
 *     {
 *       "filepath": "/path/to/file2.png",
 *       "timestamp": 654321
 *     }
 *   ]
 * }
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
*/
DataManagementResult data_management_get_all_fileinfos(DataManagementSession session, char** fileinfos_result);

/*!
 * \brief 释放 data_management_get_all_fileinfos 的返回结果
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos_result 要销毁的 data_management_get_all_fileinfos 返回结果
 *
*/
DataManagementResult data_management_free_get_all_fileinfos_result(DataManagementSession session, char* fileinfos_result);


#ifdef __cplusplus
}
#endif

#endif // DATAMANAGEMENT_H
