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
#include <future>
#include <vector>
#include "../include/lingmo-ai/ai-base/vision.h"
#include "fstream"

void testVision() {
    VisionSession session;
    vision_create_session(&session);
    vision_init_session(session);

    int a = 100;
    vision_set_prompt2image_callback(
                session,
                [](VisionImageData data, void* user_data) {
        std::cout << "---testVisionApi--data--"
                  << data.data << "--index--" << data.index
                  << "--total--" << data.total
                  << "--width--" << data.width
                  << "--userdata--" << *(int*)user_data << std::endl;
    },
    &a);
//    vision_set_prompt2image_size(session, 1024, 1024);
//    vision_set_prompt2image_number(session, 1);
//    vision_set_prompt2image_style(session, VisionImageStyle::EXPLORE_INFINITY);

//    vision_prompt2image_async(session, "花");

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);
}

static void writeBinaryDataToFile(const std::string &filename,
                           const std::vector<char> &data) {
    std::ofstream outputFile(filename, std::ios::out | std::ios::binary | std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    // 写入数据到文件
    outputFile.write(data.data(), data.size());

    // 检查是否写入成功
    if (!outputFile.good()) {
        std::cerr << "Error occurred while writing to file." << std::endl;
    } else {
        std::cout << "Binary data has been written to file: " << filename
                  << std::endl;
    }

    // 关闭文件
    outputFile.close();
}

std::future<void> session1() {
    std::future<void> fut = std::async([]() {
        VisionSession session;
        vision_create_session(&session);
        vision_init_session(session);

        int a = 10;
        vision_set_prompt2image_callback(
                    session,
                    [](VisionImageData data, void* user_data) {
            std::cout << "---testVisionApi--index--" << data.index
                      << "--total--" << data.total
                      << "--width--" << data.width
                      << "--userdata--" << *(int*)user_data << std::endl;

            std::vector<char> imagedata { data.data, data.data+data.data_size };
            writeBinaryDataToFile("/home/zkx/speech/image4.png", imagedata);
        },
        &a);
        vision_set_prompt2image_size(session, 1024, 1024);
        vision_set_prompt2image_number(session, 1);
        vision_set_prompt2image_style(session, VisionImageStyle::WATERCOLOR_PAINTING);

//        vision_prompt2image_async(session, "花");

        auto *loop_ = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop_);
    });
    return fut;
}

std::future<void> session2() {
    std::future<void> fut = std::async([]() {
        VisionSession session;
        vision_create_session(&session);
        vision_init_session(session);

        int a = 20;
        vision_set_prompt2image_callback(
                    session,
                    [](VisionImageData data, void* user_data) {
            std::cout << "---testVisionApi--index--" << data.index
                      << "--total--" << data.total
                      << "--width--" << data.width
                      << "--userdata--" << *(int*)user_data << std::endl;

            std::vector<char> imagedata { data.data, data.data+data.data_size };
            writeBinaryDataToFile("/home/zkx/speech/image5.png", imagedata);
        },
        &a);
        vision_set_prompt2image_size(session, 2048, 2048);
        vision_set_prompt2image_number(session, 1);
        vision_set_prompt2image_style(session, VisionImageStyle::OIL_PAINTING);

//        vision_prompt2image_async(session, "草");

        auto *loop_ = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop_);
    });
    return fut;
}

std::future<void> session3() {
    std::future<void> fut = std::async([]() {
        VisionSession session;
        vision_create_session(&session);
        vision_init_session(session);

        int a = 30;
        vision_set_prompt2image_callback(
                    session,
                    [](VisionImageData data, void* user_data) {
            std::cout << "---testVisionApi-index--" << data.index
                      << "--total--" << data.total
                      << "--width--" << data.width
                      << "--userdata--" << *(int*)user_data << std::endl;

            std::vector<char> imagedata { data.data, data.data+data.data_size };
            writeBinaryDataToFile("/home/zkx/speech/image6.png", imagedata);
        },
        &a);
        vision_set_prompt2image_size(session, 1024, 1024);
        vision_set_prompt2image_number(session, 1);
        vision_set_prompt2image_style(session, VisionImageStyle::EXPLORE_INFINITY);

//        vision_prompt2image_async(session, "树木");

        auto *loop_ = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop_);
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

