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

#include <iostream>
#include <thread>
#include <gio/gio.h>
#include <gio/giotypes.h>
#include <fstream>
#include "../include/lingmo-ai/ai-base/ocr.h"
#include <future>

std::future<void> session1() {
    std::future<void> fut = std::async([]() {
        OcrSession session;
        ocr_create_session(&session);

        std::string path = "/home/zkx/AI/testocr/kdkocr/2.png";

        const char* textFromImageFile = ocr_get_text_from_image_file(session, path.c_str());
        if (!textFromImageFile) {
            return;
        }
        std::string resultStr { textFromImageFile };
        std::cout << "test1: get result from image file\n" << resultStr << std::endl;
        delete textFromImageFile;

        //读取图片文件数据
        std::ifstream is(path, std::ifstream::in | std::ios::binary);
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);
        char *buffer = new char[length];
        is.read(buffer, length);

        const char* textFromImageData = ocr_get_text_from_image_data(session, buffer, length);
        if (textFromImageData) {
            std::string resultStr2 { textFromImageData };
            std::cout << "test1: get result from image data\n" << resultStr2 << std::endl;
            delete textFromImageData;
        }
    });
    return fut;
}

std::future<void> session2() {
    std::future<void> fut = std::async([]() {
        OcrSession session;
        ocr_create_session(&session);

        std::string path = "/home/zkx/AI/testocr/kdkocr/2.png";

        const char* textFromImageFile = ocr_get_text_from_image_file(session, path.c_str());
        if (!textFromImageFile) {
            return;
        }
        std::string resultStr { textFromImageFile };
        std::cout << "test2: get result from image file\n" << resultStr << std::endl;
        delete textFromImageFile;

        //读取图片文件数据
        std::ifstream is(path, std::ifstream::in | std::ios::binary);
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);
        char *buffer = new char[length];
        is.read(buffer, length);

        const char* textFromImageData = ocr_get_text_from_image_data(session, buffer, length);
        if (textFromImageData) {
            std::string resultStr2 { textFromImageData };
            std::cout << "test2: get result from image data\n" << resultStr2 << std::endl;
            delete textFromImageData;
        }
    });
    return fut;
}

std::future<void> session3() {
    std::future<void> fut = std::async([]() {
        OcrSession session;
        ocr_create_session(&session);

        std::string path = "/home/zkx/AI/testocr/kdkocr/222.png";

        const char* textFromImageFile = ocr_get_text_from_image_file(session, path.c_str());
        if (!textFromImageFile) {
            return;
        }
        std::string resultStr { textFromImageFile };
        std::cout << "test3: get result from image file\n" << resultStr << std::endl;
        delete textFromImageFile;

        //读取图片文件数据
        std::ifstream is(path, std::ifstream::in | std::ios::binary);
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);
        char *buffer = new char[length];
        is.read(buffer, length);

        const char* textFromImageData = ocr_get_text_from_image_data(session, buffer, length);
        if (textFromImageData) {
            std::string resultStr2 { textFromImageData };
            std::cout << "test3: get result from image data\n" << resultStr2 << std::endl;
            delete textFromImageData;
        }
    });
    return fut;
}

int main(int argc, char *argv[]) {
    std::future<void> fut1 = session1();
    std::future<void> fut2 = session2();
    std::future<void> fut3 = session3();

    fut1.get();
    fut2.get();
    fut3.get();

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);

    return 0;
}

