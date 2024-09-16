// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ASIOSERVICE_H
#define ASIOSERVICE_H

#include "server/asio/service.h"

class AsioService : public CppServer::Asio::Service
{
public:
    using CppServer::Asio::Service::Service;

protected:
    void onError(int error, const std::string &category, const std::string &message) override
    {
        std::cout << "Asio service caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

#endif // ASIOSERVICE_H
