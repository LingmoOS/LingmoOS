// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

#pragma once

namespace ocean {
namespace insider {
class InsiderWorker;

class OceanUIInsider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList currentItems READ currentItems NOTIFY currentItemsChanged)

public:
    explicit OceanUIInsider(QObject *parent = nullptr);
    ~OceanUIInsider() override;
    QStringList currentItems() const;

public Q_SLOTS:
    void setCurrentItem(const QString &item);

Q_SIGNALS:
    void currentItemsChanged(const QStringList &currentItems);

protected Q_SLOTS:
    void updateCurrentItem(const QString &item);

private:
    QStringList m_currentItems;
    InsiderWorker *m_insider;
};
} // namespace insider
} // namespace ocean
