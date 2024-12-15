// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONUTIL_H
#define COOPERATIONUTIL_H

#include "global_defines.h"
#include "discover/deviceinfo.h"

#include <QWidget>
#include <QDialog>

namespace cooperation_core {

class CooperationUtilPrivate;
class MainWindow;
class CooperationUtil : public QObject
{
    Q_OBJECT
public:
    static CooperationUtil *instance();

    void mainWindow(QSharedPointer<MainWindow> window);

    void activateWindow();

    void registerDeviceOperation(const QVariantMap &map);

    void setStorageConfig(const QString &value);

    void showFeatureDisplayDialog(QDialog *dlg);

    void initNetworkListener();

    static QVariantMap deviceInfo();
    static QString localIPAddress();
    static QString closeOption();
    static void saveOption(bool exit);

Q_SIGNALS:
    void onlineStateChanged(const QString &validIP);
    void storageConfig(const QString &value);

private:
    explicit CooperationUtil(QObject *parent = nullptr);
    ~CooperationUtil();

    void checkNetworkState();

private:
    QSharedPointer<CooperationUtilPrivate> d { nullptr };
};

}   // namespace cooperation_core

#endif   // COOPERATIONUTIL_H
