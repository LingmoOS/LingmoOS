// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONCOREPLUGIN_H
#define COOPERATIONCOREPLUGIN_H

#include "common/exportglobal.h"

#include <QObject>
#include <QSharedPointer>

namespace cooperation_core {
    class MainWindow;
}

namespace cooperation_transfer {

class EXPORT_API TransferPlugin : public QObject
{
    Q_OBJECT

public:
    explicit TransferPlugin(QObject *parent = nullptr);
    ~TransferPlugin();

    bool start();
    void stop();

private:
    void initialize();
    QSharedPointer<cooperation_core::MainWindow> dMain { nullptr };
};

}   // namespace cooperation_transfer

#endif   // COOPERATIONCOREPLUGIN_H
