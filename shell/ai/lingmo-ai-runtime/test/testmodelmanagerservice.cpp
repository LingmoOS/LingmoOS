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

#include <iostream>
#include <thread>

#include "services/modelmanagerserviceproxy.h"
#include "services/server.h"

int main(int argc, char *argv[]) {
    std::thread t([]() {
        auto &proxy = ModelManagerServiceProxy::getInstance();

        assert(proxy.getRemoteModelVendorListLength() == 1);

        const auto &vendorList = proxy.getRemoteModelVendorList();
        assert(vendorList.size() == 1);
        assert(vendorList[0] == BAIDU);

        assert(proxy.getRemoteModelListLength(vendorList[0]) == 1);

        const auto &modelList = proxy.getRemoteModelList(vendorList[0]);
        assert(modelList.size() == 1);
        assert(modelList[0] == NLP);

        assert(proxy.setRemoteModelConfig(vendorList[0], modelList[0], "{}"));
        assert(proxy.removeRemoteModelConfig(vendorList[0], modelList[0]));
        assert(proxy.getRemoteModelConfigLength() != 0);
        assert(!proxy.getRemoteModelConfig().empty());

        assert(proxy.getDefaultRemoteModelVendor() == BAIDU);
        assert(proxy.setDefaultRemoteModelVendor(BAIDU));

        Server::getInstance().quit();
    });

    Server::getInstance().exec();

    if (t.joinable()) {
        t.join();
    }

    return 0;
}