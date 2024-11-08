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
#include <lingmo-ai/ai-base/speech.h>
#include <gio/gio.h>
#include <gio/giotypes.h>
#include <lingmo-ai/config.h>

std::vector<char> readAudioData(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> audioData(fileSize);
    file.read(audioData.data(), fileSize);
    return audioData;
}

auto data = readAudioData("/home/zkx/16k.pcm");
auto data1 = readAudioData("/home/zkx/16k_10.pcm");

void testSpeech() {
    SpeechSession session;
    speech_create_session(&session);
    speech_init_session(session);

    int a = 10;
    speech_set_recognizing_callback(
                session,
                [](RecognitionResult result, void* userData) {
        std::cout << "---testSpeechApi-RecognitionResult--data---" << result.text_data
                  << "---speaker---" << result.speaker_id
                  << "---error---" << result.error_code
                  << "---userData---" << *(int*)userData << std::endl;

    }, (void*)&a);
    speech_set_recognized_callback(
                session,
                [](RecognitionResult result, void* userData) {
        std::cout << "---testSpeechApi-recognized finished--data---" << result.text_data
                  << "---speaker---" << result.speaker_id
                  << "---error---" << result.error_code
                  << "---userData---" << *(int*)userData << std::endl;
    }, (void*)&a);

    speech_set_synthesizing_callback(
                session,
                [](SynthesisResult result, void* userData){
        std::cout << "---testSpeechApi-SynthesisResult--size---" << result.audio_data_length
                  << "---rate---" << result.audio_rate
                  << "---userData---" << *(int*)userData << std::endl;
    }, (void*)&a);

    speech_set_synthesized_callback(
                session, [](void* userData) {
        std::cout << "---testSpeechApi--synthesized finished---"
                  << "---userData---" << *(int*)userData << std::endl;
    }, (void*)&a);


//    speech_start_continuous_recognition(session, "\{\"rate\": 16000}");
//    speech_continuous_recognition_write_audio_data_async(session, data.data(), data.size());

//    int i = 0;
//    while (i < data.size()) {
//        speech_continuous_recognition_write_audio_data_async(session, data.data()+i, 1024);
//        i += 1024;
//        usleep(10000);
//    }
//    i -= 1024;
//    speech_continuous_recognition_write_audio_data_async(
//                session, data.data() + i, data.size()-i);

//    sleep(5);
//    speech_stop_continuous_recognition(session);
//    speech_stop_continuous_recognition(session);

//    speech_recognize_once_async(session, "\{\"rate\": 1600}", data.data(), data.size());

//    speech_start_continuous_synthesis(session, "\{\"speed\": 5}");
//    speech_continuous_synthesize_write_text_async(session, "你好");
//    speech_continuous_synthesize_write_text(session, "灵墨软件");
//    speech_continuous_synthesize_write_text(session, "你吃饭了么，你吃饭了么，你吃饭了么，你吃饭了么，你吃饭了么，你吃饭了么");

//    sleep(3);
//    speech_stop_continuous_synthesis(session);

//    speech_synthesize_once_async(session, "\{\"speed\": 5}", "你好,灵墨");

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);
}


std::future<void> session1() {
    std::future<void> fut = std::async([]() {
        SpeechSession session;
        speech_create_session(&session);
        speech_init_session(session);

        int a = 10;
        speech_set_recognizing_callback(
                    session,
                    [](RecognitionResult result, void* userData) {
            std::cout << "---testSpeechApi-RecognitionResult-session111-data---" << result.text_data
                      << "---speaker---" << result.speaker_id
                      << "---error---" << result.error_code
                      << "---userData---" << *(int*)userData << std::endl;

        }, (void*)&a);
        speech_set_recognized_callback(
                    session,
                    [](RecognitionResult result, void* userData) {
            std::cout << "---testSpeechApi-recognized finished-session111-data---" << result.text_data
                      << "---speaker---" << result.speaker_id
                      << "---error---" << result.error_code
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);

        speech_set_synthesizing_callback(
                    session,
                    [](SynthesisResult result, void* userData){
            std::cout << "---testSpeechApi-SynthesisResult-session111-size---" << result.audio_data_length
                      << "---rate---" << result.audio_rate
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);

        speech_set_synthesized_callback(
                    session, [](void* userData) {
            std::cout << "---testSpeechApi--synthesized finished-session111--"
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);


//        speech_start_continuous_recognition(session, "\{\"rate\": 16000}");
//        speech_continuous_recognition_write_audio_data_async(session, data.data(), data.size());

//        speech_recognize_once_async(session, "\{\"rate\": 1600}", data.data(), data.size());

//        speech_start_continuous_synthesis(session, "\{\"speed\": 5}");
//        speech_continuous_synthesize_write_text_async(session, "你好");

    //    sleep(3);
    //    speech_stop_continuous_synthesis(session);

//        speech_synthesize_once_async(session, "\{\"speed\": 5}", "你好");

        auto *loop_ = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop_);
    });
    return fut;
}

std::future<void> session2() {
    std::future<void> fut = std::async([]() {
        SpeechSession session;
        speech_create_session(&session);
        speech_init_session(session);

        int a = 20;
        speech_set_recognizing_callback(
                    session,
                    [](RecognitionResult result, void* userData) {
            std::cout << "---testSpeechApi-RecognitionResult-session222-data---" << result.text_data
                      << "---speaker---" << result.speaker_id
                      << "---error---" << result.error_code
                      << "---userData---" << *(int*)userData << std::endl;

        }, (void*)&a);
        speech_set_recognized_callback(
                    session,
                    [](RecognitionResult result, void* userData) {
            std::cout << "---testSpeechApi-recognized finished-session222-data---" << result.text_data
                      << "---speaker---" << result.speaker_id
                      << "---error---" << result.error_code
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);

        speech_set_synthesizing_callback(
                    session,
                    [](SynthesisResult result, void* userData){
            std::cout << "---testSpeechApi-SynthesisResult-session222-size---" << result.audio_data_length
                      << "---rate---" << result.audio_rate
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);

        speech_set_synthesized_callback(
                    session, [](void* userData) {
            std::cout << "---testSpeechApi--synthesized finished-session222--"
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);


//        speech_start_continuous_recognition(session, "\{\"rate\": 16000}");
//        speech_continuous_recognition_write_audio_data_async(session, data.data(), data.size());

    //    sleep(5);
    //    speech_stop_continuous_recognition(session);
    //    speech_stop_continuous_recognition(session);

//        speech_recognize_once_async(session, "\{\"rate\": 16000}", data1.data(), data1.size());

//        speech_start_continuous_synthesis(session, "\{\"speed\": 5}");
//        speech_continuous_synthesize_write_text_async(session, "灵墨软件");

    //    sleep(3);
    //    speech_stop_continuous_synthesis(session);

//        speech_synthesize_once_async(session, "\{\"speed\": 5}", "灵墨软件");

        auto *loop_ = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop_);
    });
    return fut;
}

std::future<void> session3() {
    std::future<void> fut = std::async([]() {
        SpeechSession session;
        speech_create_session(&session);
        speech_init_session(session);

        int a = 30;
        speech_set_recognizing_callback(
                    session,
                    [](RecognitionResult result, void* userData) {
            std::cout << "---testSpeechApi-RecognitionResult-session333-data---" << result.text_data
                      << "---speaker---" << result.speaker_id
                      << "---error---" << result.error_code
                      << "---userData---" << *(int*)userData << std::endl;

        }, (void*)&a);
        speech_set_recognized_callback(
                    session,
                    [](RecognitionResult result, void* userData) {
            std::cout << "---testSpeechApi-recognized finished-session333-data---" << result.text_data
                      << "---speaker---" << result.speaker_id
                      << "---error---" << result.error_code
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);

        speech_set_synthesizing_callback(
                    session,
                    [](SynthesisResult result, void* userData){
            std::cout << "---testSpeechApi-SynthesisResult-session333-size---" << result.audio_data_length
                      << "---rate---" << result.audio_rate
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);

        speech_set_synthesized_callback(
                    session, [](void* userData) {
            std::cout << "---testSpeechApi--synthesized finished-session333--"
                      << "---userData---" << *(int*)userData << std::endl;
        }, (void*)&a);


        speech_start_continuous_recognition(session, "\{\"rate\": 16000}");
        speech_continuous_recognition_write_audio_data_async(session, data.data(), data.size());

    //    sleep(5);
    //    speech_stop_continuous_recognition(session);
    //    speech_stop_continuous_recognition(session);

//        speech_recognize_once_async(session, "\{\"rate\": 1600}", data.data(), data.size());

//        speech_start_continuous_synthesis(session, "\{\"speed\": 5}");
//        speech_continuous_synthesize_write_text_async(session, "你吃饭了么");

    //    sleep(3);
    //    speech_stop_continuous_synthesis(session);

//        speech_synthesize_once_async(session, "\{\"speed\": 5}", "你吃饭了么");

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
