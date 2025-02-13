/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QApt/Backend>
#include <QApt/Transaction>

class AppManager : public QObject
{
    Q_OBJECT

public:
    explicit AppManager(QObject *parent = nullptr);

    void uninstall(const QString &content);

private:
    void notifyUninstalling(const QString &packageName);
    void notifyUninstallFailure(const QString &packageName);
    void notifyUninstallSuccess(const QString &packageName);

private:
    QApt::Backend *m_backend;
    QApt::Transaction *m_trans;
};

#endif // APPMANAGER_H
