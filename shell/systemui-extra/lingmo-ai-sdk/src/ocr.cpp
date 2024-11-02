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

#include "ocr.h"
#include "ocrsession.h"

OcrResult ocr_create_session(OcrSession* session)
{
    if (!session) {
        return OCR_SESSION_ERROR;
    }
    ocr::OcrSession *ocrSession = new ocr::OcrSession();
    *session = ocrSession;
    return OCR_SUCCESS;
}

void ocr_destroy_session(OcrSession session)
{
    ocr::OcrSession *ocrSession = static_cast<ocr::OcrSession *>(session);
    if (ocrSession) {
        delete ocrSession;
    }
}

const char* ocr_get_text_from_image_file(OcrSession session, const char* image_file) {
    if (image_file == nullptr) {
        fprintf(stderr, "get text from image file error: image file is nullptr!\n");
        return nullptr;
    }
    ocr::OcrSession *ocrSession = static_cast<ocr::OcrSession *>(session);
    if (!ocrSession) {
        return nullptr;
    }
    return ocrSession->getTextFromImageFile(image_file);
}

const char* ocr_get_text_from_image_data(OcrSession session,
                                         const char* image_data,
                                         unsigned int image_data_length) {
    if (image_data == nullptr || image_data_length == 0) {
        fprintf(stderr, "get text from image data error: image data is nullptr!\n");
        return nullptr;
    }
    ocr::OcrSession *ocrSession = static_cast<ocr::OcrSession *>(session);
    if (!ocrSession) {
        return nullptr;
    }
    return ocrSession->getTextFromImageData(image_data, image_data_length);
}
