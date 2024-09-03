/*
 * Copyright (C) 2024 LingmoOS Team.
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

#ifndef UPDATORHELPER_H
#define UPDATORHELPER_H

#include <QObject>

// QApt
#include <QApt/Backend>
#include <QApt/Config>
#include <QApt/Transaction>

class UpdatorHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString statusDetails READ statusDetails NOTIFY statusDetailsChanged)
    Q_PROPERTY(int checkProgress READ checkProgress NOTIFY checkProgressChanged)

public:
    explicit UpdatorHelper(QObject *parent = nullptr);
    ~UpdatorHelper();

    Q_INVOKABLE void checkUpdates();
    Q_INVOKABLE void upgrade();
    Q_INVOKABLE void reboot();

    QString version();
    QString statusDetails();
    int checkProgress();

signals:
    void startingUpdate();
    void updateFinished();
    void updateError();
    void checkUpdateFinished();
    void statusDetailsChanged();
    void checkProgressChanged();
    void checkError(const QString &details);

private:
    QString m_currentVersion;
    QString m_statusDetails;
    int m_checkProgress;
    QApt::Backend *m_backend;
    QApt::Transaction *m_trans;
};

#endif // UPDATORHELPER_H
