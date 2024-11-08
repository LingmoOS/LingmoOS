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

#ifndef NLPSERVICE_H
#define NLPSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NLP_SUCCESS=0,
    NLP_SESSION_ERROR, // session 错误
    NLP_NET_ERROR, // 网络错误
    NLP_SERVER_ERROR, // 服务错误或者不可用
    NLP_UNAUTHORIZED, // 权限限制，鉴权失败，无权访问
    NLP_REQUEST_TOO_MANY, // 请求频率过高
    NLP_REQUEST_FAILED, // 请求失败
    NLP_INPUT_TEXT_LENGTH_INVAILD, // 输入文本长度超限
    NLP_PARAM_ERROR, // 参数错误
} NlpResult;

typedef struct _ChatResult {
    unsigned char *text;
    int error_code;
} NlpTextData;

typedef void* NlpSession;

typedef void (*NlpResultCallback) (NlpTextData text_data, void* user_data);

NlpResult nlp_create_session(NlpSession* session);
void nlp_destroy_session(NlpSession session);
NlpResult nlp_init_session(NlpSession session);

/// @brief 设置对话时的结果回调函数。 \note 结果是流式返回的，每次对话可能会产生多次回调
/// @param session 传入 nlp_create_session() 创建的 session
/// @param callback 传入 NlpCallback 类型的结果回到函数
/// @param user_data 调用者自定义的数据，作为 callback 的参数返回
void nlp_set_result_callback(NlpSession session, NlpResultCallback callback, void* user_data);

/// @brief 设置 session 的角色 \note 设置角色之后，该session暂时不支持 function call 功能
/// @param session 传入 nlp_create_session() 创建的 session
/// @param actor_prompt_id 角色的提示词模板 id
///  1: 中英文翻译; 2: 英中文翻译； 3-10：保留，后续可能会扩展多种语言的翻译
/// 11： 文本扩写； 12：文本润色；
/// 13：内容质检； 14: 邮件回复;
/// 15：总结概括；16：工作总结
/// 17: 代码生成; 18: 会议信息提取
void nlp_set_session_actor(NlpSession session, int actor_prompt_id);

/// @brief 设置对话过程中的上下文大小
/// @param session 传入 nlp_create_session() 创建的 session
/// @param size -1为默认; 0为无; 1为1K,逐渐递增
void nlp_set_context_size(NlpSession session, int size);

/// @brief 清除对话过程中的上下文
/// @param session 传入 nlp_create_session() 创建的 session
void nlp_clear_context(NlpSession session);

/// @brief 进行文本对话。同步接口
/// @param session 传入 nlp_create_session() 创建的 session
/// @param message 对话内容。结果通过 callback 返回给调用者
/// @return
NlpResult nlp_text_chat(NlpSession session, const char* message);

/// @brief 进行文本对话。异步接口
/// @param session 传入 nlp_create_session() 创建的 session
/// @param message 对话内容。结果通过 callback 返回给调用者
/// @return
NlpResult nlp_text_chat_async(NlpSession session, const char* message);

/// @brief 停止对话，对话过程中如果正在生成文字，调用该接口可以停止生成文字
/// @param session 传入 nlp_create_session() 创建的 session
/// @return
NlpResult nlp_stop_chat(NlpSession session);

/// @brief 获取最近产生的错误信息
/// @return 返回调用者线程产生的错误信息
const char* nlp_get_last_error_message();

#ifdef __cplusplus
}
#endif

#endif
