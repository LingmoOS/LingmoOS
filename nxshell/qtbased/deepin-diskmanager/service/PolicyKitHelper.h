// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <polkit-qt5-1/PolkitQt1/Authority>

using namespace PolkitQt1;

class PolicyKitHelper
{
public:
    inline static PolicyKitHelper* instance(){
        static PolicyKitHelper instance;
        return &instance;
    }

    bool checkAuthorization(const QString& actionId, const QString& appBusName);

private:
    PolicyKitHelper();
    ~PolicyKitHelper();

    Q_DISABLE_COPY(PolicyKitHelper)

};


