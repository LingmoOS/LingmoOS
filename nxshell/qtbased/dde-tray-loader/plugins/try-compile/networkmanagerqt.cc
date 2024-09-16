// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <networkmanagerqt/wireddevice.h>
#include <networkmanagerqt/manager.h>

using namespace NetworkManager;

int main(int argc, char* argv[])
{
    for (auto device : networkInterfaces()) {
        WiredDevice::Ptr wDevice = device.staticCast<WiredDevice>();
        wDevice->interfaceFlags();
        return 1;
    }

    return 0;
}
