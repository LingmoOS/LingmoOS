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

#ifndef OCR_H
#define OCR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OCR_SUCCESS=0,
    OCR_SESSION_ERROR, // session 错误
    OCR_PARAM_ERROR, // 参数错误
} OcrResult;

typedef void* OcrSession;

OcrResult ocr_create_session(OcrSession* session);
void ocr_destroy_session(OcrSession session);

/// @brief 识别指定图片文件中的文字
/// @param image_file
/// @return
const char* ocr_get_text_from_image_file(OcrSession session, const char* image_file);

const char* ocr_get_text_from_image_data(OcrSession session,
                                         const char* image_data,
                                         unsigned int image_data_length);

#ifdef __cplusplus
}
#endif

#endif
