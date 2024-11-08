#include <assert.h>

#include <atomic>
#include <iostream>
#include <thread>

#include "services/server.h"
#include "services/textprocessorproxy.h"
#include "services/servicemanager.h"
#include "engine/aienginepluginmanager.h"

int main(int argc, char *argv[]) {
    auto &server = Server::getInstance();

    std::atomic<bool> shouldQuit = false;

    std::cout << "server exec" << std::endl;
    server.exec();

//    shouldQuit = true;

//    t.join();

//    return 0;
}
