// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef INSIDERWORKER_H
#define INSIDERWORKER_H

#include <QObject>

namespace ocean {
namespace insider {
class PkPackagesProxy;

class InsiderWorker : public QObject
{
    Q_OBJECT
public:
    explicit InsiderWorker(QObject *parent = nullptr);
    ~InsiderWorker() override;

    QString displayManager() const;

public Q_SLOTS:
    void setDisplayManager(const QString &id);

Q_SIGNALS:
    void displayManagerChanged(const QString &displayManager);

protected Q_SLOTS:
    void checkEnabledDisplayManager();

    void onDisplayManagerFinished();

    void switchDisplayManager(bool isNew);

private:
    QString m_displayManager;
};
} // namespace insider
} // namespace ocean
#endif // INSIDERWORKER_H
