// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONSERVICE_H
#define SESSIONSERVICE_H

#include <QObject>

namespace network {
namespace sessionservice {

class SessionContainer;
class SessionService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.service.SessionNetwork")

public:
    explicit SessionService(SessionContainer *container, QObject *parent = nullptr);

private:
    SessionContainer *m_container;
};

}
}

#endif // SERVICE_H
