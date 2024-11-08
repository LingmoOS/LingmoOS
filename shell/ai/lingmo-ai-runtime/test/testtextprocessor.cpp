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

#include <assert.h>

#include <atomic>
#include <iostream>
#include <thread>

#include "services/server.h"
#include "services/textprocessorproxy.h"

int main(int argc, char *argv[]) {
    auto &server = Server::getInstance();

    std::atomic<bool> shouldQuit = false;

    std::thread t([&shouldQuit]() {
        TextProcessorProxy proxy;
        proxy.setChatCallback(
            [&shouldQuit](const std::string &answer, void *userData) {
                std::cout << "answer: " << answer << std::endl;
                assert(!answer.empty());
                Server::getInstance().quit();
            });
        proxy.chat("Hello, Sever! I'm client.");
        while (!shouldQuit)
            ;
    });

    server.exec();

    shouldQuit = true;

    t.join();

    return 0;
}