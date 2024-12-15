// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

namespace Dtk::Core {
    class DConfig;
}

namespace notification {

class NotifySetting : public QObject
{
    Q_OBJECT
public:
    static NotifySetting *instance();

    int bubbleCount() const;
    int contentRowCount() const;

signals:
    void bubbleCountChanged(int count);
    void contentRowCountChanged(int rowCount);

private:
    explicit NotifySetting(QObject *parent = nullptr);
    ~NotifySetting() override;

    void updateMaxBubbleCount();
    void updateBubbleContentRowCount();

private:
    int m_bubbleCount = 3;
    int m_contentRowCount = 6;

    Dtk::Core::DConfig *m_setting = nullptr;
};

}