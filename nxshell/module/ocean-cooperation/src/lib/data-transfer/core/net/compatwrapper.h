// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATWRAPPER_H
#define COMPATWRAPPER_H

#include "compatstruct.h"
#include <CuteIPCInterface.h>

#include <QString>
#include <QObject>
#include <QSharedPointer>

namespace cooperation_core {

class CompatWrapperPrivate;
class CompatWrapper : public QObject {
    Q_OBJECT
public:
    static CompatWrapper *instance();

    CuteIPCInterface *ipcInterface();
    QString session();

signals:
    void compatConnectResult(int result, const QString &ip);

private:
    explicit CompatWrapper(QObject *parent = nullptr);
    ~CompatWrapper();

private:
    QSharedPointer<CompatWrapperPrivate> d { nullptr };
};

}   // namespace cooperation_core

#endif // COMPATWRAPPER_H
