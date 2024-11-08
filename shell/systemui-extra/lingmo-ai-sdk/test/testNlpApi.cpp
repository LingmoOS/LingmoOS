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
#include <stdio.h>
#include <vector>
#include <fstream>
#include <future>
#include <gio/gio.h>
#include <gio/giotypes.h>
#include "lingmo-ai/ai-base/nlp.h"
#include <lingmo-ai/config.h>

void testNlp() {
    NlpSession session;
    nlp_create_session(&session);
    nlp_init_session(session);

    int a = 30;
    nlp_set_result_callback(session,
                [](NlpTextData result, void* user_data) {
        std::cout << "--nlp---session----" << result.text
                  << "--errorCode--" << result.error_code
                  << "--user---\n" << *(int*)user_data << std::endl;
    }, &a);

//    nlp_text_chat_async(session, "我是谁");

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);
}

std::future<void> session1() {
    std::future<void> fut = std::async(std::launch::async, []() {
        NlpSession session1;
        nlp_create_session(&session1);
        nlp_init_session(session1);

        int a = 10;
        nlp_set_result_callback(session1,
                    [](NlpTextData result, void* user_data) {
            std::cout << "--nlp---session1111----" << result.text
                      << "--errorCode--" << result.error_code
                      << "--user---\n" << *(int*)user_data << std::endl;
        }, &a);

//        nlp_text_chat_async(session1, "你今年多大");
    });
    return fut;
}

std::future<void> session2() {
    std::future<void> fut = std::async(std::launch::async, []() {
        NlpSession session2;
        nlp_create_session(&session2);
        nlp_init_session(session2);

        int a = 20;
        nlp_set_result_callback(session2,
                    [](NlpTextData result, void* user_data) {
            std::cout << "--nlp---session2222----" << result.text
                      << "--errorCode--" << result.error_code
                      << "--user---\n" << *(int*)user_data << std::endl;
        }, &a);

        nlp_set_context_size(session2, 100);
//        nlp_text_chat_async(session2, "你开心么");
    });
    return fut;
}

std::future<void> session3() {
    std::future<void> fut = std::async(std::launch::async, []() {
        NlpSession session3;
        nlp_create_session(&session3);
        nlp_init_session(session3);

        int a = 30;
        nlp_set_result_callback(session3,
                    [](NlpTextData result, void* user_data) {
            std::cout << "--nlp---session3333----" << result.text
                      << "--errorCode--" << result.error_code
                      << "--user---\n" << *(int*)user_data << std::endl;
        }, &a);

        nlp_text_chat_async(session3, "你吃饭了么");
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

    fflush(stdout);

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);

    return 0;
}
