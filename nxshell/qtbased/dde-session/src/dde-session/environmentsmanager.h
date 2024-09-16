// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENVIRONMENTS_MANAGER_H
#define ENVIRONMENTS_MANAGER_H

#include <QMap>
/**
 * @brief The environments_manager class
 * @brief 自动处理应用运行时的环境变量，DBus环境变量，systemd的用户环境变量等
 */
class EnvironmentsManager
{
public:
    explicit EnvironmentsManager();

    void init();

private:
    void createGeneralEnvironments();
    void createKeyringEnvironments();
    void createDBusEnvironments();

    bool unsetEnv(QString env);

private:
    QMap<QString, QString> m_envMap;
};

#endif // ENVIRONMENTS_MANAGER_H
