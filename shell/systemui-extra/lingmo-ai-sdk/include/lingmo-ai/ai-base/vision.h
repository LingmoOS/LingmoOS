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

#ifndef VISION_H
#define VISION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VISION_SUCCESS=0,
    VISION_SESSION_ERROR,
    VISION_NET_ERROR, // 网络错误
    VISION_SERVER_ERROR, // 服务错误或者不可用
    VISION_UNAUTHORIZED, // 权限限制，鉴权失败，无权访问
    VISION_REQUEST_TOO_MANY, // 请求频率过高
    VISION_REQUEST_FAILED, // 请求失败
    VISION_TASK_TIMEOUT, // 任务超时
    VISION_INPUT_TEXT_LENGTH_INVAILD, // 输入文本长度超限
    VISION_GENERATE_IMAGE_FAILED, // 生成图片失败
    VISION_GENERATE_IMAGE_BLOCKED, // 图片审核失败
    VISION_PARAM_ERROR, // 参数错误
} VisionResult;

// 图片格式
typedef enum {
    VISION_UNKNOWN = -1,
    VISION_BMP		= 0,
    VISION_ICO		= 1,
    VISION_JPEG	= 2,
    VISION_JNG		= 3,
    VISION_KOALA	= 4,
    VISION_LBM		= 5,
    VISION_IFF = VISION_LBM,
    VISION_MNG		= 6,
    VISION_PBM		= 7,
    VISION_PBMRAW	= 8,
    VISION_PCD		= 9,
    VISION_PCX		= 10,
    VISION_PGM		= 11,
    VISION_PGMRAW	= 12,
    VISION_PNG		= 13,
    VISION_PPM		= 14,
    VISION_PPMRAW	= 15,
    VISION_RAS		= 16,
    VISION_TARGA	= 17,
    VISION_TIFF	= 18,
    VISION_WBMP	= 19,
    VISION_PSD		= 20,
    VISION_CUT		= 21,
    VISION_XBM		= 22,
    VISION_XPM		= 23,
    VISION_DDS		= 24,
    VISION_GIF     = 25,
    VISION_HDR		= 26,
    VISION_FAXG3	= 27,
    VISION_SGI		= 28,
    VISION_EXR		= 29,
    VISION_J2K		= 30,
    VISION_JP2		= 31,
    VISION_PFM		= 32,
    VISION_PICT	= 33,
    VISION_RAW		= 34,
    VISION_WEBP	= 35,
    VISION_JXR		= 36
} VisionImageFormat;

//
typedef enum {
    EXPLORE_INFINITY, // 探索无限
    ANCIENT_STYLE, // 古风
    TWO_DIMENSIONAL, // 二次元
    REALISTIC_STYLE, // 写实风格
    UKIYO_E, // 浮世绘
    LOW_POLY,
    FUTURISM, // 未来主义
    PIXEL_STYLE, // 像素风格
    CONCEPTUAL_ART, // 概念艺术
    CYBERPUNK, // 赛博朋克
    LOLITA_STYLE, // 洛丽塔风格
    BAROQUE_STYLE, // 巴洛克风格
    SURREALISM, // 超现实主义
    WATERCOLOR_PAINTING, // 水彩画
    VAPORWAVE_ART, // 蒸汽波艺术
    OIL_PAINTING, // 油画
    ARTOON_DRAWING, // 卡通画
} VisionImageStyle;

typedef struct _ImageData
{
    VisionImageFormat format;
    int height;
    int width;
    int total;  // 总共生成的图片数量
    int index; // 当前数据对应的图片的索引
    unsigned char *data;
    int data_size;
    int error_code;
} VisionImageData;

typedef void* VisionSession;
typedef void (*ImageResultCallback)(VisionImageData image_data, void* user_data);

VisionResult vision_create_session(VisionSession* session);
void vision_destroy_session(VisionSession session);

///
/// \brief vision_init_session
/// \param session
/// \return
///
VisionResult vision_init_session(VisionSession session);

void vision_set_prompt2image_callback(
    VisionSession session, ImageResultCallback callback, void* user_data);

// 设置生成图像的分辨率大小
// 当前仅支持 1024*1024，1024*1536，1536*1024
void vision_set_prompt2image_size(VisionSession session, int width, int height);

// 设置生成多少张图像
// 当前支持1-6张图片
void vision_set_prompt2image_number(VisionSession session, int number);

// 设置生成图片的风格
// 默认风格为
void vision_set_prompt2image_style(VisionSession session, VisionImageStyle style);

// 根据文本描述生成图像
VisionResult vision_prompt2image(VisionSession session, const char* prompt);

// 根据文本描述生成图像,异步接口
VisionResult vision_prompt2image_async(VisionSession session, const char* prompt);

// 获取支持的图像分辨率和生成的图片数量，返回Json格式，比如：
// {
//    "resolution-ratio": ["512x512", "1024x1024", "1024x1536", "1536x1024"], // 支持的分辨率
//    "image-number": 8 // 支持生成的图片数量
// }
const char* vision_get_prompt2image_supported_params(VisionSession session);

/// @brief 获取最新的错误信息
/// @return 返回调用者线程产生的错误信息
const char* vision_get_last_error_message();

#ifdef __cplusplus
}
#endif

#endif // VISION_H
