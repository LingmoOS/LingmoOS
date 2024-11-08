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

#include "services/ocrprocessorproxy.h"
#include "services/serverproxy.h"
#include <iostream>
#include <vector>
#include <leptonica/allheaders.h>
#include <iostream>
#include <fstream>
#include <stdio.h>

OcrProcessorProxy::OcrProcessorProxy() {}

OcrProcessorProxy::~OcrProcessorProxy() {}

std::vector<std::string> OcrProcessorProxy::getText(tesseract::TessBaseAPI *api) {
    std::vector<std::string> textResult;
    if (!api) {
        fprintf(stderr, "ocr get text error: api is nullptr !\n");
        return textResult;
    }
    Boxa *boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
    if (!boxes) {
        fprintf(stderr, "ocr get text error: boxes is nullptr !\n");
        return textResult;
    }
    for (int i = 0; i < boxes->n; ++i) {
        BOX *box = boxaGetBox(boxes, i, L_CLONE);
        if (!box) {
            fprintf(stderr, "ocr get text error: box is nullptr !\n");
            return textResult;
        }

        // 文字识别
        api->SetRectangle(box->x, box->y-1, box->w, box->h+1);
        char* ocrResult = api->GetUTF8Text();

        textResult.push_back(std::string(ocrResult));
        delete [] ocrResult;

        boxDestroy(&box);
    }

    boxDestroy(boxes->box);
    delete boxes;
    return textResult;
}

std::vector<std::string> OcrProcessorProxy::getImageText(const char *imageData, int size) {
    if (imageData == nullptr) {
        fprintf(stderr, "ocr get image text error: imageData is nullptr !\n");
        return std::vector<std::string>();
    }
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

    //返回表示0初始化成功，非0值表示不同类型的错误
    if (api->Init(NULL, "chi_sim") && api->Init(NULL, "eng")) {
        fprintf(stderr, "ocr get image text error: TessBaseAPI init failed !\n");
        return std::vector<std::string>();
    }

    Pix *image = pixReadMem(reinterpret_cast<const l_uint8*>(imageData), size);
    if (!image) {
        fprintf(stderr, "ocr get image text error: image data is nullptr !\n");
        pixDestroy(&image);
        api->End();
        delete api;
        return std::vector<std::string>();
    }
    api->SetImage(image);

    std::vector<std::string> text = getText(api);

    pixDestroy(&image);
    api->End();
    delete api;
    return text;
}

const char* OcrProcessorProxy::getTextFromImageFile(const char* image_file) {
    std::fstream file(image_file);
    if (!file.is_open()) {
        fprintf(stderr, "ocr get text from image file error: image file not exists!\n");
        return nullptr;
    }

    std::string resultString;

    std::ifstream is(image_file, std::ifstream::in | std::ios::binary);
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    char *buffer = new char[length];
    is.read(buffer, length);

    std::vector<std::string> result = getImageText(buffer, length);
    is.clear();
    for(int i = 0; i < result.size(); i++){
        resultString += result.at(i);
    }

    char * strc = new char[strlen(resultString.c_str())+1];
    strcpy(strc, resultString.c_str());

    return strc;
}

const char* OcrProcessorProxy::getTextFromImageData(const char* image_data,
                                                    unsigned int image_data_length) {
    std::string resultString;
    if (image_data == nullptr || image_data_length == 0) {
        fprintf(stderr, "ocr get text from image data error: image data is nullptr!\n");
        return nullptr;
    }

    std::vector<std::string> result = getImageText(image_data, image_data_length);
    for(int i = 0; i < result.size(); i++){
        resultString += result.at(i);
    }

    char * strc = new char[strlen(resultString.c_str())+1];
    strcpy(strc, resultString.c_str());

    return strc;
}


