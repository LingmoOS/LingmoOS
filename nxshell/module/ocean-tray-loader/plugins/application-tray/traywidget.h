// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstracttrayprotocol.h"

#include <QWidget>
#include <QPointer>

namespace tray {
class AbstractTrayProtocolHandler;
class TrayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrayWidget(QPointer<AbstractTrayProtocolHandler> handler);
    ~TrayWidget();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPointer<AbstractTrayProtocolHandler> m_handler;
    QTimer *m_attentionTimer;
};
}