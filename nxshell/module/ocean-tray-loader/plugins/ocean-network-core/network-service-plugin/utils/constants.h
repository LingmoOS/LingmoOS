// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONSTRANTS_H
#define CONSTRANTS_H

namespace network {
namespace service {

enum class Connectivity {
    Unknownconnectivity = 0, // 网络连接未知。这意味着连接检查已禁用（例如，在服务器上安装）或尚未运行。图形外壳程序应假定Internet连接可用，并且不显示强制门户窗口
    Noconnectivity,          // 主机未连接到任何网络。没有活动的连接包含默认的Internet路由，因此甚至尝试进行连接检查也没有意义。图形shell应使用此状态来指示网络连接不可用。
    Portal,                  // Internet连接被捕获的入口网关劫持。图形shell可以打开sandboxed web浏览器窗口(因为捕获的门户通常会尝试对https连接进行中间人攻击），以便对网关进行身份验证，并在浏览器窗口关闭时使用CheckConnectivity()重新触发连接检查。
    Limited,                 // 主机已连接到网络，似乎无法访问完整的Internet，但尚未检测到捕获的门户
    Full                     // 主机已连接到网络，并且似乎能够访问完整的Internet。
};

}
}

#endif // SERVICE_H
