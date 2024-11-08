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

#ifndef SPEECH_H
#define SPEECH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SPEECH_SUCCESS=0,
    SPEECH_SESSION_ERROR, // session 错误
    SPEECH_NET_ERROR, // 网络错误
    SPEECH_SERVER_ERROR, // 服务错误或者不可用
    SPEECH_UNAUTHORIZED, // 权限限制，鉴权失败，无权访问
    SPEECH_REQUEST_TOO_MANY, // 请求频率过高
    SPEECH_REQUEST_FAILED, // 请求失败
    SPEECH_TEXT_LENGTH_INVAILD, // 输入文本长度超限
    SPEECH_ADIO_SIZE_INVAILD, // 音频大小超限
    SPEECH_PARAM_ERROR, // 参数错误
} SpeechResult;

typedef struct _SynthesisResult {
    char* audio_format;
    int audio_rate;
    int audio_channel;
    unsigned char* audio_data;
    unsigned int audio_data_length;
    int error_code;
} SynthesisResult;

typedef struct _RecognitionResult {
    char* text_data;
    unsigned int text_data_length;
    int speaker_id; // 0表示无效；大于0时表示具体的说话人的id
    int error_code;
} RecognitionResult;

typedef void* SpeechSession;

typedef void (*SpeechRecognitionResultCallback) (RecognitionResult result, void* user_data);
typedef void (*SpeechSynthesisResultCallback) (SynthesisResult result, void* user_data);
typedef void (*SpeechFinishedCallback) (void* user_data);

SpeechResult speech_create_session(SpeechSession* session);
void speech_destroy_session(SpeechSession session);
SpeechResult speech_init_session(SpeechSession session);

/// @brief 设置语音识别的结果回调函数
///  speech_continuous_recognition_write_audio_data() speech_recognize_once()
///  speech_continuous_translation_recognition_write_audio_data() speech_translation_recognitze_once()
///  的结果均通过该回调函数返回结果
/// @param session 传入 speech_create_session() 接口创建的 session
/// @param callback
/// @param user_data
void speech_set_recognizing_callback(SpeechSession session,
    SpeechRecognitionResultCallback callback, void* user_data);

/// @brief 设置语音识别结束的回调函数
/// @param session 传入 speech_create_session() 接口创建的 session
/// @param callback
/// @param user_data
void speech_set_recognized_callback(SpeechSession session,
                                    SpeechRecognitionResultCallback callback, void* user_data);

/// @brief 设置语音识别的结果回调函数
/// @param session 传入 speech_create_session() 接口创建的 session
/// @param callback
/// @param user_data
void speech_set_synthesizing_callback(SpeechSession session,
                                      SpeechSynthesisResultCallback callback, void* user_data);

/// @brief 设置语音识别结束时的回调函数
/// @param session 传入 speech_create_session() 接口创建的 session
/// @param callback
/// @param user_data
void speech_set_synthesized_callback(SpeechSession session,
                                     SpeechFinishedCallback callback, void* user_data);

/// @brief 开始连续语音听写/语音识别
///  将语音数据转成文本数据, 适用于实时语音转写的场景。
///  异步接口
/// @param session 传入 speech_create_session() 创建的 session
/// @param params json 格式的字符串参数
// {
//     "format": 音频格式
//     "rate": 采样率
//     "channel": 声道数
// }
/// @return
SpeechResult speech_start_continuous_recognition(SpeechSession session, const char* params);

/// @brief 传入连续语音识别时的音频数据
/// @param session 传入 speech_create_session() 创建的 session
/// @param audio_data 音频数据
/// @param audio_data_length 音频数据的长度
/// @return
SpeechResult speech_continuous_recognition_write_audio_data_async(SpeechSession session,
    const void* audio_data, unsigned int audio_data_length);

/// @brief 结束连续语音听写/语音识别
/// @param session 传入 speech_create_session() 创建的 session
/// @return
SpeechResult speech_stop_continuous_recognition(SpeechSession session);

/// @brief 听写/识别一段完整的语音。异步接口。\note 目前最多支持60s的音频数据
/// @param session 传入 speech_create_session() 创建的 session
/// @param params json 格式的字符串参数
// {
//     "format": 音频格式
//     "rate": 采样率
//     "channel": 声道数
// }
/// @param audio_data 音频数据
/// @param audio_data_length 音频数据长度
/// @return
SpeechResult speech_recognize_once_async(SpeechSession session,
    const char* params, const void* audio_data, unsigned int audio_data_length);


/// @brief 开始连续语音合成。异步接口
/// @param session 传入 speech_create_session() 创建的 session
/// @param params
// {
//     "voice_name": 发音人 - 暂时不要求设置，使用平台默认的
//     "speed": 语速
//     "volume": 音量
//     "pitch": 语调
// }
/// @return
SpeechResult speech_start_continuous_synthesis(SpeechSession session, const char* params);

/// @brief 写入连续语音合成时的文本数据
/// @param session 传入 speech_create_session() 创建的 session
/// @param text
/// @return
SpeechResult speech_continuous_synthesize_write_text_async(SpeechSession session, const char* text);

/// @brief 结束连续语音合成
/// @param session 传入 speech_create_session() 创建的 session
/// @return
SpeechResult speech_stop_continuous_synthesis(SpeechSession session);

/// @brief 一次合成一整段文字内容
/// @param session 传入 speech_create_session() 创建的 session
/// @param params 同 speech_start_continuous_synthesize() 接口的 \param params 参数
/// @param text 需要合成的文字
/// @return
SpeechResult speech_synthesize_once_async(SpeechSession session, const char* params, const char* text);


/// @brief 设置关键字识别的回调函数
/// @param session 传入 speech_create_session() 接口创建的 session
/// @param callback
/// @param user_data
void speech_set_keyword_recognized_callback(SpeechSession session,
                                            SpeechRecognitionResultCallback callback, void* user_data);

/// @brief 启动关键字识别
/// @return
SpeechResult speech_start_keyword_recognition(SpeechSession session);

/// @brief 停止关键字识别
/// @return
SpeechResult speech_stop_keyword_recognition(SpeechSession session);

/// @brief 获取最新的错误信息
/// @return 返回调用者线程产生的错误信息
const char* speech_get_last_error_message();

#ifdef __cplusplus
}
#endif

#endif
